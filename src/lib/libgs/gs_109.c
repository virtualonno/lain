#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include "../gte/gte_int.h"

void GsSetFogParam(GsFOGPARAM *fog)
{
    SetDQA(fog->dqa);
    SetDQB(fog->dqb);
    SetFarColor(fog->rfc, fog->gfc, fog->bfc);
}
