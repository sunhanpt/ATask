#include "TaskGraphInterface.h"
#include "Runnable.h"
#include "SingleThreadRunnable.h"
#include "RunnableThread.h"
#include "WindowsPlatformTLS.h"
#include <vector>

/**
*	FWorkerThread
*	Helper structure to aggregate a few items related to the individual threads.
**/
class FTaskThreadBase;

struct FWorkerThread
{
	FTaskThreadBase*		TaskGraphWorker;	// runnable
	FRunnableThread*		RunnableThread;		// 真正的运行线程
	bool					bAttached;			// 是否在运行

	FWorkerThread()
		: TaskGraphWorker(nullptr)
		, RunnableThread(nullptr)
		, bAttached(false)
	{}
};

/**
*	FTaskThreadBase
*	Base class for a thread that executes tasks
*	This class implements the FRunnable API, but external threads don't use that because those threads are created elsewhere.
**/
class FTaskThreadBase : public FRunnable, public FSingleThreadRunnable
{
public:
	FTaskThreadBase()
		: ThreadId(ENamedThreads::AnyThread)
		, PerThreadIDTLSSlot(0xffffffff)
		, OwnerWorker(nullptr)
	{
		NewTasks.reserve(128);
	}

	// Sets up some basic information for a thread. Meant to be called from a "main" thread. Also creates the stall event.
	void Setup(ENamedThreads::Type InThreadId, uint32 InPerThreadIDTLSSlot, FWorkerThread* InOwnerWorker)
	{
		ThreadId = InThreadId;
		PerThreadIDTLSSlot = InPerThreadIDTLSSlot;
		OwnerWorker = InOwnerWorker;
		assert(ThreadId >= 0);
	}

	void InitializeForCurrentThread()
	{
		FPlatformTLS::SetTlsValueUE(PerThreadIDTLSSlot, OwnerWorker);
	}

	ENamedThreads::Type GetThreadId() const
	{
		assert(OwnerWorker);
		return ThreadId;
	}

	// Used for named threads to start processing tasks until the RequestQuit has been called.
	virtual void ProcessTasksUntilQuit(int32 QueueIndex) = 0;

	// Used for named threads to start processing tasks until the thread is idle has been called.
	virtual void ProcessTaskUntilIdle(int32 QueueIndex) { assert(0); }

	// Queue a task, assuming that this thread is the same as the current thread.
	// For named threads, these go directly into the private queue. 
	// 仅仅named threads 实现了此接口
	virtual void EnqueueFromThisThread(int32 QueueIndex, FBaseGraphTask* Task)
	{
		assert(0);
	}

	/**
	*	Will cause the thread to return to the caller when it becomes idle. Used to return from ProcessTasksUntilQuit for named threads or to shut down unnamed threads.
	*	CAUTION: This will not work under arbitrary circumstances. For example you should not attempt to stop unnamed threads unless they are known to be idle.
	*	Return requests for named threads should be submitted from that named thread as FReturnGraphTask does.
	**/
	virtual void RequestQuit(int32 QueueIndex) = 0;

	// Queue a task, assuming that this thread is not the same as the current thread. 仅仅在Named thread中实现了。
	virtual bool EnqueueFromOtherThread(int32 QueueIndex, FBaseGraphTask* Task)
	{
		assert(0);
		return false;
	}

	virtual void WakeUp()
	{
		assert(0);
	}

	// Return true if this thread is processing tasks. 
	// This is only a "guess" if you ask for a thread other than yourself because that can change before the function returns.
	virtual bool IsProcessingTasks(int32 QueueIndex) = 0;

	virtual void Tick() override
	{
		ProcessTaskUntilIdle(0);
	}

	// FRunnable API
	virtual bool Init() override
	{
		InitializeForCurrentThread();
		return true;
	}

	virtual uint32 Run() override
	{
		assert(OwnerWorker);
		ProcessTasksUntilQuit(0);
		return 0;
	}

	virtual void Stop() override
	{
		RequestQuit(-1);
	}

	virtual void Exit() override
	{
	}

	virtual FSingleThreadRunnable* GetSingleThreadInterface() override
	{
		return this;
	}

private:
	ENamedThreads::Type				ThreadId;
	uint32							PerThreadIDTLSSlot;
	FThreadSafeCounter				IsStalled;
	std::vector<FBaseGraphTask*>	NewTasks;
	FWorkerThread*					OwnerWorker;
};

/**
*	FNamedTaskThread
*	A class for managing a named thread.
*/
class FNamedTaskThread : public FTaskThreadBase
{
public:
private:
	struct FThreadTaskQueue 
	{

	};
};
