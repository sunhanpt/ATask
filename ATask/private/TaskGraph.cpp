#include "TaskGraphInterface.h"
#include "Runnable.h"
#include "SingleThreadRunnable.h"
#include "RunnableThread.h"
#include "WindowsPlatformTLS.h"
#include "Event.h"
#include "CriticalSection.h"
#include <vector>


#define CREATE_HIPRI_TASK_THREADS (1)
#define CREATE_BACKGROUND_TASK_THREADS (1)

class FTaskThreadBase;
class FTaskGraphImplementation;
static FTaskGraphImplementation* TaskGraphImplementationSingleton = nullptr;

/**
*	FWorkerThread
*	Helper structure to aggregate a few items related to the individual threads.
**/
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
	virtual void ProcessTasksUntilIdle(int32 QueueIndex) { assert(0); }

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
		ProcessTasksUntilIdle(0);
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

protected:
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
	// 运行task，直到被标记需要Quit。
	virtual void ProcessTasksUntilQuit(int32 QueueIndex) override
	{
		assert(Queue(QueueIndex).StallRestartEvent); // started up.
		Queue(QueueIndex).QuitForReturn = false;
		assert(++Queue(QueueIndex).RecursionGuard == 1);
		do 
		{
			ProcessTasksNamedThread(QueueIndex, FPlatformProcess::SupportsMultithreading());
		} while (!Queue(QueueIndex).QuitForReturn && !Queue(QueueIndex).QuitForShutdown && FPlatformProcess::SupportsMultithreading());
		assert(!--Queue(QueueIndex).RecursionGuard);
	}

	// 运行tasks，直到task为空也即idle。（在这个周期内，线程不会再次开启）
	virtual void ProcessTasksUntilIdle(int32 QueueIndex) override
	{
		assert(Queue(QueueIndex).StallRestartEvent); // started up.
		Queue(QueueIndex).QuitForReturn = false;
		assert(++Queue(QueueIndex).RecursionGuard == 1);
		ProcessTasksNamedThread(QueueIndex, false);
		assert(!--Queue(QueueIndex).RecursionGuard);
	}

	void ProcessTasksNamedThread(int32 QueueIndex, bool bAllowStall)
	{
		while (!Queue(QueueIndex).QuitForReturn)
		{
			FBaseGraphTask* Task = Queue(QueueIndex).StallQueue.Pop(0, bAllowStall);
			if (!Task)
			{
				if (bAllowStall)
				{
					Queue(QueueIndex).StallRestartEvent->Wait(MAX_uint32);
					if (Queue(QueueIndex).QuitForShutdown)
						return;
				}
				else
				{
					break;
				}
			}
			else
			{
				Task->Execute(NewTasks, ENamedThreads::Type(ThreadId | (QueueIndex << ENamedThreads::QueueIndexShift)));
			}
		}
	}

	virtual void EnqueueFromThisThread(int32 QueueIndex, FBaseGraphTask* Task) override
	{
		assert(Task && Queue(QueueIndex).StallRestartEvent);
		uint32 PriIndex = ENamedThreads::GetTaskPriority(Task->ThreadToExecuteOn) ? 0 : 1;
		int32 ThreadToStart = Queue(QueueIndex).StallQueue.Push(Task, PriIndex);
		assert(ThreadToStart);
	}

	virtual bool EnqueueFromOtherThread(int32 QueueIndex, FBaseGraphTask* Task) override
	{
		assert(Task && Queue(QueueIndex).StallRestartEvent);

		uint32 PriIndex = ENamedThreads::GetTaskPriority(Task->ThreadToExecuteOn) ? 0 : 1;
		int32 ThreadToStart = Queue(QueueIndex).StallQueue.Push(Task, PriIndex);

		if (ThreadToStart >= 0)
		{
			assert(ThreadToStart == 0);
			Queue(QueueIndex).StallRestartEvent->Trigger();
			return true;
		}
		return false;
	}

	virtual void RequestQuit(int32 QueueIndex)
	{
		if (!Queue(0).StallRestartEvent)
			return;
		if (QueueIndex == -1)
		{
			assert(Queue(0).StallRestartEvent && Queue(1).StallRestartEvent);
			Queue(0).QuitForShutdown = true;
			Queue(1).QuitForShutdown = true;
			Queue(0).StallRestartEvent->Trigger();
			Queue(1).StallRestartEvent->Trigger();
		}
		else
		{
			assert(Queue(QueueIndex).StallRestartEvent);
			Queue(QueueIndex).QuitForReturn = true;
		}
	}

	virtual bool IsProcessingTasks(int32 QueueIndex)
	{
		return !!Queue(QueueIndex).RecursionGuard;
	}
private:
	struct FThreadTaskQueue 
	{
		FStallingTaskQueue<FBaseGraphTask, PLATFORM_CACHE_LINE_SIZE, 2> StallQueue;

		/** We need to disallow reentry of the processing loop **/
		uint32 RecursionGuard;

		/** Indicates we executed a return task, so break out of the processing loop. **/
		bool QuitForReturn;

		/** Indicates we executed a return task, so break out of the processing loop. **/
		bool QuitForShutdown;

		/** Event that this thread blocks on when it runs out of work. **/
		FEvent*	StallRestartEvent;

		FThreadTaskQueue()
			: RecursionGuard(0)
			, QuitForReturn(false)
			, QuitForShutdown(false)
			, StallRestartEvent(FPlatformProcess::GetSynchEventFromPool(false))
		{

		}

		~FThreadTaskQueue()
		{
			FPlatformProcess::ReturnSynchEventToPool(StallRestartEvent);
			StallRestartEvent = nullptr;
		}
	};
	FThreadTaskQueue Queues[ENamedThreads::NumQueues];

private:
	FORCEINLINE FThreadTaskQueue& Queue(int32 QueueIndex)
	{
		assert(QueueIndex >= 0 && QueueIndex < ENamedThreads::NumQueues);
		return Queues[QueueIndex];
	}

	FORCEINLINE const FThreadTaskQueue& Queue(int32 QueueIndex) const
	{
		assert(QueueIndex >= 0 && QueueIndex < ENamedThreads::NumQueues);
		return Queues[QueueIndex];
	}
};

/**
*	FTaskThreadAnyThread
*	A class for managing a worker threads.
**/
class FTaskThreadAnyThread : public FTaskThreadBase
{
	FTaskThreadAnyThread(int32 InPriorityIndex)
		: PriorityIndex(InPriorityIndex)
	{
	}

	virtual void ProcessTasksUntilQuit(int32 QueueIndex) override
	{
		//if (PriorityIndex != (ENamedThreads::BackgroundThreadPriority >> ENamedThreads::ThreadPriorityShift))
		//{
		//	FMemory::SetupTLSCachesOnCurrentThread();
		//}
		assert(!QueueIndex);
		do 
		{
			ProcessTasks();
		} while (!Queue.QuitForShutdown && FPlatformProcess::SupportsMultithreading());
	}

	virtual void ProcessTasksUntilIdle(int32 QueueIndex) override
	{
		if (!FPlatformProcess::SupportsMultithreading())
		{
			ProcessTasks();
		}
		else
		{
			assert(0);
		}
	}

	virtual void RequestQuit(int32 QueueIndex)
	{
		assert(QueueIndex < 0);
		assert(Queue.StallRestartEvent);
		Queue.QuitForShutdown = true;
		Queue.StallRestartEvent->Trigger();
	}

	virtual void WakeUp() final override
	{
		Queue.StallRestartEvent->Trigger();
	}

	void StallForTunning(bool Stall)
	{
		if (Stall)
		{
			Queue.StallForTuning.Lock();
			Queue.bStallForTuning = true;
		}
		else
		{
			Queue.bStallForTuning = false;
			Queue.StallForTuning.UnLock();
		}
	}

	virtual bool IsProcessingTasks(int32 QueueIndex) override
	{
		assert(!QueueIndex);
		return !!Queue.RecursionGuard;
	}

private:
	FBaseGraphTask* FindWork();

	void ProcessTasks()
	{
		++Queue.RecursionGuard;
		bool bDidStall = false;
		while (1)
		{
			FBaseGraphTask* Task = FindWork();
			if (!Task)
			{
				if (FPlatformProcess::SupportsMultithreading())
				{
					Queue.StallRestartEvent->Wait(MAX_uint32);
					bDidStall = true;
				}

				if (Queue.QuitForShutdown || !FPlatformProcess::SupportsMultithreading())
				{
					break;
				}
				continue;
			}

			// the Win scheduler is ill behaved and will sometimes let BG tasks run even when other tasks are ready....kick the scheduler between tasks
			if (!bDidStall && PriorityIndex == (ENamedThreads::BackgroundThreadPriority >> ENamedThreads::ThreadPriorityShift))
			{
				FPlatformProcess::Sleep(0);
			}
			bDidStall = false;
			Task->Execute(NewTasks, ENamedThreads::Type(ThreadId));
		}
		--Queue.RecursionGuard;
	}

private:
	struct FThreadTaskQueue
	{
		FEvent* StallRestartEvent;
		uint32	RecursionGuard;
		bool	QuitForShutdown;
		bool	bStallForTuning;
		FCriticalSection StallForTuning;

		FThreadTaskQueue()
			: StallRestartEvent(FPlatformProcess::GetSynchEventFromPool(false))
			, RecursionGuard(0)
			, QuitForShutdown(false)
			, bStallForTuning(false)
		{

		}

		~FThreadTaskQueue()
		{
			FPlatformProcess::ReturnSynchEventToPool(StallRestartEvent);
			StallRestartEvent = nullptr;
		}
	};
	
	FThreadTaskQueue Queue;

	int32 PriorityIndex;
};


class FTaskGraphImplemention : public FTaskGraphInterface
{
public:
	static FTaskGraphImplemention& Get()
	{
		return *TaskGraphImplementationSingleton;
	}

	FTaskGraphImplemention(int32)
	{
		// TODO:
	}

	virtual ~FTaskGraphImplemention()
	{

	}

	virtual void QueueTask(class FBaseGraphTask* Task, ENamedThreads::Type ThreadToExecuteOn, ENamedThreads::Type CurrentThreadIfKnown /* = ENamedThreads::AnyThread */)
	{

	}
private:

	FTaskThreadBase& Thread(int32 Index)
	{
		assert(Index >= 0 && Index < NumThreads);
		assert(WorkerThreads[Index].TaskGraphWorker->GetThreadId() == Index);
		return *WorkerThreads[Index].TaskGraphWorker;
	}

	ENamedThreads::Type GetCurrentThread()
	{
		ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread;
		FWorkerThread* TLSPointer = (FWorkerThread*)FPlatformTLS::GetTlsValueUE(PerThreadIDTLSSlot);
		if (TLSPointer)
		{
			assert(TLSPointer - WorkerThreads >= 0 && TLSPointer - WorkerThreads < NumThreads);
			int32 ThreadIndex = TLSPointer - WorkerThreads;
			assert(Thread(ThreadIndex).GetThreadId() == ThreadIndex);
			if (ThreadIndex < NumNamedThreads)
			{
				CurrentThreadIfKnown = ENamedThreads::Type(ThreadIndex);
			}
			else
			{
				int32 Priority = (ThreadIndex - NumNamedThreads) / NumTaskThreadsPerSet;
				CurrentThreadIfKnown = ENamedThreads::SetPriorities(ENamedThreads::Type(ThreadIndex), Priority, false);
			}
		}
		return CurrentThreadIfKnown;
	}

	int32 ThreadIndexToPriorityIndex(int32 ThreadIndex)
	{
		assert(ThreadIndex >= NumNamedThreads && ThreadIndex < NumThreads);
		int32 Result = (ThreadIndex - NumNamedThreads) / NumTaskThreadsPerSet;
		assert(Result >= 0 && Result < NumTaskThreadSets);
		return Result;
	}

	enum
	{
		/** Compile time maximum number of threads. Didn't really need to be a compile time constant, but task thread are limited by MAX_LOCK_FREE_LINKS_AS_BITS **/
		// 这里的26是跟LockFreeList中指针所占的位数有关的，指针占了26位。
		MAX_THREADS = 26 * (CREATE_HIPRI_TASK_THREADS + CREATE_BACKGROUND_TASK_THREADS + 1) + ENamedThreads::ActualRenderingThread + 1,
		MAX_THREAD_PRIORITIES = 3
	};

	FWorkerThread		WorkerThreads[MAX_THREADS];
	int32				NumThreads;					/** Number of threads actually in use. **/
	int32				NumNamedThreads;			/** Number of named threads actually in use. **/
	int32				NumTaskThreadSets;			/** Number of tasks thread sets for priority **/
	int32				NumTaskThreadsPerSet;		/** Number of tasks threads per priority set **/
	bool				bCreatedHiPriorityThreads;
	bool				bCreatedBackgroundPriorityThreads;

	/**
	* "External Threads" are not created, the thread is created elsewhere and makes an explicit call to run
	* Here all of the named threads are external but that need not be the case.
	* All unnamed threads must be internal
	**/
	ENamedThreads::Type LastExternalThread;
	FThreadSafeCounter	ReentrancyCheck;
	/** Index of TLS slot for FWorkerThread* pointer. **/
	uint32				PerThreadIDTLSSlot;

	/** Array of callbacks to call before shutdown. **/
	std::vector<std::function<void()> > ShutdownCallbacks;

	FStallingTaskQueue<FBaseGraphTask, PLATFORM_CACHE_LINE_SIZE, 2>	IncomingAnyThreadTasks[MAX_THREAD_PRIORITIES];
};


FBaseGraphTask* FTaskThreadAnyThread::FindWork()
{
	return nullptr; // TODO: 完成FTaskGraphImplementation需要重新完善此函数。
}

