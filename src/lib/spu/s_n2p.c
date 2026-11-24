#include "spu_int.h"

#define SPU_PITCH_MAX 0x3fff
#define SEMITONES_PER_OCTAVE 12
#define NOTE_FRACTION_BITS 7
#define NOTE_FRACTION_STEPS (1 << NOTE_FRACTION_BITS)

/* Q15 multipliers for semitone intervals and fractions of a semitone. */
static unsigned short semitone_pitch[SEMITONES_PER_OCTAVE] = {
    0x8000, 0x879c, 0x8fac, 0x9837, 0xa145, 0xaadc, 0xb504, 0xbfc8,
    0xcb2f, 0xd744, 0xe411, 0xf1a1,
};
static unsigned short fraction_pitch[NOTE_FRACTION_STEPS] = {
    0x8000, 0x800e, 0x801d, 0x802c, 0x803b, 0x804a, 0x8058, 0x8067,
    0x8076, 0x8085, 0x8094, 0x80a3, 0x80b1, 0x80c0, 0x80cf, 0x80de,
    0x80ed, 0x80fc, 0x810b, 0x811a, 0x8129, 0x8138, 0x8146, 0x8155,
    0x8164, 0x8173, 0x8182, 0x8191, 0x81a0, 0x81af, 0x81be, 0x81cd,
    0x81dc, 0x81eb, 0x81fa, 0x8209, 0x8218, 0x8227, 0x8236, 0x8245,
    0x8254, 0x8263, 0x8272, 0x8282, 0x8291, 0x82a0, 0x82af, 0x82be,
    0x82cd, 0x82dc, 0x82eb, 0x82fa, 0x830a, 0x8319, 0x8328, 0x8337,
    0x8346, 0x8355, 0x8364, 0x8374, 0x8383, 0x8392, 0x83a1, 0x83b0,
    0x83c0, 0x83cf, 0x83de, 0x83ed, 0x83fd, 0x840c, 0x841b, 0x842a,
    0x843a, 0x8449, 0x8458, 0x8468, 0x8477, 0x8486, 0x8495, 0x84a5,
    0x84b4, 0x84c3, 0x84d3, 0x84e2, 0x84f1, 0x8501, 0x8510, 0x8520,
    0x852f, 0x853e, 0x854e, 0x855d, 0x856d, 0x857c, 0x858b, 0x859b,
    0x85aa, 0x85ba, 0x85c9, 0x85d9, 0x85e8, 0x85f8, 0x8607, 0x8617,
    0x8626, 0x8636, 0x8645, 0x8655, 0x8664, 0x8674, 0x8683, 0x8693,
    0x86a2, 0x86b2, 0x86c1, 0x86d1, 0x86e0, 0x86f0, 0x8700, 0x870f,
    0x871f, 0x872e, 0x873e, 0x874e, 0x875d, 0x876d, 0x877d, 0x878c,
};

int _spu_note2pitch(short center_note, short center_fraction,
                    short note, short fraction)
{
    short octave;
    short semitone;
    unsigned long pitch;
    int shift;
    unsigned int fine;

    fraction += center_fraction;
    fine = (unsigned short)fraction;
    note = note + (fine >> NOTE_FRACTION_BITS) - center_note;
    fraction = fine & (NOTE_FRACTION_STEPS - 1);
    octave = note / SEMITONES_PER_OCTAVE - 2;
    semitone = note % SEMITONES_PER_OCTAVE;

    if (semitone < 0) {
        semitone += SEMITONES_PER_OCTAVE;
        octave--;
    }
    pitch = (unsigned long)(semitone_pitch[semitone] * fraction_pitch[(unsigned short)fraction] >> 16);
    if (octave >= 0) {
        pitch = SPU_PITCH_MAX;
    } else {
        shift = -octave;
        pitch += 1 << (shift - 1);
        pitch >>= shift;
    }
    return (unsigned short)pitch;
}

unsigned long _spu_pitch2note(int center_note, int center_fraction,
                            unsigned short pitch)
{
    int bit;
    int octave_bit = 0;
    int i;
    unsigned short semitone;
    unsigned int note_value;
    unsigned int fine;
    unsigned int rounded_fraction;

    if (pitch > SPU_PITCH_MAX) {
        pitch = SPU_PITCH_MAX;
    }
    for (bit = 0; bit < 14; bit++) {
        if ((pitch >> bit) & 1) {
            octave_bit = bit;
        }
    }
    /* For nonzero pitch, normalization puts the high bit at bit 15,
       so both Q15 searches find an entry. */
    pitch <<= 15 - octave_bit;
    for (i = SEMITONES_PER_OCTAVE - 1; i >= 0; i--) {
        if (pitch >= semitone_pitch[i]) {
            semitone = i;
            break;
        }
    }
    pitch = ((unsigned long)pitch << 15) / semitone_pitch[semitone];
    for (i = NOTE_FRACTION_STEPS - 1; i >= 0; i--) {
        if (pitch >= fraction_pitch[i]) {
            note_value = i;
            break;
        }
    }
    rounded_fraction = note_value + 1;
    note_value = center_fraction + rounded_fraction;
    semitone = semitone + ((unsigned int)center_note + (octave_bit - 12) * SEMITONES_PER_OCTAVE);
    fine = (unsigned short)note_value;
    semitone += fine >> NOTE_FRACTION_BITS;
    note_value = ((unsigned long)semitone << 8) | (fine & 0x7e);
    return note_value;
}
