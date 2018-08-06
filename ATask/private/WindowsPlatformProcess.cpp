#include "WindowsPlatformProcess.h"
#include "FEventWin.h"
#include "SingleThreadEvent.h"
#include "EventPool.h"
bool FWindowsPlatformProcess::SupportsMultithreading()
{
	return true;
}

FEvent* FWindowsPlatformProcess::GetSynchEventFromPool(bool bIsManualReset)
{
	return bIsManualReset 
		? FEventPool<EEventPoolType::ManualReset>::Get().GetEventFromPool()
		: FEventPool<EEventPoolType::AutoReset>::Get().GetEventFromPool();
}

void FWindowsPlatformProcess::ReturnSynchEventToPool(FEvent* Event)
{
	if (!Event)
	{
		return;
	}

	if (Event->IsManualReset())
	{
		FEventPool<EEventPoolType::ManualReset>::Get().ReturnToPool(Event);
	}
	else
	{
		FEventPool<EEventPoolType::AutoReset>::Get().ReturnToPool(Event);
	}
}

FEvent* FWindowsPlatformProcess::CreateSynchEvent(bool bIsManualReset /* = false */)
{
	FEvent* Event = nullptr;
	if (FPlatformProcess::SupportsMultithreading())
	{
		Event = new FEventWin();
	}
	else
	{
		Event = new FSingleThreadEvent;
	}

	if (!Event->Create(bIsManualReset))
	{
		delete Event;
		Event = nullptr;
	}
	return Event;
}