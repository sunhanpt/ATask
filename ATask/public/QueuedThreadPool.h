#pragma once
#include "RunnableThread.h"

class IQueuedWork;
class FQueuedThread;
class FQueuedThreadPool
{
public:
	virtual ~FQueuedThreadPool() {}
	virtual bool Create(int InNunQueuedThread, int StackSize, EThreadPriority ThreadPriority = TPri_Normal) = 0;
	virtual void Destory() = 0;
	virtual void AddQueuedWork(IQueuedWork* InQueuedWork) = 0;
	virtual bool RetractQueuedWork(IQueuedWork* InQueuedWork) = 0;
	virtual IQueuedWork* ReturnToPoolOrGetNextJob(FQueuedThread* InQueuedThread) = 0;
	virtual int GetNumThreads() = 0;

public:
	static FQueuedThreadPool* Allocate();
	static int OvveridStackSize;
};

extern FQueuedThreadPool* GThreadPool; // 程序启动时创建