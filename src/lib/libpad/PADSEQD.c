#include "pad_int.h"

static int sendAuto(PadState *pad);
static void recvAuto(PadState *pad);
static int checkEngaged(PadState *pad);

void _padInitDirSeq(void)
{
    _padFuncSendAuto = sendAuto;
    _padFuncChkEng = checkEngaged;
    _padFuncRecvAuto = recvAuto;
}

static int sendAuto(PadState *pad)
{
    if (pad->rxBuffer[0] == 0xf3) {
        if (pad->controllerType == 0) {
            _padCmdParaMode(pad, 0);
            return 0;
        }
        if (pad->configStep == 255) {
            _padCmdParaMode(pad, 0);
            return 0;
        }
        if (pad->configState == 2)
            _padFuncClrInfo(pad);
    }

    switch (pad->configStep) {
    case 0:
        break;
    case 1:
        _padCmdParaMode(pad, 1);
        break;
    case 254:
        _padCmdParaMode(pad, 0);
        break;
    case 255:
        break;
    default:
        if (pad->prepareCommand != 0)
            pad->prepareCommand(pad);
        else
            _padSendAtLoadInfo(pad);
        break;
    }
    return 0;
}

static void recvAuto(PadState *pad)
{
    unsigned char previousType;
    unsigned char step;
    int i;
    unsigned char id = pad->rxBuffer[0];

    if ((id & 0xf0) == 0) {
        pad->resultBuffer[0] = 255;
        pad->resultBuffer[1] = 0;
        pad->controllerType = 0;
        pad->resultLength = 0;
        _padFuncClrInfo(pad);
        return;
    }

    previousType = pad->controllerType;
    pad->controllerType = id >> 4;
    if (pad->controllerType == 15) {
        pad->controllerType = previousType;
    } else {
        pad->resultBuffer[0] = 0;
        pad->resultBuffer[1] = pad->rxBuffer[0];
        pad->resultLength = pad->rxLength;
        for (i = 2; i < pad->rxLength; i++)
            pad->resultBuffer[i] = pad->rxBuffer[i];
    }

    if ((pad->rxBuffer[1] == 0 && (pad->configStep != 1 || pad->prepareCommand != 0) &&
         pad->forceInfoRefresh == 0) || pad->controllerType != previousType)
        _padFuncClrInfo(pad);

    step = pad->configStep;
    pad->retries = 0;
    if (step == 255 || (step != 0 && pad->command == 0))
        return;

    if ((unsigned char)(step - 2) < 252 && pad->rxBuffer[0] != 0xf3) {
        _padFuncClrInfo(pad);
        return;
    }

    switch (pad->configStep) {
    case 0:
        pad->configState = 1;
        pad->configStep++;
        return;
    case 1:
        pad->commandIndex = 0;
        pad->configStep++;
        return;
    case 254:
        pad->configStep = 255;
        return;
    default:
        if (pad->readCommandReply != 0)
            pad->configStep += pad->readCommandReply(pad);
        else
            pad->configStep += _padRecvAtLoadInfo(pad);
        return;
    }
}

void _dirFailAuto(PadState *pad, int result)
{
    pad->timeoutCount++;
    if (pad->configStep != 0) {
        if (pad->configStep == 1) {
            if (pad->retries < 11) {
                ++pad->retries;
            } else {
                pad->configState = 2;
                pad->configStep = 255;
            }
            return;
        }
        if (pad->retries < 11) {
            ++pad->retries;
            return;
        }
        if (pad->configState != 0)
            _padFuncClrInfo(pad);
    }

    if (pad->rxBuffer[0] != 0xf3) {
        pad->resultBuffer[0] = 255;
        pad->resultBuffer[1] = 0;
        pad->controllerType = 0;
        pad->resultLength = 0;
    }
}

static int checkEngaged(PadState *pad)
{
    if (pad->modeExtendedId == 0 || pad->configStep != 255)
        return 1;
    return 0;
}
