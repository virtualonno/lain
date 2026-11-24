/* Sequence playback tick handler. */

#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void _SsSndPlay(short sep_no, short seq_no)
{
    _SsSeqPlay(sep_no, seq_no);
}
