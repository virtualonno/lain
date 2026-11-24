#include <libpad.h>
#include <memory.h>
#include "pad_int.h"

static SerialRegisters *sio = (SerialRegisters *)0x1f801040;
static unsigned char receiveBuffers[2][35];
static unsigned char transmitBuffers[2][35];
static PadState pads[2];

static void clearInfo(PadState *pad);
static void nextPort(int result);
static void clearCommand(PadState *pad);
static int getTxByte(PadState *pad, int tap);
static void limitCurrent(PadState *pad);
static int padToPort(PadState *pad);
static PadState *portToPad(int port);

void PadInitDirect(unsigned char *port1, unsigned char *port2)
{
    PadState *pad;
    int port;
    int i;
    unsigned char *p;

    _padIntExec = 0;
    _padModeMtap = 0;
    _padInitDirSeq();
    _padInfoDir = pads;
    _padFuncNextPort = nextPort;
    _padFuncClrInfo = clearInfo;
    _padFuncGetTxd = getTxByte;
    _padFuncCurrLimit = limitCurrent;
    _padFuncPtr2Port = padToPort;
    _padFuncPort2Info = portToPad;
    _padFuncClrCmdNo = clearCommand;
    bzero((unsigned char *)pads, sizeof(pads));
    pads[0].resultBuffer = port1;
    pads[1].resultBuffer = port2;
    for (port = 0, pad = pads; port < 2; port++, pad++) {
        pad->sockets = 0;
        pad->parent = pad;
        pad->resultBuffer[0] = 0xff;
        pad->resultBuffer[1] = 0;
        pad->rxBuffer = receiveBuffers[port];
        pad->txBuffer = transmitBuffers[port];
        p = pad->actuatorAlignment;
        for (i = 0; i < 6; i++)
            *p++ = 0xff;
    }
    _padSetVsyncParam();
    _padIntExec = 1;
}

static void clearInfo(PadState *pad)
{
    int i;
    unsigned char *p;

    if (pad->configState != 0) {
        pad->configState = 0;
        pad->configStep = 0;
        pad->modeExtendedId = 0;
        pad->prepareCommand = 0;
        pad->readCommandReply = 0;
        pad->modeCount = 0;
        pad->modeOffset = 0;
        pad->modeExtendedId = 0;
        pad->actuatorCount = 0;
        pad->combinationCount = 0;
        pad->modeIds = 0;
        pad->actuators = 0;
        pad->combinations = 0;
        p = pad->actuatorAlignment;
        for (i = 0; i < 6; i++)
            *p++ = 0xff;
    }
}

static void nextPort(int result)
{
    int ready;
    PadState *pad;

    do {
        pad = &pads[_padSioChan];
        if (result != -9) {
            if (result == 0)
                _padFixResult[_padSioChan] = 0;
            else {
                _dirFailAuto(pad, result);
                clearCommand(pad);
            }
        }
        _padSioState = 0;
        sio->control = 0;
        _padSioChan++;
        if (_padSioChan <= _padChanStop)
            ready = _padInitSioMode(&pads[_padSioChan]);
        else
            ready = 1;
        result = 0xffff;
    } while (!ready);
}

static void clearCommand(PadState *pad)
{
    unsigned char command = pad->command;
    pad->command = 0;
    pad->commandPending = command;
}

static int getTxByte(PadState *pad, int tap)
{
    int index = pad->txLength - 3;

    switch (pad->command) {
    case 0:
        if (index < 6 && !pad->actuatorEnabled[index])
            return 0;
        if (index < pad->actuatorValueCount)
            return pad->actuatorValues[index];
        return 0;
    case 0x4d:
        return index < pad->commandDataLength ? pad->commandData[index] : 0xff;
    default:
        return index < pad->commandDataLength ? pad->commandData[index] : 0;
    }
}

static void limitCurrent(PadState *pad)
{
    int actuator, slot, slots, mask, enabled;
    int total;
    unsigned char *align, *p;

    bzero(pad->actuatorEnabled, sizeof(pad->actuatorEnabled));
    if (pad->modeExtendedId != 0 && pad->actuatorValues != 0) {
        slots = pad->actuatorValueCount > 6 ? 6 : pad->actuatorValueCount;
        for (actuator = 0; actuator < pad->actuatorCount; actuator++) {
            enabled = 0;
            mask = pad->actuators[actuator].size != 0 ? 0xff : 1;
            align = pad->actuatorAlignment;
            p = pad->actuatorValues;
            for (slot = 0; slot < slots; align++, slot++, p++) {
                if (*align == actuator && (*p & mask)) {
                    enabled = 1;
                    break;
                }
            }
            if (enabled) {
                total = _padTotalCurr + pad->actuators[actuator].current;
                if (total <= 60)
                    _padTotalCurr = total;
                else
                    enabled = 0;
            }
            if (enabled) {
                align = pad->actuatorAlignment;
                p = pad->actuatorEnabled;
                for (slot = 0; slot < slots; slot++, p++) {
                    if (*align++ == actuator)
                        *p = 1;
                }
            }
        }
    } else if ((pad->controllerType == 4 || pad->controllerType == 5 ||
                pad->controllerType == 7) && pad->modeExtendedId == 0 &&
               pad->actuatorValueCount >= 2) {
        if ((pad->actuatorValues[0] & 0xc0) == 0x40 &&
            (pad->actuatorValues[1] & 1) && _padTotalCurr + 10 <= 60) {
            pad->actuatorEnabled[0] = pad->actuatorEnabled[1] = 1;
            _padTotalCurr += 10;
        }
    } else if (pad->controllerType == 3) {
        pad->actuatorEnabled[0] = 1;
    } else if (pad->modeExtendedId == 0) {
        for (slot = 0; slot < 6; slot++)
            pad->actuatorEnabled[slot] = 1;
    }
}

static int padToPort(PadState *pad)
{
    int port;
    for (port = 0; port < 2; port++) {
        if (pad == &pads[port])
            return (port + 1) << 4;
    }
    return 0xff;
}

static PadState *portToPad(int port)
{
    if ((port & 0xf0) == 0)
        return &pads[0];
    return &pads[1];
}
