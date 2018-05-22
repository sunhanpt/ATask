#pragma once
#include "RunnableThread.h"

class FEvent;
class FQueuedThreadPool;
class IQueuedWork;
class FQueuedThread
{
public:
	FQueuedThread() {}
	virtual ~FQueuedThread() {}

public:
	virtual int Run();
	virtual bool Create(FQueuedThreadPool* InOwningThreadPool, int StackSize = 0, EThreadPriority ThreadPriority = TPri_Normal);
	virtual bool KillThread();
	virtual void DoWork();

private:
	FQueuedThreadPool* OwnThreadPool;
	FEvent* DoWorkEvent;
	volatile int TimeToDie;
	IQueuedWork* volatile QueuedWork;
	FRunnableThread* Thread;
};