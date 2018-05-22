#include "FEventWin.h"

void FEventWin::Trigger()
{
	SetEvent(Event);
}

void FEventWin::Reset()
{
	ResetEvent(Event);
}

bool FEventWin::Wait(unsigned int WaitTime)
{
	return (WaitForSingleObject(Event, WaitTime) == WAIT_OBJECT_0);
}