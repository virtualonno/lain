#ifndef DISC_ID_H
#define DISC_ID_H

enum DiscPollStatus {
    DISC_POLL_NO_DISC = -2,
    DISC_POLL_WRONG_DISC,
    DISC_POLL_WAIT_LID,
    DISC_POLL_CHECKING,
    DISC_POLL_READY
};

int PollDiscChange(void);
int IdentifyDisc(void);

#endif
