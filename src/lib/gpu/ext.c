/*
 * Texture page and CLUT loaders, and default drawing/display environments.
 */
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>

u_short LoadTPage(u_long *pix, int tp, int abr, int x, int y, int w, int h)
{
	RECT rect;

	rect.x = x;
	rect.y = y;
	rect.h = h;
	switch (tp) {
	case 0: /* 4-bit CLUT: 4 texels per 16-bit pixel */
		rect.w = w / 4;
		break;
	case 1: /* 8-bit CLUT: 2 texels per 16-bit pixel */
		rect.w = w / 2;
		break;
	case 2: /* 16-bit direct */
		rect.w = w;
		break;
	}
	LoadImage(&rect, pix);
	return GetTPage(tp, abr, x, y);
}

u_short LoadClut(u_long *clut, int x, int y)
{
	RECT rect;

	setRECT(&rect, x, y, 256, 1);
	LoadImage(&rect, clut);
	return GetClut(x, y);
}

u_short LoadClut2(u_long *clut, int x, int y)
{
	RECT rect;

	setRECT(&rect, x, y, 16, 1);
	LoadImage(&rect, clut);
	return GetClut(x, y);
}

DRAWENV *SetDefDrawEnv(DRAWENV *env, int x, int y, int w, int h)
{
	long mode = GetVideoMode();

	setRECT(&env->clip, x, y, w, h);
	setRECT(&env->tw, 0, 0, 0, 0);
	setRGB0(env, 0, 0, 0);
	env->dtd = 1;
	env->dfe = h <= (mode != MODE_NTSC ? 288 : 256);
	env->ofs[0] = x;
	env->ofs[1] = y;
	env->tpage = getTPage(0, 0, 640, 0);
	env->isbg = 0;
	return env;
}

DISPENV *SetDefDispEnv(DISPENV *env, int x, int y, int w, int h)
{
	setRECT(&env->disp, x, y, w, h);
	setRECT(&env->screen, 0, 0, 0, 0);
	env->isrgb24 = 0;
	env->isinter = 0;
	env->pad1 = 0;
	env->pad0 = 0;
	return env;
}
