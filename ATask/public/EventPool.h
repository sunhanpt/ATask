#pragma once;
#include "LockFreeList.h"
#include "Event.h"

// Event分为开启（有信号）和关闭(无信号）两个状态。手动开启Event用SetEvent，手动关闭用ResetEvent。处于无信号时，线程sleep。
// 当设置成自动Event时，WaitForSingleObject接口会等待Event开启，开启之后，会自动把Event关闭。也就是说，手动的Event可以被多个线程监听
// 自动的Event只能被一个线程监听。
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