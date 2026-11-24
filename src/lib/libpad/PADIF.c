#include "pad_int.h"

static int sendAddressByte(PadState *pad);
static int sendCommandByte(PadState *pad);
static int readIdByte(PadState *pad);
static int checkMarkerByte(PadState *pad);
static int transferDataBytes(PadState *pad);

static int bytesLeft = 0;
static SerialRegisters *sio = (SerialRegisters *)0x1f801040;
static int sendSocketsAuto = 0;
int (*padIntFunc[])(PadState *pad) = {
    sendAddressByte, sendCommandByte, readIdByte, checkMarkerByte, transferDataBytes
};

static int sendAddressByte(PadState *pad)
{
    sendSocketsAuto = _padFuncSendAuto(pad);
    pad->rxBuffer[0] = 0;
    return _padSioRW(pad, -2);
}

static int sendCommandByte(PadState *pad)
{
    if (_padSioChan == _padChanStart && _padGunExec) {
        _padFuncGetGunPos(pad);
        _padFuncSetGunPort();
    }
    if (sendSocketsAuto) {
        _padFuncSendAuto(&pad->sockets[0]);
        _padFuncSendAuto(&pad->sockets[1]);
    }
    if (pad->command != 0)
        return _padSioRW2(pad, pad->command);
    return _padSioRW2(pad, 0x42);
}

static int readIdByte(PadState *pad)
{
    int tx;
    int rx;

    if (sendSocketsAuto) {
        _padFuncSendAuto(&pad->sockets[2]);
        _padFuncSendAuto(&pad->sockets[3]);
    }
    if (pad->command == 0)
        tx = _padModeMtap;
    else
        tx = 0;
    rx = _padSioRW2(pad, tx);
    if (rx < 0)
        return rx;
    if ((bytesLeft = (rx & 15) * 2) == 0)
        bytesLeft = 32;
    return 0;
}

static int checkMarkerByte(PadState *pad)
{
    int multitap = 0;
    int rx;

    if ((pad->rxBuffer[0] >> 4) == 8)
        multitap = pad->command == 0;
    rx = _padSioRW2(pad, (unsigned char)_padFuncGetTxd(pad, multitap));
    if (rx == 0x5a || rx == 0 || rx < 0)
        return rx;
    return -4;
}

static int transferDataBytes(PadState *pad)
{
    int multitap = 0;
    int slot;
    int rx;
    int otherPort;
    PadState *resultPad;
    int id;

    _padFuncCurrLimit(pad);
    if (_padModeMtap) {
        id = pad->rxBuffer[0];
        if ((id >> 4) == 8)
            multitap = pad->command == 0;
    }
    if (multitap) {
        for (slot = -1; slot < 4; ++slot) {
            if (--bytesLeft <= 0)
                break;
            if (slot >= 0)
                _padFuncCurrLimit(&pad->sockets[slot]);
            rx = _padSioRW2(pad, (unsigned char)_padFuncGetTxd(pad, 1));
            if (rx < 0)
                return rx;
            setRC2wait(60);
            if (!_padClrIntSio0())
                return -3;
        }
    }

    resultPad = 0;
    otherPort = _padSioChan == 0;
    while (bytesLeft >= 2) {
        if (_padFixResult[otherPort] < 0)
            break;
        if (_padFixResult[otherPort] > 0) {
            resultPad = _padInfoDir[otherPort].sockets + _padFixResult[otherPort] - 1;
            _padFuncRecvAuto(resultPad);
        }
        switch (_padFixResult[otherPort]) {
        case 4:
            _padFixResult[otherPort] = 3;
            break;
        case 3:
            _padFuncRecvAuto(resultPad - 1);
            _padFixResult[otherPort] = 1;
            break;
        case 1:
        case 0:
            resultPad = &_padInfoDir[otherPort];
            _padFuncRecvAuto(resultPad);
            _padFuncClrCmdNo(resultPad);
            _padFixResult[otherPort] = -1;
            break;
        }
        rx = _padSioRW(pad, (unsigned char)_padFuncGetTxd(pad, multitap));
        if (rx < 0)
            return rx;
        setRC2wait(60);
        if (!_padClrIntSio0())
            return -3;
        bytesLeft--;
    }

    while (--bytesLeft > 0) {
        rx = _padSioRW(pad, (unsigned char)_padFuncGetTxd(pad, multitap));
        if (rx < 0)
            return rx;
        if (sio->baud != 0x22) {
            setRC2wait(60);
            if (!_padClrIntSio0())
                return -3;
        }
    }
    _padWaitRXready();
    pad->rxBuffer[pad->rxLength++] = sio->data;
    _padFuncNextPort(0);
    return 0;
}
