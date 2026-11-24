#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

void SsSeqPlay(short seq_no, char mode, short play_count)
{
    Snd_SetPlayMode(seq_no, 0, mode, play_count);
}

void SsSepPlay(short sep_no, short seq_no, char mode, short play_count)
{
    Snd_SetPlayMode(sep_no, seq_no, mode, play_count);
}
