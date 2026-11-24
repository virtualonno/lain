/* The host log: main and InitProgram pass OpenHostLog the path d:\usr\tmp\psx.log,
   and main and the heap pass HostLogPrintf their diagnostics. Every routine here is empty. */
#include "host_log.h"

void HostLogPrintf(const char *format, ...)
{
}

void OpenHostLog(const char *path)
{
}

void UnusedHostLogStub1(void)
{
}

void UnusedHostLogStub2(void)
{
}
