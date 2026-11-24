/*
 * LIBPAD's internal interface: the per-port controller record, the SIO0 registers and the
 * entry points its modules share.
 */
#ifndef _PAD_INT_H_
#define _PAD_INT_H_

typedef struct PadActuator {
    unsigned char function;
    unsigned char subfunction;
    unsigned char size;
    unsigned char current;
    unsigned char signal;
} PadActuator;

typedef struct PadActCombination {
    unsigned char count;
    unsigned char *indices;
} PadActCombination;

/* One controller's state, 0xF0 bytes; PADPORTD.c holds the two ports' records. */
typedef struct PadState PadState;
struct PadState {
    unsigned short *modeIds;
    PadActuator *actuators;
    PadActCombination *combinations;
    PadState *sockets;
    PadState *parent;
    void (*prepareCommand)(PadState *pad);
    int (*readCommandReply)(PadState *pad);
    unsigned char unused1c[4];      /* nothing reads the unused fields */
    unsigned char *requestedAlignment;
    unsigned char commandParam;
    unsigned char *actuatorValues;
    unsigned char *commandData;
    unsigned char *resultBuffer;
    unsigned char actuatorValueCount;
    unsigned char resultLength;
    unsigned char commandDataLength;
    unsigned char command;
    unsigned char commandPending;
    unsigned char parentBusy;
    unsigned char *rxBuffer;
    unsigned char *txBuffer;
    unsigned char rxLength;
    unsigned char txLength;
    unsigned char configStep;
    unsigned char commandIndex;
    unsigned char combinationBytesLeft;
    unsigned char configState;
    unsigned char retries;
    unsigned int timeoutCount;
    unsigned char forceInfoRefresh;
    unsigned char requestedMode[3];
    unsigned char unused54[3];
    unsigned char actuatorEnabled[6];
    unsigned char actuatorAlignment[6];
    unsigned char infoBuffer[128];
    unsigned char modeCount;
    unsigned char modeOffset;
    unsigned short modeExtendedId;
    unsigned char controllerType;
    unsigned char actuatorCount;
    unsigned char combinationCount;
    unsigned char configReplyCount;
    unsigned short combinationDataSize;
    unsigned short confirmValue;
};

/* The SIO0 controller and memory-card port, at 0x1f801040. */
typedef struct SerialRegisters {
    volatile unsigned char data;
    unsigned char unused01[3];   /* nothing reads or writes the unused fields */
    volatile unsigned short status;
    unsigned short unused06;
    volatile unsigned short mode;
    volatile unsigned short control;
    unsigned short unused0c;
    volatile unsigned short baud;
} SerialRegisters;


/* PADMAIN.c */
extern void (*_padFuncNextPort)(int result);
extern void (*_padFuncClrInfo)(PadState *pad);
extern int (*_padFuncGetTxd)(PadState *pad, int tap);
extern void (*_padFuncCurrLimit)(PadState *pad);
extern int (*_padFuncPtr2Port)(PadState *pad);
extern PadState *(*_padFuncPort2Info)(int port);
extern int (*_padFuncSendAuto)(PadState *pad);
extern int (*_padFuncChkEng)(PadState *pad);
extern void (*_padFuncRecvAuto)(PadState *pad);
extern void (*_padFuncClrCmdNo)(PadState *pad);
extern void (*_padFuncIntGun)(void);
extern void (*_padFuncSetGunPort)(void);
extern void (*_padFuncGetGunPos)(PadState *pad);
extern PadState *_padInfoDir;
extern int _padIntExec;
extern int _padGunExec;
extern int _padSioChan;
extern int _padSioState;
extern int _padTotalCurr;
extern int _padModeMtap;
extern int _padChanStart;
extern int _padChanStop;
extern int _padFixResult[2];

void _padSetVsyncParam(void);
int _padChkVsync(void);
void _padStartCom(void);
void _padStopCom(void);
int _padInitSioMode(PadState *pad);
int _padSioRW(PadState *pad, int tx);
int _padSioRW2(PadState *pad, int tx);
int _padClrIntSio0(void);
void _padWaitRXready(void);

/* PADIF.c */
extern int (*padIntFunc[])(PadState *pad);

/* PADCMD.c */
void _padSetAct(PadState *pad, unsigned char *values, int count);
void _padSetCmd(PadState *pad, unsigned char cmd, unsigned char *data, int length);
void _padSendAtLoadInfo(PadState *pad);
int _padRecvAtLoadInfo(PadState *pad);
int _padLoadActInfo(PadState *pad, unsigned char *buffer);
int _padGetActSize(PadState *pad);
int _padSetActAlign(PadState *pad, unsigned char *alignment);
int _padSetMainMode(PadState *pad, unsigned char offset, unsigned char lock);
void _padCmdParaMode(PadState *pad, int enter);

/* PADSEQD.c */
void _padInitDirSeq(void);
void _dirFailAuto(PadState *pad, int result);

/* WAITRC2.c */
extern unsigned long _startTime;
extern unsigned long _waitTime;

void setRC2wait(unsigned long wait);
int chkRC2wait(void);

#endif
