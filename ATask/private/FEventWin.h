#pragma once
#include "Event.h"
#include <windows.h>
class FEventWin : public FEvent
{
public:
	FEventWin() : Event(nullptr) {}
	virtual ~FEventWin()
	{
		if (Event)
		{
			CloseHandle(Event);
		}
	}

	virtual bool Create(bool bIsManualReset  = false) override
	{
		Event = CreateEvent(nullptr, bIsManualReset, 0, nullptr);
		ManualReset = bIsManualReset;
		return Event != nullptr;
	}

	virtual void Trigger() override;
	virtual void Reset() override;
	virtual bool Wait(unsigned int WaitTime) override;
private:
	/** Holds the handle to the event. */
	HANDLE Event;

	/** Whether the signaled state of the event needs to be reset manually. */
	bool ManualReset;
};