#pragma once
#include "CoreTypes.h"
#include "Event.h"
#include <cassert>
class FSingleThreadEvent : public FEvent
{
public:
	FSingleThreadEvent()
		: bTriggered(false)
		, bManualReset(false)
	{

	}

public:
	virtual bool Create(bool bIsManualReset = false) override
	{
		bManualReset = bIsManualReset;
		return true;
	}

	virtual bool IsManualReset() override
	{
		return bManualReset;
	}

	virtual void Trigger() override
	{
		bTriggered = true;
	}

	virtual void Reset() override
	{
		bTriggered = false;
	}

	virtual bool Wait(unsigned int WaitTime) override
	{
		assert(bTriggered);
		bTriggered = bManualReset;
		return true;
	}
private:
	bool bTriggered;
	bool bManualReset;
};