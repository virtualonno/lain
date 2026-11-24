#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsSetControlChange(short sep_no, short seq_no, unsigned char control)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char value = *score->read_pos++;

    switch (control) {
    case 0:
        score->vab_id = value;
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        break;
    case 6:
        SsFCALL.control[CC_DATAENTRY](sep_no, seq_no, value);
        break;
    case 7:
        SsFCALL.control[CC_MAINVOL](sep_no, seq_no, value);
        break;
    case 10:
        SsFCALL.control[CC_PANPOT](sep_no, seq_no, value);
        break;
    case 11:
        SsFCALL.control[CC_EXPRESSION](sep_no, seq_no, value);
        break;
    case 64:
        SsFCALL.control[CC_DAMPER](sep_no, seq_no, value);
        break;
    case 91:
        SsFCALL.control[CC_EXTERNAL](sep_no, seq_no, value);
        break;
    case 98:
        SsFCALL.control[CC_NRPN1](sep_no, seq_no, value);
        break;
    case 99:
        SsFCALL.control[CC_NRPN2](sep_no, seq_no, value);
        break;
    case 100:
        SsFCALL.control[CC_RPN1](sep_no, seq_no, value);
        break;
    case 101:
        SsFCALL.control[CC_RPN2](sep_no, seq_no, value);
        break;
    case 121:
        SsFCALL.control[CC_RESETALL](sep_no, seq_no);
        break;
    default:
        score->delta = _SsReadDeltaValue(sep_no, seq_no);
        break;
    }
}
