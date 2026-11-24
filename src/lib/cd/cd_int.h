/*
 * Internal hardware definitions, streaming state, and interfaces for LIBCD.
 */
#ifndef _CD_INT_H_
#define _CD_INT_H_

#include <sys/types.h>
#include <libcd.h>
#include "psx_hw.h"

/* Drive commands libcd.h does not define; CdlReset is the name the module's string table gives
 * command 0x0a */
#define CdlReset		0x0a
#define CD_CMD_TEST		0x19

/* Drive status bit 3 (ID error), and the bits a command's first result byte is failed by */
#define CD_STAT_ID_ERROR	0x08
#define CD_STAT_ERROR_BITS	(CdlStatShellOpen | CD_STAT_ID_ERROR | CdlStatSeekError | CdlStatError)

/* The value written to CD_REG0 selects which of four register banks the other three ports reach */
#define CD_BANK_COMMAND		0	/* command, parameters, request */
#define CD_BANK_INTR		1	/* interrupt enable and flags */
#define CD_BANK_ATV_LEFT	2	/* audio volume from the left channel */
#define CD_BANK_ATV_RIGHT	3	/* audio volume from the right channel, and apply */

#define CD_STAT_INDEX_MASK	0x03	/* the bank selected */
#define CD_STAT_RSLRRDY		0x20	/* response FIFO not empty */
#define CD_STAT_DRQSTS		0x40	/* data FIFO not empty */
#define CD_REQ_CLEAR		0x00
#define CD_REQ_BFRD		0x80	/* request the sector's data */
#define CD_INT_MASK		0x07
#define CD_ADVC_APPLY		0x20	/* apply the audio volume written */
#define CD_ATV_UNITY		0x80	/* full volume in the audio volume matrix */
#define CD_RESULT_SIZE		8	/* response FIFO depth in bytes */
#define CD_COM_MAX		32
#define CD_INT_MAX		8

/* timings written to the bus delay registers */
#define CDROM_DELAY_DMA		0x20943
#define CDROM_DELAY_DEFAULT	0x20843
#define CDROM_DELAY_CHOP	0x21020843
#define COM_DELAY_DMA		0x1323
#define COM_DELAY_DEFAULT	0x1325

#define DMA_DPCR_CDROM_EN	0x00008000	/* DMA3 (CD-ROM) enable */
#define DMA_BCR_SINGLE_BLOCK	0x00010000	/* one block */
#define DMA_CHCR_CDROM_READ	0x11000000	/* CD-ROM -> RAM, start, trigger */
#define DMA_CHCR_CDROM_CHOP	0x11400100	/* the same, chopped with a 16-cycle CPU window */
#define DMA_WAIT_TIMEOUT	0x10000

#define SPUCNT_CD_INIT		(SPUCNT_ENABLE | SPUCNT_UNMUTE | SPUCNT_CD_MIX)
#define SPU_VOL_MAX		0x3fff

#define CD_ALARM_VSYNC		960	/* in vertical blanks */
#define CD_ALARM_LIMIT		0x3c0000

/* event.c defines these */
extern CdlCB CD_cbread;
extern int CD_read_dma_mode;

/* bios.c defines these */
extern CdlCB CD_cbsync;
extern CdlCB CD_cbready;
extern int CD_debug;
extern int CD_status;
extern int CD_status1;
extern int CD_nopen;
extern int DS_active;	/* at 1 StUnSetRing clears libds's callbacks; no routine in this program writes it */
extern CdlLOC CD_pos;
extern u_char CD_mode;
extern u_char CD_com;
extern char *CD_comstr[CD_COM_MAX];
extern char *CD_intstr[CD_INT_MAX];

extern int CD_sync(int mode, u_char *result);
extern int CD_ready(int mode, u_char *result);
extern int CD_cw(u_char com, u_char *param, u_char *result, int async);
extern int CD_vol(CdlATV *vol);
extern void CD_flush(void);
extern int CD_initvol(void);
extern void CD_initintr(void);
extern int CD_init(void);
extern int CD_datasync(int mode);
extern int CD_getsector(void *madr, int size);
extern int CD_getsector2(void *madr, int size);
extern void CD_set_test_parmnum(int n);

/* StMode: the stream's sector format, chosen by CdRead2 from CdlModeSize1 */
#define ST_MODE_2340		0	/* 2340-byte sectors with header and subheader */
#define ST_MODE_2048		1

#define ST_HEADER_VERSION	0x0160	/* HeaderArea[StVER] of a stream sector */
#define ST_TYPE_CHANNEL_SHIFT	10	/* the stream channel in HeaderArea[StTYPE] */
#define ST_TYPE_CHANNEL_MASK	0x1f

extern u_long StMode;
extern void init_ring_status(int start, u_long n);
extern void data_ready_callback(void);

/* StSetStream's two callbacks are commons of its own module (c_005.c): the linker places a common
 * with the first module in link order that defines it. */
extern void (*StFunc1)();	/* frame-complete callback */
extern void (*StFunc2)();	/* called at the end frame, or when a frame would pass the ring's end and
				 * no end frame is set */

#ifdef CD_STREAM_MODULE
/* Each streaming module's object holds these four pointers in its own .data and these eighteen
 * commons: every one of those modules defines them. */
static volatile u_char *CD_REG0 = (volatile u_char *)CDROM_REG0_ADDR;
static volatile u_char *CD_REG1 = (volatile u_char *)CDROM_REG1_ADDR;
static volatile u_char *CD_REG2 = (volatile u_char *)CDROM_REG2_ADDR;
static volatile u_char *CD_REG3 = (volatile u_char *)CDROM_REG3_ADDR;

u_long Stframe_no;
u_long StFinalSector;
short Stsector_offset;
u_long StRgb24;		/* 0: 16-bit MDEC stream; 1: 24-bit */
u_long StEmu_Idx;	/* sector index in the RAM stream StSetEmulate gives */
u_long StCdIntrFlag;
u_long StRingIdx1;	/* the ring slot StCdInterrupt fills next */
u_long StRingIdx2;	/* the first slot of the frame being filled */
u_long StRingIdx3;	/* the slot StGetNext reads next */
u_long StStartFrame;	/* frame to wait for while StSTART_FLAG is 1 (0: none) */
u_long StCHANNEL;	/* requested channel, taken at the end of a frame */
u_long *StEmu_Addr;	/* RAM stream from StSetEmulate */
u_long StEndFrame;	/* frames numbered this or higher are dropped (0: none) */
u_long StSTART_FLAG;	/* 1: drop sectors until frame StStartFrame arrives */
u_long *StRingAddr;
u_long *StRingBase;	/* where the current sector's data goes */
u_long StRingSize;
u_long CChannel;	/* the channel being read */
#endif /* CD_STREAM_MODULE */

#endif /* _CD_INT_H_ */
