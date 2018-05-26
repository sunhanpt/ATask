#pragma once
class FEvent
{
public:
	virtual bool Create(bool bIsManualReset = false) = 0;
	virtual bool IsManualReset() = 0;
	virtual void Trigger() = 0;
	virtual void Reset() = 0;
	virtual bool Wait(unsigned int WaitTime) = 0;
	virtual bool Wait()
	{
		return Wait(0xffffffff);
	}

	FEvent(){}

	virtual ~FEvent() {}
};