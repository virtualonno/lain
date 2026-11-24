#include <sys/types.h>
#include <libapi.h>
#include <libpad.h>
#include "etc_int.h"
#include "../api/api_int.h"
#include "pad_int.h"

/* 8 bytes no code reads. */
static unsigned long unusedWords[2] = { 0x280c7350, 0x1043062c };
void (*_padFuncNextPort)(int result) = 0;
void (*_padFuncClrInfo)(PadState *pad) = 0;
int (*_padFuncGetTxd)(PadState *pad, int tap) = 0;
void (*_padFuncCurrLimit)(PadState *pad) = 0;
int (*_padFuncPtr2Port)(PadState *pad) = 0;
PadState *(*_padFuncPort2Info)(int port) = 0;
int (*_padFuncSendAuto)(PadState *pad) = 0;
int (*_padFuncChkEng)(PadState *pad) = 0;
void (*_padFuncRecvAuto)(PadState *pad) = 0;
void (*_padFuncClrCmdNo)(PadState *pad) = 0;
void (*_padFuncIntGun)(void) = 0;
void (*_padFuncSetGunPort)(void) = 0;
void (*_padFuncGetGunPos)(PadState *pad) = 0;
PadState *_padInfoDir = 0;
int _padIntExec = 0;
int _padGunExec = 0;
int _padSioChan = 0;
int _padSioState = 0;
int _padTotalCurr = 0;
int _padModeMtap = 0;
int _padChanStart = 0;
int _padChanStop = 1;
int _padFixResult[2] = { -1, -1 };
static volatile struct IntrReg *intr = (struct IntrReg *)INTR_STAT_ADDR;
static SerialRegisters *sio = (SerialRegisters *)SIO0_REG_BASE;
static int vsyncSeen = 0;
/* 12 bytes no code reads. */
static int unusedZeros[3] = { 0, 0, 0 };
static struct IntrChainEntry vsyncChainEntry;
/* [2] and [3]: nothing reads them. */
static int disabledVsyncs[4];

#define MAX_DISABLED_VSYNCS 150
#define SIO_CONTROL_TX_ENABLE 1
#define SIO_CONTROL_DTR 2
#define SIO_CONTROL_ACK 0x10
#define SIO_CONTROL_RESET 0x40
#define SIO_CONTROL_ACK_IRQ 0x1000
#define SIO_CONTROL_PORT2 0x2000
#define SIO_CONTROL_SELECT_PORT1 (SIO_CONTROL_ACK_IRQ | SIO_CONTROL_DTR | SIO_CONTROL_TX_ENABLE)
#define SIO_CONTROL_SELECT_PORT2 (SIO_CONTROL_PORT2 | SIO_CONTROL_SELECT_PORT1)
#define SIO_MODE_8BIT_X1 13 /* 8-bit characters, baud factor 1 */
#define SIO_STANDARD_BAUD 0x88
#define SIO_STATUS_RX_READY 2
#define SIO_STATUS_TX_READY 1
#define SIO_MULTITAP_BAUD 0x22
#define RC2_COUNT (*(volatile unsigned short *)RCNT2_COUNT_ADDR)
#define RC2_MODE (*(volatile unsigned short *)RCNT2_MODE_ADDR)
#define RC2_TARGET (*(volatile unsigned short *)RCNT2_TARGET_ADDR)

#define SIO_STATUS_DSR 0x80
#define SIO_STATUS_IRQ 0x200

static void advanceSioTransfer(PadState *pad);
static long testVsync(void);
static long pollPadsOnVsync(void);

unsigned PadEnableCom(unsigned mask)
{
    unsigned previous = (_padChanStop << 1) | (_padChanStart == 0);

    if (previous == mask)
        return previous;

    _padIntExec = 0;
    if (mask & 1) {
        _padChanStart = 0;
        if (disabledVsyncs[0] >= MAX_DISABLED_VSYNCS)
            _padFuncClrInfo(&_padInfoDir[0]);
        disabledVsyncs[0] = 0;
    } else {
        _padChanStart = 1;
    }
    if (mask & 2) {
        _padChanStop = 1;
        if (disabledVsyncs[1] >= MAX_DISABLED_VSYNCS)
            _padFuncClrInfo(&_padInfoDir[1]);
        disabledVsyncs[1] = 0;
    } else {
        _padChanStop = 0;
    }
    _padIntExec = 1;
    return previous;
}

void _padSetVsyncParam(void)
{
    vsyncChainEntry.handler = pollPadsOnVsync;
    vsyncChainEntry.check = testVsync;
    vsyncChainEntry.next = 0;
    vsyncChainEntry.unused = 0;
}

static long testVsync(void)
{
    if (!(intr->mask & INTR_MASK_VBLANK))
        return 0;
    if (!(intr->stat & INTR_MASK_VBLANK))
        return 0;
    if (_padFuncIntGun)
        _padFuncIntGun();
    return 1;
}

static long pollPadsOnVsync(void)
{
    int channel;

    if (sio->control & SIO_CONTROL_DTR) {
        sio->control = 0;
        return 0;
    }

    vsyncSeen = 1;
    if (_padChanStart && disabledVsyncs[0] < MAX_DISABLED_VSYNCS)
        disabledVsyncs[0]++;
    if (!_padChanStop && disabledVsyncs[1] < MAX_DISABLED_VSYNCS)
        disabledVsyncs[1]++;
    if (_padIntExec) {
        channel = _padChanStart;
        if (channel <= _padChanStop) {
            _padSioState = 0;
            _padSioChan = channel;
            if (!_padInitSioMode(&_padInfoDir[channel]))
                _padFuncNextPort(0xffff);
            _padTotalCurr = 0;
            while (_padSioChan <= _padChanStop)
                advanceSioTransfer(&_padInfoDir[_padSioChan]);
            sio->baud = SIO_STANDARD_BAUD;
        }
    }
    return 0;
}

int _padChkVsync(void)
{
    int seen = vsyncSeen;
    vsyncSeen = 0;
    return seen;
}

void _padStartCom(void)
{
    _padIntExec = 0;
    EnterCriticalSection();
    SysDeqIntRP(2, &vsyncChainEntry);
    SysEnqIntRP(2, &vsyncChainEntry);
    intr->stat = ~INTR_MASK_VBLANK;
    intr->mask |= INTR_MASK_VBLANK;
    ChangeClearRCnt(3, 0);
    ExitCriticalSection();
    _padFuncClrInfo(&_padInfoDir[0]);
    _padFuncClrInfo(&_padInfoDir[1]);
    disabledVsyncs[0] = disabledVsyncs[1] = 0;
    _padIntExec = 1;
}

void _padStopCom(void)
{
    EnterCriticalSection();
    ChangeClearRCnt(3, 1);
    SysDeqIntRP(2, &vsyncChainEntry);
    ExitCriticalSection();
}

int _padInitSioMode(PadState *pad)
{
    sio->control = SIO_CONTROL_RESET;
    sio->control = 0;
    sio->mode = SIO_MODE_8BIT_X1;
    sio->baud = SIO_STANDARD_BAUD;
    if (pad->controllerType == 8)
        setRC2wait(80);
    else
        setRC2wait(145);

    sio->control = _padSioChan ? SIO_CONTROL_SELECT_PORT2 : SIO_CONTROL_SELECT_PORT1;
    if (_padFixResult[_padSioChan] >= 0) {
        while (_padFixResult[_padSioChan] > 0) {
            --_padFixResult[_padSioChan];
            _padFuncRecvAuto(&pad->sockets[_padFixResult[_padSioChan]]);
        }
        if (_padFixResult[_padSioChan] == 0) {
            _padFixResult[_padSioChan] = -1;
            _padFuncRecvAuto(pad);
            _padFuncClrCmdNo(pad);
        }
    }

    if (sio->status & SIO_STATUS_IRQ) {
        sio->control |= SIO_CONTROL_ACK;
        if (sio->status & SIO_STATUS_IRQ) {
            while (!chkRC2wait())
                ;
            sio->data = 1;
            setRC2wait(100);
            if (!_padClrIntSio0())
                return 0;
            _padWaitRXready();
            (void)sio->data;
            setRC2wait(430);
            while (!(intr->stat & INTR_MASK_PAD_MEMCARD)) {
                if (chkRC2wait())
                    return 0;
            }
            sio->data = 0x42;
            setRC2wait(60);
            if (!_padClrIntSio0())
                return 0;
            _padWaitRXready();
            (void)sio->data;
            setRC2wait(430);
            while (!(intr->stat & INTR_MASK_PAD_MEMCARD)) {
                if (chkRC2wait())
                    return 0;
            }
            sio->data = 1;
            setRC2wait(60);
            if (!_padClrIntSio0())
                return 0;
            _padWaitRXready();
            (void)sio->data;
            return 0;
        }
        intr->stat = ~INTR_MASK_PAD_MEMCARD;
    }
    if (pad->forceInfoRefresh && pad->command)
        return 0;
    return 1;
}

static void advanceSioTransfer(PadState *pad)
{
    int result = padIntFunc[_padSioState++](pad);

    if (result >= 0) {
        if (_padSioState != 0 &&
            (_padSioState != 3 || pad->rxBuffer[0] != 0x80)) {
            setRC2wait(60);
            if (!_padClrIntSio0())
                _padFuncNextPort(-3);
        }
        if (_padSioState >= 5)
            _padSioState--;
    } else {
        _padFuncNextPort(result);
    }
}

int _padSioRW(PadState *pad, int tx)
{
    int rx;
    int id;
    unsigned short baud;
    unsigned short timerStart;

    if (tx < 0) {
        rx = sio->data;
        pad->rxLength = 255;
        pad->txLength = 1;
        pad->txBuffer[0] = ~tx;
        while (!(sio->status & SIO_STATUS_TX_READY))
            ;
        while (!chkRC2wait())
            ;
        sio->data = ~tx;
    } else {
        baud = SIO_STANDARD_BAUD;
        id = pad->rxBuffer[0];
        if ((id >> 4) == 8 && pad->rxLength >= 9)
            baud = SIO_MULTITAP_BAUD;

        timerStart = RC2_COUNT;
        _waitTime = 430;
        _startTime = timerStart;
        while (!(sio->status & SIO_STATUS_RX_READY))
            ;
        rx = sio->data;
        sio->baud = baud;
        while (!(intr->stat & INTR_MASK_PAD_MEMCARD)) {
            if (chkRC2wait())
                return -20;
        }
        sio->data = tx;
        if (baud == SIO_MULTITAP_BAUD) {
            intr->stat = ~INTR_MASK_PAD_MEMCARD;
            sio->control |= SIO_CONTROL_ACK;
        }
        pad->txLength++;
        pad->rxBuffer[pad->rxLength] = rx;
        pad->rxLength++;
    }
    return rx;
}

int _padSioRW2(PadState *pad, int tx)
{
    int rx;
    int id;
    unsigned short baud = SIO_STANDARD_BAUD;

    id = pad->rxBuffer[0];
    if ((id >> 4) == 8 && pad->rxLength >= 9)
        baud = SIO_MULTITAP_BAUD;
    do {
    } while (!(sio->status & SIO_STATUS_RX_READY));
    setRC2wait(400);
    rx = sio->data;
    if (pad->rxLength != 0 || (rx >> 4) != 8)
        sio->baud = baud;
    else
        sio->baud = SIO_MULTITAP_BAUD;

    while (!(intr->stat & INTR_MASK_PAD_MEMCARD)) {
        unsigned int now = RC2_COUNT;

        if (now < _startTime) {
            if (RC2_TARGET != 0)
                now += RC2_TARGET;
            else
                now += 0x10000;
        }
        if (RC2_MODE & RCNT_MODE_DIV8_CLOCK) {
            if (now - _startTime >= _waitTime)
                return -2;
        } else {
            if ((now - _startTime) >> 3 >= _waitTime)
                return -2;
        }
    }
    if (pad->controllerType != 8 && _padSioState == 2) {
        setRC2wait(60);
        while (!chkRC2wait())
            ;
    }
    sio->data = tx;
    if (_padSioState == 3 && rx == 0x80) {
        intr->stat = ~INTR_MASK_PAD_MEMCARD;
        sio->control |= SIO_CONTROL_ACK;
    }
    pad->txLength++;
    if (pad->rxLength != 255)
        pad->rxBuffer[pad->rxLength] = rx;
    pad->rxLength++;
    return rx;
}

int _padClrIntSio0(void)
{
    intr->stat = ~INTR_MASK_PAD_MEMCARD;
    while (sio->status & SIO_STATUS_DSR) {
        if (chkRC2wait())
            return 0;
    }
    sio->control |= SIO_CONTROL_ACK;
    return 1;
}

void _padWaitRXready(void)
{
    do {
    } while (!(sio->status & SIO_STATUS_RX_READY));
}
