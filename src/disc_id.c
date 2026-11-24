/* Disc identity and the disc-change poll: the drive's shell and seek status
 * through CdControlB, and which disc is in by its LAIN_1.INF or LAIN_2.INF. */
#include <sys/types.h>
#include <libcd.h>
#include "disc_id.h"

short gDiscPollStarted = 0;
short gDiscShellOpen; /* CdlStatShellOpen bit of the last status the poll kept */
short gDiscMotorPollsLeft; /* polls left while the shell is closed and the motor is not on */
short gDiscErrorPollsLeft; /* polls left while the status reports an error */

enum DiscSeekResult {
    DISC_SEEK_TIMEOUT,
    DISC_SEEK_ERROR,
    DISC_SEEK_OK,
    DISC_SEEK_SHELL_OPEN = CdlStatShellOpen
};

static __inline__ int SeekToDiscStartInline(void)
{
    u_char result[8];
    CdlLOC loc;
    int retries;

    retries = 60;
    loc.minute = 0;
    loc.second = 0;
    loc.sector = 0;
    loc.track = 0;
    for (;;) {
        CdControlB(CdlSeekL, (u_char *)&loc, result);
        if (result[0] & CdlStatError) {
            if (result[1] & 0x40) {
                return DISC_SEEK_ERROR;
            }
            if (result[0] & CdlStatShellOpen) {
                return DISC_SEEK_SHELL_OPEN;
            }
            if (retries == 0) {
                return DISC_SEEK_TIMEOUT;
            }
            retries--;
        } else {
            return DISC_SEEK_OK;
        }
    }
}

static __inline__ void StartDiscPollInline(void)
{
    u_char status[8];
    int mode;

    mode = 0;
    CdControlB(CdlSetmode, (u_char *)&mode, status);
    CdControlB(CdlNop, 0, status);
    gDiscPollStarted = 1;
    gDiscMotorPollsLeft = 300;
    gDiscErrorPollsLeft = 600;
    gDiscShellOpen = status[0] & CdlStatShellOpen;
}

/* Watches the drive through blocking CdControlB calls; resets to wait for the lid after any
 * terminal status. */
int PollDiscChange(void)
{
    u_char status[8];

    if (gDiscPollStarted == 0) {
        StartDiscPollInline();
    }
    CdControlB(CdlNop, 0, status);
    if (gDiscShellOpen & CdlStatShellOpen) {
        if (!(status[0] & CdlStatShellOpen)) {
            if (status[0] & CdlStatStandby) {
                gDiscMotorPollsLeft = 300;
                if (CdControlB(CdlGetTN, 0, status) == 0) {
                    return DISC_POLL_CHECKING;
                }
                if (status[0] & CdlStatError) {
                    if (--gDiscErrorPollsLeft == 0) {
                        gDiscShellOpen = status[0] & CdlStatShellOpen;
                        gDiscErrorPollsLeft = 600;
                        return DISC_POLL_NO_DISC;
                    }
                }
                if (status[0] & ~CdlStatStandby) {
                    return DISC_POLL_CHECKING;
                }
                gDiscErrorPollsLeft = 600;
                switch (SeekToDiscStartInline()) {
                case DISC_SEEK_OK:
                    gDiscPollStarted = 0;
                    return DISC_POLL_READY;
                case DISC_SEEK_ERROR:
                    gDiscShellOpen = status[0] & CdlStatShellOpen;
                    return DISC_POLL_WRONG_DISC;
                case DISC_SEEK_SHELL_OPEN:
                    gDiscShellOpen = status[0] & CdlStatShellOpen;
                    return DISC_POLL_WAIT_LID;
                case DISC_SEEK_TIMEOUT:
                    gDiscShellOpen = status[0] & CdlStatShellOpen;
                    return DISC_POLL_NO_DISC;
                }
                return DISC_POLL_CHECKING;
            } else {
                if (--gDiscMotorPollsLeft == 0) {
                    gDiscShellOpen = status[0] & CdlStatShellOpen;
                    gDiscMotorPollsLeft = 300;
                    return DISC_POLL_NO_DISC;
                }
                return DISC_POLL_CHECKING;
            }
        }
    } else {
        gDiscShellOpen = status[0] & CdlStatShellOpen;
    }
    return DISC_POLL_WAIT_LID;
}

int SeekToDiscStart(void)
{
    return SeekToDiscStartInline();
}

void StartDiscPoll(void)
{
    StartDiscPollInline();
}

int IdentifyDisc(void)
{
    CdlFILE file;

    if (CdSearchFile(&file, "\\LAIN_1.INF;1") != 0) {
        return 0;
    }
    if (CdSearchFile(&file, "\\LAIN_2.INF;1") != 0) {
        return 1;
    }
    return -1;
}
