#pragma once
class FEvent
{
public:
	virtual bool Create(bool bIsManualReset = false) = 0;
	virtual bool IsManualReset() = 0;
	virtual void Trigger() = 0;
	virtual void Reset() = 0;
	virtual bool Wait(unsigned int WaitTime);
	virtual bool Wait()
	{
		return Wait(0xffffffff);
	}

	FEvent() : EventId(0){}

	virtual ~FEvent() {}

private:
	/** Counter used to generate an unique id for the events. */
	static unsigned int EventUniqueId;

	/** An unique id of this event. */
	unsigned int EventId;
};