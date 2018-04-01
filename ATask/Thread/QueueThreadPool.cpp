#include <queue>
#include <vector>
#include "QueuedThreadPool.h"

class IQueuedWork;
class FQueuedThread;

class FQueuedThreadPoolBase : public FQueuedThreadPool
{
public:
	FQueuedThreadPoolBase() {}
	virtual ~FQueuedThreadPoolBase() {}

public:

protected:
	std::queue<IQueuedWork*> QueuedWorks;
	std::queue<FQueuedThread*> QueuedThreads;
	std::vector<FQueuedThread*> AllThreads;
};

uint32_t FQueuedThreadPool::OverrideStackSize = 0;
FQueuedThreadPool* FQueuedThreadPool::Allocate()
{
	return new FQueuedThreadPoolBase;
}