#include <queue>
#include <vector>
#include "../HAL/HAL.h"
#include "Runnable.h"
#include "QueuedThreadPool.h"


class IQueuedWork;
class FQueuedThread;

class FQueuedThreadPoolBase : public FQueuedThreadPool
{
public:
	FQueuedThreadPoolBase() : SyncQueue(nullptr), TimeToDie(false){}
	virtual ~FQueuedThreadPoolBase() { Destory(); }

public:
	virtual bool Create(uint32_t InNumQueuedThreads, uint32_t StackSize = (32 * 1024), EThreadPriority ThreadPriority = TPri_Normal) override;
	virtual void Destory() override;
	virtual void QueuedThreadWork(IQueuedWork* InQueuedWork) override;
	virtual bool RetractQueuedWork(IQueuedWork* InQueuedWork) override;
	virtual IQueuedWork* ReturnToPoolOrGetNextJob(class FQueuedThread* InQueuedThread) override;
	virtual int32_t GetNumThreads() const override
	{
		return AllThreads.size();
	}

protected:
	std::queue<IQueuedWork*>		QueuedWorks;
	std::queue<FQueuedThread*>		QueuedThreads;
	std::vector<FQueuedThread*>		AllThreads;

	FCriticalSection*				SyncQueue;
	bool							TimeToDie;
};


bool FQueuedThreadPoolBase::Create(uint32_t InNumQueuedThreads, uint32_t StackSize /* = (32 * 1024) */, EThreadPriority ThreadPriority /* = TPri_Normal */)
{

}

void FQueuedThreadPoolBase::Destory()
{

}

void FQueuedThreadPoolBase::QueuedThreadWork(IQueuedWork* InQueuedWork)
{

}

bool FQueuedThreadPoolBase::RetractQueuedWork(IQueuedWork* InQueuedWork)
{

}

IQueuedWork* FQueuedThreadPoolBase::ReturnToPoolOrGetNextJob(class FQueuedThread* InQueuedThread)
{

}

uint32_t FQueuedThreadPool::OverrideStackSize = 0;
FQueuedThreadPool* FQueuedThreadPool::Allocate()
{
	return new FQueuedThreadPoolBase;
}


/////////////////////////////////////QueuedThread/////////////////////////////////////
class FQueuedThread : public FRunnable
{
public:
};