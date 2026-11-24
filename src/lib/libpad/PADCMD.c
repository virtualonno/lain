/* LIBPAD's configuration commands: they read a controller's modes, actuators and
   combinations into its record and set its actuator alignment and main mode. */
#include <libpad.h>
#include "pad_int.h"

static void requestModeInfo(PadState *pad);
static void requestModeId(PadState *pad, int index);
static void requestActuatorInfo(PadState *pad, int index);
static void requestCombinationInfo(PadState *pad, int index);
static void requestCombinationData(PadState *pad);
static void sendActInfo(PadState *pad);
static int recvActInfo(PadState *pad);
static void sendActAlign(PadState *pad);
static int recvActAlign(PadState *pad);
static void sendMainMode(PadState *pad);
static int recvMainMode(PadState *pad);

void _padSetAct(PadState *pad, unsigned char *values, int count)
{
    pad->actuatorValues = values;
    pad->actuatorValueCount = count;
}

void _padSetCmd(PadState *pad, unsigned char cmd, unsigned char *data, int length)
{
    pad->command = cmd;
    pad->commandData = data;
    pad->commandDataLength = length;
}

void _padSendAtLoadInfo(PadState *pad)
{
    switch (pad->configStep) {
    case 2:
        requestModeInfo(pad);
        break;
    case 3:
        requestModeId(pad, pad->modeOffset);
        break;
    case 4:
        requestCombinationInfo(pad, pad->commandIndex);
        break;
    }
}

int _padRecvAtLoadInfo(PadState *pad)
{
    unsigned short id;

    switch (pad->configStep) {
    case 2:
        if (pad->rxBuffer[7] != 0)
            return 0;
        if (pad->modeCount == pad->rxBuffer[3] && pad->modeOffset == pad->rxBuffer[4] &&
            pad->actuatorCount == pad->rxBuffer[5] && pad->combinationCount == pad->rxBuffer[6])
            pad->confirmValue = 0;
        else
            pad->confirmValue = 0xffff;
        pad->modeCount = pad->rxBuffer[3];
        pad->modeOffset = pad->rxBuffer[4];
        pad->modeExtendedId = 0;
        pad->actuatorCount = pad->rxBuffer[5];
        pad->combinationCount = pad->rxBuffer[6];
        pad->combinationDataSize = 0;
        if (pad->confirmValue != 0)
            return 0;
        pad->configReplyCount = 0;
        break;
    case 3:
        if (pad->rxBuffer[2] != 0 || pad->rxBuffer[3] != 0)
            return 0;
        id = (pad->rxBuffer[4] << 8) + pad->rxBuffer[5];
        pad->modeExtendedId = id;
        if (pad->confirmValue != id) {
            pad->confirmValue = id;
            return 0;
        }
        pad->confirmValue = 0xffff;
        pad->configReplyCount = 0;
        pad->commandIndex = 0;
        break;
    case 4:
        if (pad->rxBuffer[2] != 0 || pad->rxBuffer[3] != 0)
            return 0;
        pad->combinationDataSize += sizeof(PadActCombination) + ((pad->rxBuffer[4] + 3) & ~3);
        if (++pad->commandIndex < pad->combinationCount)
            return 0;
        if (_padGetActSize(pad) > (int)sizeof(pad->infoBuffer)) {
            _padFuncClrInfo(pad);
            pad->configStep = 254;
            pad->configState = PadStateFindCTP1;
        } else if (pad->confirmValue != pad->combinationDataSize) {
            pad->confirmValue = pad->combinationDataSize;
            pad->commandIndex = 0;
            pad->combinationDataSize = 0;
            return 0;
        } else {
            pad->confirmValue = 0;
            pad->configReplyCount = 0;
            pad->configStep = 255;
            _padLoadActInfo(pad, pad->infoBuffer);
            pad->configStep = 2;
        }
        return 0;
    }
    return 1;
}

int _padGetActSize(PadState *pad)
{
    int modeSize = ((pad->modeCount + 1) / 2) * 4;
    int actuatorSize = ((pad->actuatorCount * (int)sizeof(PadActuator) + 3) & 0xffc) + 4;

    return modeSize + actuatorSize + pad->combinationDataSize;
}

int _padLoadActInfo(PadState *pad, unsigned char *buffer)
{
    long modes;
    long actuators;

    if (buffer == 0 || pad->actuators != 0 || _padFuncChkEng(pad))
        return 0;

    pad->configState = PadStateReqInfo;
    modes = (((long)buffer + 3) >> 2) << 2;
    pad->modeIds = (unsigned short *)modes;
    pad->commandIndex = 0;
    pad->configStep = 1;
    pad->prepareCommand = sendActInfo;
    pad->readCommandReply = recvActInfo;
    actuators = modes + ((pad->modeCount + 1) / 2) * 4;
    pad->actuators = (PadActuator *)actuators;
    pad->combinations = (PadActCombination *)(actuators + ((pad->actuatorCount * (int)sizeof(PadActuator) + 3) & 0xffc));
    return 1;
}

static void sendActInfo(PadState *pad)
{
    switch (pad->configStep) {
    case 2:
        requestModeId(pad, pad->commandIndex);
        break;
    case 3:
        requestActuatorInfo(pad, pad->commandIndex);
        break;
    case 4:
        if (pad->combinationBytesLeft == 0)
            requestCombinationInfo(pad, pad->commandIndex);
        else
            requestCombinationData(pad);
        break;
    }
}

static int recvActInfo(PadState *pad)
{
    unsigned char *reply;
    PadActuator *act;
    PadActCombination *combo;
    int count;
    unsigned char *data;
    static unsigned char *dst;
    int byte;

    switch (pad->configStep) {
    case 2:
        if (pad->rxBuffer[2] != 0 || pad->rxBuffer[3] != 0)
            return 0;
        pad->modeIds[pad->commandIndex] = (pad->rxBuffer[4] << 8) + pad->rxBuffer[5];
        if (pad->confirmValue != pad->modeIds[pad->commandIndex]) {
            pad->confirmValue = pad->modeIds[pad->commandIndex];
            return 0;
        }
        pad->confirmValue = 0;
        pad->configReplyCount = 0;
        if (++pad->commandIndex < pad->modeCount)
            return 0;
        pad->commandIndex = 0;
        break;
    case 3:
        reply = pad->rxBuffer;
        if (reply[2] != 0 || reply[3] != 0)
            return 0;
        act = &pad->actuators[pad->commandIndex];
        if (act->function == reply[4] && act->subfunction == (reply[5] & 0x7f) &&
            act->size == reply[6] && act->current == reply[7] && act->signal == ((byte = reply[5]) >> 7))
            pad->confirmValue = 0;
        else
            pad->confirmValue = 0xffff;
        act->function = pad->rxBuffer[4];
        act->subfunction = pad->rxBuffer[5] & 0x7f;
        act->size = pad->rxBuffer[6];
        act->current = pad->rxBuffer[7];
        byte = pad->rxBuffer[5];
        act->signal = byte >> 7;
        if (pad->confirmValue != 0)
            return 0;
        pad->configReplyCount = 0;
        if (++pad->commandIndex < pad->actuatorCount)
            return 0;
        pad->commandIndex = 0;
        pad->combinationBytesLeft = 0;
        break;
    case 4:
        reply = pad->rxBuffer;
        if (reply[2] != 0) {
            pad->combinationBytesLeft = 0;
            return 0;
        }
        combo = &pad->combinations[pad->commandIndex];
        if (pad->combinationBytesLeft == 0) {
            combo->count = pad->combinationBytesLeft = reply[4];
            count = 3;
            data = pad->rxBuffer + 5;
            if (pad->commandIndex == 0)
                dst = combo->indices = (unsigned char *)&pad->combinations[pad->combinationCount];
            else
                dst = combo->indices = combo[-1].indices + ((combo[-1].count + 3) & ~3);
        } else {
            count = 5;
            data = reply + 3;
        }
        while (count--) {
            if (pad->combinationBytesLeft == 0)
                break;
            if (dst >= (unsigned char *)&pad->modeCount) {
                pad->commandIndex = 0;
                pad->combinationBytesLeft = 0;
                return 0;
            }
            if (*dst != *data)
                pad->confirmValue = 0xffff;
            *dst++ = *data++;
            --pad->combinationBytesLeft;
        }
        if (pad->combinationBytesLeft != 0)
            return 0;
        if (pad->confirmValue != 0) {
            pad->confirmValue = 0;
            pad->combinationBytesLeft = 0;
            return 0;
        }
        if (++pad->commandIndex >= pad->combinationCount) {
            pad->configState = PadStateStable;
            pad->configStep = 254;
            pad->configReplyCount = 0;
            return 0;
        } else {
            pad->combinationBytesLeft = 0;
            pad->configReplyCount = 0;
            return 0;
        }
    }
    return 1;
}

int _padSetActAlign(PadState *pad, unsigned char *alignment)
{
    if (_padFuncChkEng(pad))
        return 0;
    pad->configStep = 1;
    pad->prepareCommand = sendActAlign;
    pad->requestedAlignment = alignment;
    pad->readCommandReply = recvActAlign;
    return 1;
}

static void sendActAlign(PadState *pad)
{
    unsigned char *alignment = pad->requestedAlignment;
    pad->command = 0x4d;
    pad->commandDataLength = 6;
    pad->commandData = alignment;
}

static int recvActAlign(PadState *pad)
{
    int act;
    int i;
    int count;
    int size;
    unsigned char *alignment;

    for (act = 0; act < pad->actuatorCount; act++) {
        alignment = pad->requestedAlignment;
        count = 0;
        for (i = 5; i >= 0; i--) {
            if (*alignment++ == act)
                count++;
        }
        size = pad->actuators[act].size;
        alignment = pad->requestedAlignment;
        if (size == 0)
            size = 1;
        for (i = 0; i < 6; i++) {
            if (*alignment++ == act) {
                if (count < size) {
                    pad->actuatorAlignment[i] = 255;
                    count--;
                } else {
                    pad->actuatorAlignment[i] = act;
                }
            }
        }
    }
    pad->configStep = 254;
    return 0;
}

int _padSetMainMode(PadState *pad, unsigned char offset, unsigned char lock)
{
    if (_padFuncChkEng(pad))
        return 0;
    pad->configStep = 1;
    pad->prepareCommand = sendMainMode;
    pad->readCommandReply = recvMainMode;
    pad->requestedMode[0] = offset;
    pad->requestedMode[1] = lock;
    pad->requestedMode[2] = offset == pad->modeOffset;
    return 1;
}

static void sendMainMode(PadState *pad)
{
    switch (pad->configStep) {
    case 2:
        pad->command = 0x44;
        pad->commandData = pad->requestedMode;
        pad->commandDataLength = 2;
        break;
    case 3:
        pad->command = 0x4d;
        pad->commandData = pad->actuatorAlignment;
        pad->commandDataLength = 6;
        break;
    }
}

static int recvMainMode(PadState *pad)
{
    if (pad->requestedMode[2]) {
        if (pad->configStep == 2)
            return 1;
        pad->configStep = 254;
    } else {
        _padFuncClrInfo(pad);
    }
    return 0;
}

void _padCmdParaMode(PadState *pad, int enter)
{
    pad->command = 0x43;
    pad->commandData = &pad->commandParam;
    pad->commandParam = enter;
    pad->commandDataLength = 1;
}

static void requestModeInfo(PadState *pad)
{
    pad->command = 0x45;
    pad->commandData = 0;
    pad->commandDataLength = 0;
}

static void requestModeId(PadState *pad, int index)
{
    pad->command = 0x4c;
    pad->commandData = &pad->commandParam;
    pad->commandParam = index;
    pad->commandDataLength = 1;
}

static void requestActuatorInfo(PadState *pad, int index)
{
    pad->command = 0x46;
    pad->commandData = &pad->commandParam;
    pad->commandParam = index;
    pad->commandDataLength = 1;
}

static void requestCombinationInfo(PadState *pad, int index)
{
    pad->command = 0x47;
    pad->commandData = &pad->commandParam;
    pad->commandParam = index;
    pad->commandDataLength = 1;
}

static void requestCombinationData(PadState *pad)
{
    pad->command = 0x4b;
    pad->commandData = 0;
    pad->commandDataLength = 0;
}
