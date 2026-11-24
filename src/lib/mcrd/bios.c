/* Memory card BIOS event handlers and polling. */
#include <sys/types.h>
#include <kernel.h>
#include <libapi.h>

static long swIOE, swError, swTimeout, swNewcard;
static long hwIOE, hwError, hwTimeout, hwNewcard;

/* The event callbacks set these latches asynchronously. */
static volatile int swIOESeen, swErrorSeen, swTimeoutSeen, swNewcardSeen;
static volatile int hwIOESeen, hwErrorSeen, hwTimeoutSeen, hwNewcardSeen;

void _clr_card_event(void);

long funcEvSpIOE(void)
{
	swIOESeen = 1;
	return 0;
}

long funcEvSpError(void)
{
	swErrorSeen = 1;
	return 0;
}

long funcEvSpTimeout(void)
{
	swTimeoutSeen = 1;
	return 0;
}

long funcEvSpNewcard(void)
{
	swNewcardSeen = 1;
	return 0;
}

long funcEvSpIOEx(void)
{
	hwIOESeen = 1;
	return 0;
}

long funcEvSpErrorx(void)
{
	hwErrorSeen = 1;
	return 0;
}

long funcEvSpTimeoutx(void)
{
	hwTimeoutSeen = 1;
	return 0;
}

long funcEvSpNewcardx(void)
{
	hwNewcardSeen = 1;
	return 0;
}

void _card_open(long val)
{
	InitCARD(val);
	StartCARD();
	_bu_init();
}

void _card_start(void)
{
	int entered = EnterCriticalSection();

	swIOE = OpenEvent(SwCARD, EvSpIOE, EvMdINTR, funcEvSpIOE);
	swError = OpenEvent(SwCARD, EvSpERROR, EvMdINTR, funcEvSpError);
	swTimeout = OpenEvent(SwCARD, EvSpTIMOUT, EvMdINTR, funcEvSpTimeout);
	swNewcard = OpenEvent(SwCARD, EvSpNEW, EvMdINTR, funcEvSpNewcard);
	hwIOE = OpenEvent(HwCARD, EvSpIOE, EvMdINTR, funcEvSpIOEx);
	hwError = OpenEvent(HwCARD, EvSpERROR, EvMdINTR, funcEvSpErrorx);
	hwTimeout = OpenEvent(HwCARD, EvSpTIMOUT, EvMdINTR, funcEvSpTimeoutx);
	hwNewcard = OpenEvent(HwCARD, EvSpNEW, EvMdINTR, funcEvSpNewcardx);

	EnableEvent(swIOE);
	EnableEvent(swError);
	EnableEvent(swTimeout);
	EnableEvent(swNewcard);
	EnableEvent(hwIOE);
	EnableEvent(hwError);
	EnableEvent(hwTimeout);
	EnableEvent(hwNewcard);
	_clr_card_event();
	if (entered == 1)
		ExitCriticalSection();
}

void _card_close(void)
{
	StopCARD();
}

void _card_stop(void)
{
	int entered = EnterCriticalSection();

	CloseEvent(swIOE);
	CloseEvent(swError);
	CloseEvent(swTimeout);
	CloseEvent(swNewcard);
	CloseEvent(hwIOE);
	CloseEvent(hwError);
	CloseEvent(hwTimeout);
	CloseEvent(hwNewcard);
	if (entered == 1)
		ExitCriticalSection();
}

void _clr_card_event(void)
{
	TestEvent(swIOE);
	TestEvent(swError);
	TestEvent(swTimeout);
	TestEvent(swNewcard);
	TestEvent(hwIOE);
	TestEvent(hwError);
	TestEvent(hwTimeout);
	TestEvent(hwNewcard);
	/* The chained assignments read each volatile latch back before storing the next. */
	swIOESeen = swErrorSeen = swTimeoutSeen = swNewcardSeen = 0;
	hwIOESeen = hwErrorSeen = hwTimeoutSeen = hwNewcardSeen = 0;
}

/* Unlike the checks below, the waits omit the I/O completion bit from the result. */
int _get_card_event(void)
{
	int events;

	do {
		events = swIOESeen + (swErrorSeen << 1) + (swTimeoutSeen << 2) + (swNewcardSeen << 3);
	} while (events == 0);
	TestEvent(hwIOE);
	TestEvent(hwError);
	TestEvent(hwTimeout);
	TestEvent(hwNewcard);
	swIOESeen = swErrorSeen = swTimeoutSeen = swNewcardSeen = 0;
	return events >> 1;
}

int _get_card_event_x(void)
{
	int events;

	do {
		events = hwIOESeen + (hwErrorSeen << 1) + (hwTimeoutSeen << 2) + (hwNewcardSeen << 3);
	} while (events == 0);
	TestEvent(swIOE);
	TestEvent(swError);
	TestEvent(swTimeout);
	TestEvent(swNewcard);
	hwIOESeen = hwErrorSeen = hwTimeoutSeen = hwNewcardSeen = 0;
	return events >> 1;
}

int _chk_card_event(void)
{
	return swIOESeen + (swErrorSeen << 1) + (swTimeoutSeen << 2) + (swNewcardSeen << 3);
}

int _chk_card_event_x(void)
{
	return hwIOESeen + (hwErrorSeen << 1) + (hwTimeoutSeen << 2) + (hwNewcardSeen << 3);
}
