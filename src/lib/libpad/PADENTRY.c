#include <libpad.h>
#include "pad_int.h"

int PadChkVsync(void)
{
    return _padChkVsync();
}

void PadStartCom(void)
{
    _padStartCom();
}

void PadStopCom(void)
{
    _padStopCom();
}

int PadChkMtap(int port)
{
    if (_padModeMtap)
        return _padInfoDir[port >> 4].controllerType == 8;
    return 0;
}

int PadGetState(int port)
{
    PadState *pad = _padFuncPort2Info(port);

    if (pad->command || pad->commandPending ||
        (pad != pad->parent && pad->parentBusy) || pad->resultBuffer[0]) {
        switch (pad->configState) {
        case PadStateFindCTP1:
            return PadStateFindPad;
        case PadStateFindCTP2:
            return PadStateFindPad;
        case PadStateStable:
            return PadStateReqInfo;
        }
    }
    return pad->configState;
}

int PadInfoMode(int port, int request, int index)
{
    PadState *pad = _padFuncPort2Info(port);

    switch (request) {
    case InfoModeCurID:
        return pad->controllerType;
    case InfoModeCurExID:
        return pad->modeExtendedId;
    case InfoModeCurExOffs:
        return pad->modeOffset;
    case InfoModeIdTable:
        if (index < 0)
            return pad->modeCount;
        if (index < pad->modeCount)
            return pad->modeIds[index];
        break;
    case 100:
        return pad->timeoutCount;
    }
    return 0;
}

int PadInfoAct(int port, int actuator, int request)
{
    PadState *pad = _padFuncPort2Info(port);

    if (actuator < 0)
        return pad->actuatorCount;
    if (actuator < pad->actuatorCount) {
        const PadActuator *act = &pad->actuators[actuator];

        switch (request) {
        case InfoActFunc:
            return act->function;
        case InfoActSub:
            return act->subfunction;
        case InfoActSize:
            return act->size;
        case InfoActCurr:
            return act->current;
        case InfoActSign:
            return act->signal;
        }
    }
    return 0;
}

int PadInfoComb(int port, int combination, int index)
{
    PadState *pad = _padFuncPort2Info(port);

    if (combination < 0)
        return pad->combinationCount;
    if (combination < pad->combinationCount) {
        const PadActCombination *combo = &pad->combinations[combination];

        if (index < 0)
            return combo->count;
        if (index < combo->count)
            return combo->indices[index];
        return 0;
    }
    return 0;
}

int PadSetActAlign(int port, unsigned char *alignment)
{
    return _padSetActAlign(_padFuncPort2Info(port), alignment);
}

int PadSetMainMode(int port, int offset, int lock)
{
    return _padSetMainMode(_padFuncPort2Info(port), offset, lock);
}

void PadSetAct(int port, unsigned char *values, int count)
{
    _padSetAct(_padFuncPort2Info(port), values, count);
}
