#pragma once
#include "Event.h"
#include "windows.h"
class FEventWin : public FEvent
{
public:

	/** Default constructor. */
	FEventWin()
		: Event(nullptr)
	{ }

	/** Virtual destructor. */
	virtual ~FEventWin()
	{
		if (Event != nullptr)
		{
			CloseHandle(Event);
		}
	}

	// FEvent interface

	virtual bool Create(bool bIsManualReset = false) override
	{
		// Create the event and default it to non-signaled
		Event = CreateEvent(nullptr, bIsManualReset, 0, nullptr);
		ManualReset = bIsManualReset;

		return Event != nullptr;
	}

	virtual bool IsManualReset() override
	{
		return ManualReset;
	}

	virtual void Trigger() override;
	virtual void Reset() override;
	virtual bool Wait(unsigned int WaitTime, const bool bIgnoreThreadIdleStats = false) override;

private:
	/** Holds the handle to the event. */
	HANDLE Event;

	/** Whether the signaled state of the event needs to be reset manually. */
	bool ManualReset;
};