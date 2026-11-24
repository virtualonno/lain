#include <sys/types.h>
#include <libsnd.h>

#include "snd_int.h"

extern int _SsVmKeyOff(short seq_sep_no, short vab_id, short program, unsigned short note);
extern int _SsVmKeyOn(short seq_sep_no, short vab_id, short program, unsigned short note,
                      unsigned short velocity, unsigned short pan);

void _SsNoteOn(short sep_no, short seq_no, unsigned char note, unsigned char velocity)
{
    SndSeqScore *score = &_ss_score[sep_no][seq_no];
    unsigned char channel = score->channel;
    unsigned short pan = score->panpot[channel];

    unsigned short key_velocity = velocity;

    if (velocity) {
        if (!((score->channel_mute >> channel) & 1)) {
            _SsVmKeyOn(sep_no | (seq_no << 8), (signed char)score->vab_id,
                       score->programs[channel], note, key_velocity, pan);
        }
    } else {
        _SsVmKeyOff(sep_no | (seq_no << 8), (signed char)score->vab_id,
                    score->programs[channel], note);
    }
}
