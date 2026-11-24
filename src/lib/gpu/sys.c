/*
 * GPU control, drawing/display environments, command queue, and DMA transfer.
 */
#include <sys/types.h>
#include <stdio.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include "etc_int.h"
#include "gpu_int.h"

/* memory.h would also declare memset, which this module defines static */
extern void *memcpy();

static void memset(char *p, int c, int n);

static int _addque(), _addqueCopy(), _clr(), _cwb(), _drs(), _dws();
static int _exeque(), _getctl(), _otc(), _param(), _reset(), _sync();
static void _ctl(), _cwc();
static u_long _status();

static void checkRECT(char *s, RECT *r);
static void SetDrawEnvFillRect(DR_ENV *dr_env, DRAWENV *de);
static u_long get_mode(int dfe, int dtd, u_short tpage);
static u_long get_cs(short x, short y);
static u_long get_ce(short x, short y);
static u_long get_ofs(short x, short y);
static u_long get_tw(RECT *tw);
static void set_alarm(void);
static int get_alarm(void);
static int _version(int mode);

/* a queued call: the routine, its parameter or a copy of it in buf, and a value */
typedef struct {
	void	(*func)();
	u_long	*param;
	u_long	value;
	u_long	buf[21];
} GpuQueueEntry;

/* the GPU's jump table */
typedef struct {
	char	*ver;
	int	(*addque)();
	int	(*addqueCopy)();
	int	(*clr)();
	void	(*ctl)();
	int	(*cwb)();
	void	(*cwc)();
	int	(*drs)();
	int	(*dws)();
	int	(*exeque)();
	int	(*getctl)();
	int	(*otc)();
	int	(*param)();
	int	(*reset)();
	u_long	(*status)();
	int	(*sync)();
} GpuJumpTable;

/* "Ps", a byte that differs by library, then five more; nothing reads them */
static u_long gpu_id[2] = { 0x25077350, 0x0043539b };

static GpuJumpTable jtb = {
	"$Id: sys.c,v 1.140 1998/01/12 07:52:27 noda Exp yos $",
	_addque, _addqueCopy, _clr, _ctl, _cwb, _cwc, _drs, _dws,
	_exeque, _getctl, _otc, _param, _reset, _status, _sync,
};

static GpuJumpTable *gpu = &jtb;

int (*GPU_printf)() = printf;

struct {
	u_char	version;
	u_char	que;
	u_char	debug;
	u_char	reverse;
	short	w, h;		/* frame buffer size */
	volatile int syncflag;	/* a command queued since the last draw sync */
	void	(*drawsync)();	/* DrawSyncCallback */
	DRAWENV	draw;
	volatile DISPENV disp;
} GEnv = { 0 };

static int vram_width[] = { 1024, 1024, 1024 };
static int vram_height[] = { 512, 512, 1024 };

int ResetGraph(int mode)
{
	switch (mode & 7) {
	case 3:
	case 0:
		printf("ResetGraph:jtb=%08x,env=%08x\n", &jtb, &GEnv);
	case 5:
		memset((char *)&GEnv, 0, sizeof(GEnv));
		ResetCallback();
		GPU_cw((u_long)gpu & GPU_ADDR_MASK);
		GEnv.version = _reset(mode);
		GEnv.que = 1;
		GEnv.w = vram_width[GEnv.version];
		GEnv.h = vram_height[GEnv.version];
		memset((char *)&GEnv.draw, -1, sizeof(DRAWENV));
		memset((char *)&GEnv.disp, -1, sizeof(DISPENV));
		return GEnv.version;
	}
	if (GEnv.debug >= 2)
		GPU_printf("ResetGraph(%d)...\n", mode);
	return gpu->reset(1);
}

int SetGraphDebug(int level)
{
	u_char old = GEnv.debug;

	GEnv.debug = level;
	if (GEnv.debug)
		GPU_printf("SetGraphDebug:level:%d,type:%d reverse:%d\n",
			GEnv.debug, GEnv.version, GEnv.reverse);
	return old;
}

int SetGraphQueue(int mode)
{
	u_char old = GEnv.que;

	if (GEnv.debug >= 2)
		GPU_printf("SetGrapQue(%d)...\n", mode);
	if (mode != GEnv.que) {
		gpu->reset(1);
		GEnv.que = mode;
		DMACallback(DMA_GPU, 0);
	}
	return old;
}

int GetGraphDebug(void)
{
	return GEnv.debug;
}

u_long DrawSyncCallback(void (*func)())
{
	void (*old)();

	if (GEnv.debug >= 2)
		GPU_printf("DrawSyncCallback(%08x)...\n", func);
	old = GEnv.drawsync;
	GEnv.drawsync = func;
	return (u_long)old;
}

void SetDispMask(int mask)
{
	if (GEnv.debug >= 2)
		GPU_printf("SetDispMask(%d)...\n", mask);
	if (mask == 0)
		memset((char *)&GEnv.disp, -1, sizeof(DISPENV));
	gpu->ctl(mask ? GP1_DISP_ON : GP1_DISP_OFF);
}

int DrawSync(int mode)
{
	if (GEnv.debug >= 2)
		GPU_printf("DrawSync(%d)...\n", mode);
	return gpu->sync(mode);
}

static void checkRECT(char *s, RECT *r)
{
	switch (GEnv.debug) {
	case 1:
		if (r->w > GEnv.w || r->w + r->x > GEnv.w ||
		    r->y > GEnv.h || r->y + r->h > GEnv.h ||
		    r->w <= 0 || r->x < 0 || r->y < 0 || r->h <= 0) {
			GPU_printf("%s:bad RECT", s);
			GPU_printf("(%d,%d)-(%d,%d)\n", r->x, r->y, r->w, r->h);
		}
		break;
	case 2:
		GPU_printf("%s:", s);
		GPU_printf("(%d,%d)-(%d,%d)\n", r->x, r->y, r->w, r->h);
		break;
	}
}

int ClearImage(RECT *rect, u_char r, u_char g, u_char b)
{
	checkRECT("ClearImage", rect);
	return gpu->addqueCopy(gpu->clr, rect, sizeof(RECT), (b << 16) | (g << 8) | r);
}

int ClearImage2(RECT *rect, u_char r, u_char g, u_char b)
{
	checkRECT("ClearImage2", rect);
	return gpu->addqueCopy(gpu->clr, rect, sizeof(RECT), CLR_FLAG_DFE | (b << 16) | (g << 8) | r);
}

int LoadImage(RECT *rect, u_long *p)
{
	checkRECT("LoadImage", rect);
	return gpu->addqueCopy(gpu->dws, rect, sizeof(RECT), p);
}

int StoreImage(RECT *rect, u_long *p)
{
	checkRECT("StoreImage", rect);
	return gpu->addqueCopy(gpu->drs, rect, sizeof(RECT), p);
}

static u_long move_packet[5] = { GPU_DMA_TAG(4, GPU_DMA_END), GP0_VRAM_BLIT };

int MoveImage(RECT *rect, int x, int y)
{

	checkRECT("MoveImage", rect);
	if (rect->w == 0 || rect->h == 0)
		return -1;
	move_packet[2] = *(u_long *)&rect->x;
	move_packet[3] = (y << 16) | (x & 0xffff);
	move_packet[4] = *(u_long *)&rect->w;
	return gpu->addqueCopy(gpu->cwc, move_packet, sizeof(move_packet), 0);
}

static u_long term[5] = { GPU_DMA_TAG(4, GPU_DMA_END), 0, 0, 0, 0 };
/* a list's last link: a copy of 2 x 1 pixels from (0, 0) onto itself, then term */
static u_long termLink[5] = { 0, GP0_VRAM_BLIT, 0, 0, (1 << 16) | 2 };

u_long *ClearOTag(u_long *ot, int n)
{
	if (GEnv.debug >= 2)
		GPU_printf("ClearOTag(%08x,%d)...\n", ot, n);
	if (--n != 0) {
		do {
			--n;
			setlen(ot, 0);
			setaddr(ot, ot + 1);
			ot++;
		} while (n != 0);
	}
	termLink[0] = GPU_DMA_TAG(4, (u_long)term & GPU_ADDR_MASK);
	*ot = (u_long)termLink & GPU_ADDR_MASK;
	return ot;
}

u_long *ClearOTagR(u_long *ot, int n)
{
	if (GEnv.debug >= 2)
		GPU_printf("ClearOTagR(%08x,%d)...\n", ot, n);
	gpu->otc(ot, n);
	termLink[0] = GPU_DMA_TAG(4, (u_long)term & GPU_ADDR_MASK);
	*ot = (u_long)termLink & GPU_ADDR_MASK;
	return ot;
}

void DrawPrim(void *p)
{
	int len = getlen(p);

	gpu->sync(0);
	gpu->cwb((u_long *)p + 1, len);
}

void DrawOTag(u_long *p)
{
	if (GEnv.debug >= 2)
		GPU_printf("DrawOTag(%08x)...\n", p);
	gpu->addqueCopy(gpu->cwc, p, 0, 0);
}

DRAWENV *PutDrawEnv(DRAWENV *de)
{
	if (GEnv.debug >= 2)
		GPU_printf("PutDrawEnv(%08x)...\n", de);
	SetDrawEnvFillRect(&de->dr_env, de);
	de->dr_env.tag |= GPU_DMA_END;
	gpu->addqueCopy(gpu->cwc, &de->dr_env, sizeof(DR_ENV), 0);
	memcpy(&GEnv.draw, de, sizeof(DRAWENV));
	return de;
}

void DrawOTagEnv(u_long *p, DRAWENV *de)
{
	if (GEnv.debug >= 2)
		GPU_printf("DrawOTagEnv(%08x,&08x)...\n", p, de);
	SetDrawEnvFillRect(&de->dr_env, de);
	setaddr(&de->dr_env, p);
	gpu->addqueCopy(gpu->cwc, &de->dr_env, sizeof(DR_ENV), 0);
	memcpy(&GEnv.draw, de, sizeof(DRAWENV));
}

DRAWENV *GetDrawEnv(DRAWENV *de)
{
	memcpy(de, &GEnv.draw, sizeof(DRAWENV));
	return de;
}

typedef struct {
	u_short start, end;
} HRange;

static HRange hRange[2][5] = {
	{ { 0x24e, 0xc4e }, { 0x258, 0xc58 }, { 0x21b, 0xc9b }, { 0x267, 0xc67 }, { 0x26c, 0xc6c } },
	{ { 0x262, 0xc62 }, { 0x270, 0xc70 }, { 0x230, 0xcb0 }, { 0x27b, 0xc7b }, { 0x280, 0xc80 } },
};
static u_char hClock[] = { 10, 8, 7, 5, 4 };

DISPENV *PutDispEnv(DISPENV *de)
{
	GpuJumpTable *g;
	u_long mode = GP1_DISP_MODE;
	int fits;
	volatile RECT *r;

	if (GEnv.debug >= 2)
		GPU_printf("PutDispEnv(%08x)...\n", de);
	{
		u_short x;
		u_long y;

		y = (u_short)de->disp.y;
		y &= 0x3ff;
		x = de->disp.x;
		x &= 0x3ff;
		g = gpu;
		g->ctl(GP1_DISP_START | y << 10 | x);
	}
	r = &GEnv.disp.disp;
	if (*(volatile u_long *)&GEnv.disp.isinter != *(u_long *)&de->isinter ||
	    r->x != de->disp.x || r->y != de->disp.y ||
	    r->w != de->disp.w || r->h != de->disp.h) {
		de->pad0 = GetVideoMode();
		if (de->pad0 == MODE_PAL)
			mode |= GP1_DMODE_PAL;
		if (de->isrgb24)
			mode |= GP1_DMODE_RGB24;
		if (de->isinter)
			mode |= GP1_DMODE_INTERLACE;
		if (GEnv.reverse)
			mode |= GP1_DMODE_REVERSE;
		if (de->disp.w > 280) {
			if (de->disp.w <= 352)
				mode |= GP1_DMODE_HRES_320;
			else if (de->disp.w <= 400)
				mode |= GP1_DMODE_HRES_368;
			else if (de->disp.w <= 560)
				mode |= GP1_DMODE_HRES_512;
			else
				mode |= GP1_DMODE_HRES_640;
		}
		fits = de->disp.h <= (de->pad0 ? 288 : 256);
		if (!fits)
			mode |= GP1_DMODE_INTERLACE | GP1_DMODE_VRES_480;
		gpu->ctl(mode);
		de->pad0 = GP1_CTL_DISP_MODE;
	}

	if (GEnv.disp.screen.x != de->screen.x || GEnv.disp.screen.y != de->screen.y ||
	    GEnv.disp.screen.w != de->screen.w || GEnv.disp.screen.h != de->screen.h ||
	    de->pad0 == GP1_CTL_DISP_MODE) {
		int hs, he, vs, ve, res, w;

		de->pad0 = GetVideoMode();
		vs = de->screen.y + (de->pad0 ? 19 : 16);
		ve = vs + (de->screen.h ? de->screen.h : 240);
		if (de->disp.w <= 280)
			res = 0;
		else if (de->disp.w <= 352)
			res = 1;
		else if (de->disp.w <= 400)
			res = 2;
		else if (de->disp.w <= 560)
			res = 3;
		else
			res = 4;
		hs = hRange[de->pad0][res].start + de->screen.x * hClock[res];
		w = hRange[de->pad0][res].end - hRange[de->pad0][res].start;
		if (de->screen.w)
			he = hs + (w * de->screen.w >> 8);
		else
			he = hs + w;

		if (de->pad0) {
			limitRange(hs, 540, 3220);
			limitRange(he, hs + hClock[res] * 4, 3260);
			limitRange(vs, 19, 303);
			limitRange(ve, vs + 2, 305);
		} else {
			limitRange(hs, 500, 3250);
			limitRange(he, hs + hClock[res] * 4, 3290);
			limitRange(vs, 16, 257);
			limitRange(ve, vs + 2, 258);
		}
		g = gpu;
		g->ctl(GP1_DISP_HRANGE | (he & 0xfff) << 12 | hs & 0xfff);
		g = gpu;
		g->ctl(GP1_DISP_VRANGE | (ve & 0x3ff) << 10 | vs & 0x3ff);
	}
	memcpy((DISPENV *)&GEnv.disp, de, sizeof(DISPENV));
	return de;
}

DISPENV *GetDispEnv(DISPENV *de)
{
	memcpy(de, &GEnv.disp, sizeof(DISPENV));
	return de;
}

int GetODE(void)
{
	return gpu->status() >> 31;
}

void SetTexWindow(DR_TWIN *p, RECT *tw)
{
	setlen(p, 2);
	p->code[0] = get_tw(tw);
	p->code[1] = 0;
}

void SetDrawArea(DR_AREA *p, RECT *r)
{
	setlen(p, 2);
	p->code[0] = get_cs(r->x, r->y);
	p->code[1] = get_ce(r->x + r->w - 1, r->y + r->h - 1);
}

void SetDrawOffset(DR_OFFSET *p, u_short *ofs)
{
	setlen(p, 2);
	p->code[0] = get_ofs(ofs[0], ofs[1]);
	p->code[1] = 0;
}

void SetPriority(DR_STP *p, int pbc, int pbw)
{
	setlen(p, 2);
	p->code[0] = (pbc ? (GP0_MASK_BIT | GP0_MASK_CHECK) : GP0_MASK_BIT) | (pbw != 0);
	p->code[1] = 0;
}

void SetDrawStp(DR_STP *p, int pbw)
{
	setlen(p, 2);
	p->code[0] = pbw ? (GP0_MASK_BIT | GP0_MASK_SET) : GP0_MASK_BIT;
	p->code[1] = 0;
}

void SetDrawMode(DR_MODE *p, int dfe, int dtd, int tpage, RECT *tw)
{
	setlen(p, 2);
	p->code[0] = get_mode(dfe, dtd, tpage);
	p->code[1] = get_tw(tw);
}

void SetDrawEnv(DR_ENV *dr_env, DRAWENV *de)
{
	u_long *p = (u_long *)dr_env;
	RECT clip;
	int n;

	p[1] = get_cs(de->clip.x, de->clip.y);
	p[2] = get_ce(de->clip.w + de->clip.x - 1, de->clip.y + de->clip.h - 1);
	p[3] = get_ofs(de->ofs[0], de->ofs[1]);
	p[4] = get_mode(de->dfe, de->dtd, de->tpage);
	p[5] = get_tw(&de->tw);
	p[6] = GP0_MASK_BIT;
	n = 7;
	if (de->isbg) {
		clip.x = de->clip.x;
		clip.y = de->clip.y;
		clip.w = de->clip.w;
		clip.h = de->clip.h;
		limitRange(clip.w, 0, GEnv.w - 1);
		limitRange(clip.h, 0, GEnv.h - 1);
		clip.x -= de->ofs[0];
		clip.y -= de->ofs[1];
		p[n++] = GP0_TILE | de->b0 << 16 | de->g0 << 8 | de->r0;
		p[n++] = *(u_long *)&clip.x;
		p[n++] = *(u_long *)&clip.w;
	}
	setlen(p, n - 1);
}

/* Like SetDrawEnv, but uses GP0_FILL_RECT when clip x and w are 64-pixel aligned. */
static void SetDrawEnvFillRect(DR_ENV *dr_env, DRAWENV *de)
{
	u_long *p = (u_long *)dr_env;
	RECT clip;
	int n;

	p[1] = get_cs(de->clip.x, de->clip.y);
	p[2] = get_ce(de->clip.w + de->clip.x - 1, de->clip.y + de->clip.h - 1);
	p[3] = get_ofs(de->ofs[0], de->ofs[1]);
	p[4] = get_mode(de->dfe, de->dtd, de->tpage);
	p[5] = get_tw(&de->tw);
	p[6] = GP0_MASK_BIT;
	n = 7;
	if (de->isbg) {
		clip.x = de->clip.x;
		clip.y = de->clip.y;
		clip.w = de->clip.w;
		clip.h = de->clip.h;
		limitRange(clip.w, 0, GEnv.w - 1);
		limitRange(clip.h, 0, GEnv.h - 1);
		if ((clip.x & 0x3f) || (clip.w & 0x3f)) {
			clip.x -= de->ofs[0];
			clip.y -= de->ofs[1];
			p[n++] = GP0_TILE | de->b0 << 16 | de->g0 << 8 | de->r0;
			p[n++] = *(u_long *)&clip.x;
			p[n++] = *(u_long *)&clip.w;
		} else {
			p[n++] = GP0_FILL_RECT | de->b0 << 16 | de->g0 << 8 | de->r0;
			p[n++] = *(u_long *)&clip.x;
			p[n++] = *(u_long *)&clip.w;
		}
	}
	setlen(p, n - 1);
}

static u_long get_mode(int dfe, int dtd, u_short tpage)
{
	return GP0_DRAW_MODE | (dtd ? GP0_MODE_DTD : 0) |
		(dfe ? GP0_MODE_DFE : 0) | (tpage & GP0_MODE_TPAGE_MASK);
}

static u_long get_cs(short x, short y)
{
	limitRange(x, 0, GEnv.w - 1);
	limitRange(y, 0, GEnv.h - 1);
	return GP0_DRAW_AREA_TL | (y & 0x3ff) << 10 | x & 0x3ff;
}

static u_long get_ce(short x, short y)
{
	limitRange(x, 0, GEnv.w - 1);
	limitRange(y, 0, GEnv.h - 1);
	return GP0_DRAW_AREA_BR | (y & 0x3ff) << 10 | x & 0x3ff;
}

static u_long get_ofs(short x, short y)
{
	return GP0_DRAW_OFFSET | (y & 0x7ff) << 11 | x & 0x7ff;
}

static u_long get_tw(RECT *tw)
{
	u_long pat[4];

	if (tw) {
		pat[0] = (tw->x & 0xff) >> 3;
		pat[2] = (-tw->w & 0xff) >> 3;
		pat[1] = (tw->y & 0xff) >> 3;
		pat[3] = (-tw->h & 0xff) >> 3;
		return GP0_TEX_WINDOW | pat[1] << 15 | pat[0] << 10 | pat[3] << 5 | pat[2];
	}
	return 0;
}

static volatile u_long *GPU_DATA = (u_long *)GPU_DATA_ADDR;
static volatile u_long *GPU_STATUS = (u_long *)GPU_STATUS_ADDR;
static volatile u_long *DMA2_MADR = (u_long *)DMA2_MADR_ADDR;
static volatile u_long *DMA2_BCR = (u_long *)DMA2_BCR_ADDR;
static volatile u_long *DMA2_CHCR = (u_long *)DMA2_CHCR_ADDR;
static volatile u_long *DMA6_MADR = (u_long *)DMA6_MADR_ADDR;
static volatile u_long *DMA6_BCR = (u_long *)DMA6_BCR_ADDR;
static volatile u_long *DMA6_CHCR = (u_long *)DMA6_CHCR_ADDR;
static volatile u_long *DMA_DPCR = (u_long *)DMA_DPCR_ADDR;

volatile int _qin = 0;
volatile int _qout = 0;
static int que_mask = 0;
static int exe_mask = 0;
static int reset_mask = 0;
static int alarm_time = 0;
static int alarm_count = 0;

static u_char ctlbuf[256];
volatile GpuQueueEntry _que[GPU_QUE_SIZE];

static u_long _status(void)
{
	return *GPU_STATUS;
}

static int _otc(u_long *ot, int n)
{
	*DMA_DPCR |= DMA_DPCR_OTC_EN;
	*DMA6_CHCR = 0;
	*DMA6_MADR = (u_long)&ot[n - 1];
	*DMA6_BCR = n;
	*DMA6_CHCR = DMA_CHCR_OTC_CLEAR;
	set_alarm();
	while (*DMA6_CHCR & DMA_CHCR_BUSY) {
		if (get_alarm())
			return -1;
	}
	return n;
}

static int _clr(RECT *rect, u_long color)
{
	static u_long p0[9];
	static u_long p1[8];

	limitRange(rect->w, 0, GEnv.w - 1);
	limitRange(rect->h, 0, GEnv.h - 1);
	if ((rect->x & 0x3f) || (rect->w & 0x3f)) {
		p0[0] = GPU_DMA_TAG(8, p1);
		p0[1] = GP0_DRAW_AREA_TL;
		p0[2] = GP0_DRAW_AREA_BR | GPU_INFO_MASK;
		p0[3] = GP0_DRAW_OFFSET;
		p0[4] = GP0_MASK_BIT;
		p0[5] = GP0_DRAW_MODE | (*GPU_STATUS & GPUSTAT_DRAW_MODE_MASK) | ((color >> 31) << 10);
		p0[6] = GP0_TILE | (color & GPU_COLOR_MASK);
		p0[7] = *(u_long *)&rect->x;
		p0[8] = *(u_long *)&rect->w;
		p1[0] = GPU_DMA_TAG(3, GPU_DMA_END);
		p1[1] = GP0_DRAW_AREA_TL | _param(GP1_INFO_DRAW_AREA_TL);
		p1[2] = GP0_DRAW_AREA_BR | _param(GP1_INFO_DRAW_AREA_BR);
		p1[3] = GP0_DRAW_OFFSET | _param(GP1_INFO_DRAW_OFFSET);
	} else {
		p0[0] = GPU_DMA_TAG(5, GPU_DMA_END);
		p0[1] = GP0_MASK_BIT;
		p0[2] = GP0_DRAW_MODE | (*GPU_STATUS & GPUSTAT_DRAW_MODE_MASK) | ((color >> 31) << 10);
		p0[3] = GP0_FILL_RECT | (color & GPU_COLOR_MASK);
		p0[4] = *(u_long *)&rect->x;
		p0[5] = *(u_long *)&rect->w;
	}
	_cwc(p0);
	return 0;
}

static int _dws(RECT *rect, u_long *p)
{
	int size, n;
	int blocks;
	/* always 0; the original still tests it */
	int mode = 0;

	set_alarm();
	limitRange(rect->w, 0, GEnv.w);
	limitRange(rect->h, 0, GEnv.h);
	size = (rect->w * rect->h + 1) / 2;
	if (size <= 0)
		return -1;
	n = size % GPU_DMA_BLOCK_WORDS;
	blocks = size / GPU_DMA_BLOCK_WORDS;
	while (!(*GPU_STATUS & GPUSTAT_CMD_READY)) {
		if (get_alarm())
			return -1;
	}
	*GPU_STATUS = GP1_DMA_OFF;
	*GPU_DATA = GP0_CLEAR_CACHE;
	*GPU_DATA = mode ? GP0_CPU_TO_VRAM_ALT : GP0_CPU_TO_VRAM;
	*GPU_DATA = *(u_long *)&rect->x;
	*GPU_DATA = *(u_long *)&rect->w;
	while (n--)
		*GPU_DATA = *p++;
	if (blocks) {
		*GPU_STATUS = GP1_DMA_CPU_TO_GP0;
		*DMA2_MADR = (u_long)p;
		*DMA2_BCR = blocks << 16 | GPU_DMA_BLOCK_WORDS;
		*DMA2_CHCR = DMA_CHCR_GPU_WRITE;
	}
	return 0;
}

static int _drs(RECT *rect, u_long *p)
{
	int size, n;
	int blocks;

	set_alarm();
	limitRange(rect->w, 0, GEnv.w);
	limitRange(rect->h, 0, GEnv.h);
	size = (rect->w * rect->h + 1) / 2;
	if (size <= 0)
		return -1;
	n = size % GPU_DMA_BLOCK_WORDS;
	blocks = size / GPU_DMA_BLOCK_WORDS;
	while (!(*GPU_STATUS & GPUSTAT_CMD_READY)) {
		if (get_alarm())
			return -1;
	}
	*GPU_STATUS = GP1_DMA_OFF;
	*GPU_DATA = GP0_CLEAR_CACHE;
	*GPU_DATA = GP0_VRAM_TO_CPU;
	*GPU_DATA = *(u_long *)&rect->x;
	*GPU_DATA = *(u_long *)&rect->w;
	while (!(*GPU_STATUS & GPUSTAT_VRAM_READY)) {
		if (get_alarm())
			return -1;
	}
	while (n--)
		*p++ = *GPU_DATA;
	if (blocks) {
		*GPU_STATUS = GP1_DMA_GPU_TO_CPU;
		*DMA2_MADR = (u_long)p;
		*DMA2_BCR = blocks << 16 | GPU_DMA_BLOCK_WORDS;
		*DMA2_CHCR = DMA_CHCR_GPU_READ;
	}
	return 0;
}

static void _ctl(u_long cmd)
{
	*GPU_STATUS = cmd;
	ctlbuf[cmd >> 24] = cmd;
}

static int _getctl(int n)
{
	return ctlbuf[n];
}

static int _cwb(u_long *p, int n)
{
	*GPU_STATUS = GP1_DMA_OFF;
	while (n--)
		*GPU_DATA = *p++;
	return 0;
}

static void _cwc(u_long *p)
{
	*GPU_STATUS = GP1_DMA_CPU_TO_GP0;
	*DMA2_MADR = (u_long)p;
	*DMA2_BCR = 0;
	*DMA2_CHCR = DMA_CHCR_GPU_LIST;
}

static int _param(u_long n)
{
	*GPU_STATUS = GP1_GET_INFO | n;
	return *GPU_DATA & GPU_INFO_MASK;
}

static int _addque(void (*func)(), u_long *param, u_long value)
{
	return _addqueCopy(func, param, 0, value);
}

static int _addqueCopy(void (*func)(), u_long *param, int size, u_long value)
{
	int i;

	set_alarm();
	while (((_qin + 1) & GPU_QUE_MASK) == _qout) {
		if (get_alarm())
			return -1;
		_exeque();
	}
	que_mask = SetIntrMask(0);
	GEnv.syncflag = 1;
	if (GEnv.que == 0 ||
	    (_qin == _qout && !(*DMA2_CHCR & DMA_CHCR_BUSY) && GEnv.drawsync == 0)) {
		while (!(*GPU_STATUS & GPUSTAT_CMD_READY))
			;
		(*func)(param, value);
		SetIntrMask(que_mask);
		return 0;
	}
	DMACallback(DMA_GPU, (void (*)())_exeque);
	if (size) {
		for (i = 0; i < size / 4; i++)
			_que[_qin].buf[i] = param[i];
		_que[_qin].param = _que[_qin].buf;
	} else {
		_que[_qin].param = param;
	}
	_que[_qin].value = value;
	_que[_qin].func = func;
	_qin = (_qin + 1) & GPU_QUE_MASK;
	SetIntrMask(que_mask);
	_exeque();
	return (_qin - _qout) & GPU_QUE_MASK;
}

static int _exeque(void)
{
	if (*DMA2_CHCR & DMA_CHCR_BUSY)
		return 1;
	exe_mask = SetIntrMask(0);
	while (_qin != _qout && !(*DMA2_CHCR & DMA_CHCR_BUSY)) {
		if (((_qout + 1) & GPU_QUE_MASK) == _qin && GEnv.drawsync == 0)
			DMACallback(DMA_GPU, 0);
		while (!(*GPU_STATUS & GPUSTAT_CMD_READY))
			;
		(*_que[_qout].func)(_que[_qout].param, _que[_qout].value);
		_qout = (_qout + 1) & GPU_QUE_MASK;
	}
	SetIntrMask(exe_mask);
	if (_qin == _qout && !(*DMA2_CHCR & DMA_CHCR_BUSY) && GEnv.syncflag && GEnv.drawsync) {
		GEnv.syncflag = 0;
		(*GEnv.drawsync)();
	}
	return (_qin - _qout) & GPU_QUE_MASK;
}

static int _reset(int mode)
{
	reset_mask = SetIntrMask(0);
	_qin = _qout = 0;
	switch (mode & 7) {
	case 0:
	case 5:
		*DMA2_CHCR = DMA_CHCR_GPU_IDLE;
		*DMA_DPCR |= DMA_DPCR_GPU_EN;
		*GPU_STATUS = GP1_RESET;
		memset((char *)ctlbuf, 0, sizeof(ctlbuf));
		memset((char *)_que, 0, sizeof(_que));
		break;
	case 1:
	case 3:
		*DMA2_CHCR = DMA_CHCR_GPU_IDLE;
		*DMA_DPCR |= DMA_DPCR_GPU_EN;
		*GPU_STATUS = GP1_ACK_IRQ;
		*GPU_STATUS = GP1_RESET_FIFO;
		break;
	}
	SetIntrMask(reset_mask);
	return (mode & 7) ? 0 : _version(mode);
}

static int _sync(int mode)
{
	int n;

	if (mode == 0) {
		set_alarm();
		while (_qin != _qout) {
			_exeque();
			if (get_alarm())
				return -1;
		}
		while ((*DMA2_CHCR & DMA_CHCR_BUSY) || !(*GPU_STATUS & GPUSTAT_CMD_READY)) {
			if (get_alarm())
				return -1;
		}
		return 0;
	}
	n = (_qin - _qout) & GPU_QUE_MASK;
	if (n)
		_exeque();
	if ((*DMA2_CHCR & DMA_CHCR_BUSY) || !(*GPU_STATUS & GPUSTAT_CMD_READY))
		return n ? n : 1;
	return n;
}

static void set_alarm(void)
{
	alarm_time = VSync(-1) + GPU_ALARM_VSYNC;
	alarm_count = 0;
}

static int get_alarm(void)
{
	if (alarm_time < VSync(-1) || alarm_count++ > GPU_ALARM_LIMIT) {
		*GPU_STATUS;
		printf("GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x
",
			(_qin - _qout) & GPU_QUE_MASK, *GPU_STATUS, *DMA2_CHCR, *DMA2_MADR);
		reset_mask = SetIntrMask(0);
		_qin = _qout = 0;
		*DMA2_CHCR = DMA_CHCR_GPU_IDLE;
		*DMA_DPCR |= DMA_DPCR_GPU_EN;
		*GPU_STATUS = GP1_ACK_IRQ;
		*GPU_STATUS = GP1_RESET_FIFO;
		SetIntrMask(reset_mask);
		return -1;
	}
	return 0;
}

static int _version(int mode)
{
	*GPU_STATUS = GP1_GET_INFO | GP1_INFO_GPU_TYPE;
	if ((*GPU_DATA & GPU_INFO_MASK) != 2) {
		*GPU_DATA = GP0_DRAW_MODE | GP0_MODE_2M_DFE | (*GPU_STATUS & GPUSTAT_2M_DRAW_MODE_MASK);
		*GPU_DATA;
		return 0;
	}
	if (!(mode & 8))
		return 1;
	*GPU_STATUS = GP1_EXT_VRAM;
	return 2;
}

static void restore_exeque(void);

/* Immediate transfers abort the caller if the queued DMA channel never clears. */
#define WAIT_GPU_TRANSFER() \
	do { \
		alarm_time = VSync(-1) + GPU_ALARM_VSYNC; \
		alarm_count = 0; \
		while ((*DMA2_CHCR & DMA_CHCR_BUSY) || !(*GPU_STATUS & GPUSTAT_CMD_READY)) { \
			if (get_alarm()) \
				return -1; \
		} \
		DMACallback(DMA_GPU, restore_exeque); \
	} while (0)

int LoadImage2(RECT *rect, u_long *p)
{
	checkRECT("LoadImage2", rect);
	WAIT_GPU_TRANSFER();
	gpu->dws(rect, p);
	return 0;
}

int StoreImage2(RECT *rect, u_long *p)
{
	checkRECT("StoreImage", rect);
	WAIT_GPU_TRANSFER();
	gpu->drs(rect, p);
	return 0;
}

int MoveImage2(RECT *rect, int x, int y)
{
	checkRECT("MoveImage", rect);
	WAIT_GPU_TRANSFER();
	if (rect->w == 0 || rect->h == 0)
		return -1;
	move_packet[2] = *(u_long *)&rect->x;
	move_packet[3] = (y << 16) | (x & 0xffff);
	move_packet[4] = *(u_long *)&rect->w;
	gpu->cwc(move_packet);
	return 0;
}

int DrawOTag2(u_long *p)
{
	if (GEnv.debug >= 2)
		GPU_printf("DrawOTag(%08x)...\n", p);
	WAIT_GPU_TRANSFER();
	gpu->cwc(p);
	return 0;
}

static void restore_exeque(void)
{
	DMACallback(DMA_GPU, (void (*)())_exeque);
}

static void memset(char *p, int c, int n)
{
	while (n--)
		*p++ = c;
}
