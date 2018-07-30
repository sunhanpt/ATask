#include "QueuedThreadPool.h"
#include "CriticalSection.h"
#include "IQueuedWork.h"
#include <vector>

class FQueuedThreadPoolBase : public FQueuedThreadPool
{
public:
	virtual bool Create(int InNunQueuedThread, int StackSize, EThreadPriority ThreadPriority = TPri_Normal);
	virtual void Destory();
	virtual void AddQueuedWork(IQueuedWork* InQueuedWork);
	virtual bool RetractQueuedWork(IQueuedWork* InQueuedWork);
	virtual IQueuedWork* ReturnToPoolOrGetNextJob(FQueuedThread* InQueuedThread);
	virtual int GetNumThreads();

private:
	FCriticalSection *SyncObject;
	std::vector<IQueuedWork*> QueuedWorks;
	std::vector<FQueuedThread*> QueuedThreads;
	std::vector<FQueuedThread*> AllThreads;
	bool TimeToDie;
};

FQueuedThreadPool* GThreadPool = nullptr;
int FQueuedThreadPool::OvveridStackSize = 0;
FQueuedThreadPool* FQueuedThreadPool::Allocate()
{
	return new FQueuedThreadPoolBase;
}

/////////////////////////////////////*FQueuedThreadPoolBase*/////////////////////////////////////

bool FQueuedThreadPoolBase::Create(int InNunQueuedThread, int StackSize, EThreadPriority ThreadPriority /* = TPri_Normal */)
{
	return true;
}

void FQueuedThreadPoolBase::Destory()
{

}

void FQueuedThreadPoolBase::AddQueuedWork(IQueuedWork* InQueuedWork)
{

}

bool FQueuedThreadPoolBase::RetractQueuedWork(IQueuedWork* InQueuedWork)
{
	return true;
}

IQueuedWork* FQueuedThreadPoolBase::ReturnToPoolOrGetNextJob(FQueuedThread* InQueuedThread)
{
	return nullptr;
}

int FQueuedThreadPoolBase::GetNumThreads()
{
	return 0;
}

