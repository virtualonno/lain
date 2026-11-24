#include <sys/types.h>
#include <stdio.h>
#include "snd_int.h"

short SsSeqOpen(unsigned long *data, short vab_id)
{
    short seq_no = 0;
    unsigned char *seq_data = (unsigned char *)data;
    short status;
    int i;
    short found;
    unsigned long open = _snd_openflag;

    if (open == ~0) {
        printf("Can't Open Sequence data any more\n\n");
        return -1;
    }

    i = 0;
    found = 0;
    do {
        if ((open & (1 << i)) == 0) {
            seq_no = i;
            found = 1;
        }
        ++i;
        if (i >= SND_SEQ_SEP_MAX) {
            printf("Can't Open Sequence data any more\n\n");
            return -1;
        }
    } while (!found);

    _snd_openflag |= 1 << seq_no;
    status = _SsInitSoundSeq(seq_no, vab_id, seq_data);

    SsFCALL.noteon = (void (*)())_SsNoteOn;
    SsFCALL.programchange = (void (*)())_SsSetProgramChange;
    SsFCALL.metaevent = (void (*)())_SsGetMetaEvent;
    SsFCALL.pitchbend = (void (*)())_SsSetPitchBend;
    SsFCALL.control[CC_NUMBER] = (void (*)())_SsSetControlChange;
    SsFCALL.control[CC_BANKCHANGE] = (void (*)())_SsContBankChange;
    SsFCALL.control[CC_MAINVOL] = (void (*)())_SsContMainVol;
    SsFCALL.control[CC_PANPOT] = (void (*)())_SsContPanpot;
    SsFCALL.control[CC_EXPRESSION] = (void (*)())_SsContExpression;
    SsFCALL.control[CC_DAMPER] = (void (*)())_SsContDamper;
    SsFCALL.control[CC_NRPN1] = (void (*)())_SsContNrpn1;
    SsFCALL.control[CC_NRPN2] = (void (*)())_SsContNrpn2;
    SsFCALL.control[CC_RPN1] = (void (*)())_SsContRpn1;
    SsFCALL.control[CC_RPN2] = (void (*)())_SsContRpn2;
    SsFCALL.control[CC_EXTERNAL] = (void (*)())_SsContExternal;
    SsFCALL.control[CC_RESETALL] = (void (*)())_SsContResetAll;
    SsFCALL.control[CC_DATAENTRY] = (void (*)())_SsContDataEntry;

    SsFCALL.ccentry[DE_PRIORITY] = (void (*)())_SsSetNrpnVabAttr0;
    SsFCALL.ccentry[DE_MODE] = (void (*)())_SsSetNrpnVabAttr1;
    SsFCALL.ccentry[DE_LIMITL] = (void (*)())_SsSetNrpnVabAttr2;
    SsFCALL.ccentry[DE_LIMITH] = (void (*)())_SsSetNrpnVabAttr3;
    SsFCALL.ccentry[DE_ADSR_AR_L] = (void (*)())_SsSetNrpnVabAttr4;
    SsFCALL.ccentry[DE_ADSR_AR_E] = (void (*)())_SsSetNrpnVabAttr5;
    SsFCALL.ccentry[DE_ADSR_DR] = (void (*)())_SsSetNrpnVabAttr6;
    SsFCALL.ccentry[DE_ADSR_SL] = (void (*)())_SsSetNrpnVabAttr7;
    SsFCALL.ccentry[DE_ADSR_SR_L] = (void (*)())_SsSetNrpnVabAttr8;
    SsFCALL.ccentry[DE_ADSR_SR_E] = (void (*)())_SsSetNrpnVabAttr9;
    SsFCALL.ccentry[DE_ADSR_RR_L] = (void (*)())_SsSetNrpnVabAttr10;
    SsFCALL.ccentry[DE_ADSR_RR_E] = (void (*)())_SsSetNrpnVabAttr11;
    SsFCALL.ccentry[DE_ADSR_SR] = (void (*)())_SsSetNrpnVabAttr12;
    SsFCALL.ccentry[DE_VIB_TIME] = (void (*)())_SsSetNrpnVabAttr13;
    SsFCALL.ccentry[DE_PORTA_DEPTH] = (void (*)())_SsSetNrpnVabAttr14;
    SsFCALL.ccentry[DE_REV_TYPE] = (void (*)())_SsSetNrpnVabAttr15;
    SsFCALL.ccentry[DE_REV_DEPTH] = (void (*)())_SsSetNrpnVabAttr16;
    SsFCALL.ccentry[DE_ECHO_FB] = (void (*)())_SsSetNrpnVabAttr17;
    SsFCALL.ccentry[DE_ECHO_DELAY] = (void (*)())_SsSetNrpnVabAttr18;
    SsFCALL.ccentry[DE_DELAY] = (void (*)())_SsSetNrpnVabAttr19;

    if (status == -1)
        return -1;
    return seq_no;
}
