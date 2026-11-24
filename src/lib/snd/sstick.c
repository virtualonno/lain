#include <sys/types.h>
#include <libetc.h>
#include <libsnd.h>

#include "snd_int.h"

void SsSetTickMode(long tick_mode)
{
    long video;

    video = GetVideoMode();
    if (tick_mode & SS_NOTICK) {
        _snd_seq_tick_env.manual = 1;
        _snd_seq_tick_env.tick_mode = tick_mode & 0xfff;
    } else {
        _snd_seq_tick_env.manual = 0;
        _snd_seq_tick_env.tick_mode = tick_mode;
    }
    if (_snd_seq_tick_env.tick_mode < SS_TICKMODE_MAX) {
        switch (_snd_seq_tick_env.tick_mode) {
        case SS_TICK50:
            VBLANK_MINUS = 50;
            if (video == MODE_PAL) {
                _snd_seq_tick_env.tick_mode = SS_TICKVSYNC;
            } else {
                _snd_seq_tick_env.tick_mode = 50;
            }
            break;
        case SS_TICK60:
            VBLANK_MINUS = 60;
            if (video == MODE_NTSC) {
                _snd_seq_tick_env.tick_mode = SS_TICKVSYNC;
            } else {
                _snd_seq_tick_env.tick_mode = 60;
            }
            break;
        case SS_TICK120:
            VBLANK_MINUS = 120;
            break;
        case SS_TICK240:
            VBLANK_MINUS = 240;
            break;
        case SS_TICKVSYNC:
            switch (video) {
            case MODE_NTSC:
                VBLANK_MINUS = 60;
                break;
            case MODE_PAL:
                VBLANK_MINUS = 50;
                break;
            default:
                VBLANK_MINUS = 60;
                break;
            }
            break;
        case SS_NOTICK0:
            if (video == MODE_NTSC) {
                VBLANK_MINUS = 60;
            } else if (video == MODE_PAL) {
                VBLANK_MINUS = 50;
            } else {
                VBLANK_MINUS = 60;
            }
            break;
        default:
            VBLANK_MINUS = 60;
            break;
        }
    } else {
        VBLANK_MINUS = _snd_seq_tick_env.tick_mode;
    }
}
