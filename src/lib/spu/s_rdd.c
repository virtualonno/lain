#include "spu_int.h"

long SpuReadDecodedData(SpuDecodedData *data, long flag)
{
    short *samples;
    unsigned short tsa;
    unsigned long blocks;

    switch (flag) {
    case SPU_CDONLY:
        samples = data->cd_left;
        tsa = 0;
        blocks = 32;
        break;
    case SPU_VOICEONLY:
        samples = data->voice1;
        tsa = 256;
        blocks = 32;
        break;
    default:
        samples = data->cd_left;
        tsa = 0;
        blocks = 64;
        break;
    }
    _spu_Fr_((unsigned long)samples, tsa, blocks);
    if (_spu_RXX[SPU_REG_SPUSTAT] & SPUSTAT_CAPTURE_HALF) {
        return SPU_DECODED_SECONDHALF;
    }
    return SPU_DECODED_FIRSTHALF;
}
