#include "WindowsPlatformProcess.h"
#include "WindowsEvent.h"
#include <assert.h>
bool FWindowsPlatformProcess::SupportsMultithreading()
{
	return true; // determined by cmd input parameters.
}



/////////////////////////////////////*Event*/////////////////////////////////////
bool FEventWin::Wait(unsigned int WaitTime, const bool bIgnoreThreadIdleStats /*= false*/)
{
	assert(Event);
	return (WaitForSingleObject(Event, WaitTime) == WAIT_OBJECT_0);
}

void FEventWin::Trigger()
{
	assert(Event);
	SetEvent(Event);
}

void FEventWin::Reset()
{
	assert(Event);
	ResetEvent(Event);
}