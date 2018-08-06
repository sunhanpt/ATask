#pragma once;
#include "LockFreeList.h"
#include "Event.h"

enum EEventPoolType
{
	/** Creates events that have their signaled state reset automatically. */
	AutoReset,

	/** Creates events that have their signaled state reset manually. */
	ManualReset
};

// 为了包含FEvent,保证Event不被外界delete
class FSafeRecyclableEvent final : public FEvent
{
public:
	FEvent* InnerEvent;

	FSafeRecyclableEvent(FEvent* InInnerEvent)
		: InnerEvent(InInnerEvent)
	{
	}

	~FSafeRecyclableEvent()
	{
		InnerEvent = nullptr;
	}

	virtual bool Create(bool bIsManualReset = false)
	{
		return InnerEvent->Create(bIsManualReset);
	}

	virtual bool IsManualReset()
	{
		return InnerEvent->IsManualReset();
	}

	virtual void Trigger()
	{
		InnerEvent->Trigger();
	}

	virtual void Reset()
	{
		InnerEvent->Reset();
	}

	virtual bool Wait(unsigned int WaitTime)
	{
		return InnerEvent->Wait(WaitTime);
	}
};


template<EEventPoolType PoolType>
class FEventPool
{
public:
	static FEventPool& Get()
	{
		static FEventPool SingletonEventPool;
		return SingletonEventPool;
	}
	
	FEvent* GetEventFromPool()
	{
		FEvent* Event = Pool.Pop();
		if (!Event)
		{
			Event = FPlatformProcess::CreateSynchEvent((PoolType == EEventPoolType::ManualReset));
		}

		return new FSafeRecyclableEvent(Event);
	}

	void ReturnToPool(FEvent* Event)
	{
		assert(Event);
		assert(Event->IsManualReset() == (PoolType == EEventPoolType::ManualReset));
		FSafeRecyclableEvent* SafeEvent = (FSafeRecyclableEvent*)Event;
		FEvent* Result = SafeEvent->InnerEvent;
		delete SafeEvent;
		assert(Result);
		Result->Reset();
		Pool.Push(Result);
	}

private:
	TLockFreePointerListUnordered<FEvent, PLATFORM_CACHE_LINE_SIZE> Pool;

	FEventPool() = default;
	~FEventPool() = default;
};