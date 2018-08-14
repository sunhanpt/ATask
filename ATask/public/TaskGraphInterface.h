#pragma once
#include "CoreTypes.h"
#include <cassert>
#include <memory>
#include <vector>
#include <functional>
#include "WindowsPlatform.h"
#include "ThreadSafeCounter.h"
#include "LockFreeList.h"

namespace ENamedThreads
{
	enum Type
	{
		UnusedAnchor = -1,
#if STATS
		StatsThread,
#endif
		RHIThread,
		AudioThread,
		GameThread,
		ActualRenderingThread = GameThread + 1,
		AnyThread = 0xff,

		MainQueue = 0x000,
		LocalQueue = 0x100,

		NumQueues = 2,

		ThreadIndexMask = 0xff,
		QueueIndexMask = 0x100,
		QueueIndexShift = 8,

		NormalTaskPriority = 0x000,
		HighTaskPriority = 0x200,

		NumTaskPriorities = 2,
		TaskPriorityMask = 0x200,
		TaskPriorityShift = 9,

		NormalThreadPriority = 0x000,
		HighThreadPriority = 0x400,
		BackgroundThreadPriority = 0x800,

		NumThreadPriorities = 3,
		ThreadPriorityMask = 0xC00,
		ThreadPriorityShift = 10,

		/** Combinations **/
#if STATS
		StatsThread_Local = StatsThread | LocalQueue,
#endif

		GameThread_Local = GameThread | LocalQueue, // Name thread使用的都是localQueue
		ActualRenderingThread_Local = ActualRenderingThread | LocalQueue,

		AnyHiPriThreadNormalTask = AnyThread | HighThreadPriority | NormalTaskPriority,
		AnyHiPriThreadHiPriTask = AnyThread | HighThreadPriority | HighTaskPriority,

		AnyNormalThreadNormalTask = AnyThread | NormalThreadPriority | NormalTaskPriority,
		AnyNormalThreadHiPriTask = AnyThread | NormalThreadPriority | HighTaskPriority,

		AnyBackgroundThreadNormalTask = AnyThread | BackgroundThreadPriority | NormalTaskPriority,
		AnyBackgroundHiPriTask = AnyThread | BackgroundThreadPriority | HighTaskPriority,
	};

	extern int32 bHasBackgroundThreads;
	extern int32 bHasHighPriorityThreads;

	__forceinline Type GetThreadIndex(Type ThreadAndIndex)
	{
		// 是AnyThread的一个返回AnyThread(0xff), 否则返回NameThread的ID。
		return ((ThreadAndIndex & ThreadIndexMask) == AnyThread) ? AnyThread : Type(ThreadAndIndex & ThreadIndexMask);
	}

	__forceinline int32 GetQueueIndex(Type ThreadAndIndex)
	{
		return (ThreadAndIndex & QueueIndexMask) >> QueueIndexShift;
	}

	__forceinline int32 GetTaskPriority(Type ThreadAndIndex)
	{
		return (ThreadAndIndex & TaskPriorityMask) >> TaskPriorityShift;
	}

	__forceinline int32 GetThreadPriorityIndex(Type ThreadAndIndex)
	{
		int32 Result = (ThreadAndIndex & ThreadPriorityMask) >> ThreadPriorityShift;
		assert(Result >= 0 && Result < NumThreadPriorities);
		return Result;
	}

	__forceinline Type SetPriorities(Type ThreadAndIndex, Type ThreadPriority, Type TaskPriority)
	{
		assert(
			!(ThreadAndIndex & ~ThreadIndexMask)&&
			!(ThreadPriority & ~ThreadPriorityMask)&&
			!(TaskPriority & TaskPriorityMask) != ThreadPriorityMask &&
			!(TaskPriority & ~TaskPriorityMask)
		);
		return Type(ThreadAndIndex | ThreadPriority | TaskPriority);
	}

	__forceinline Type SetPriorities(Type ThreadAndIndex, int32 PrirorityIndex, bool bHiPri)
	{
		assert(
			!(ThreadAndIndex & ~ThreadIndexMask) &&
			PrirorityIndex >= 0 && PrirorityIndex < NumThreadPriorities
		);
		return Type((ThreadAndIndex | PrirorityIndex << ThreadPriorityShift) | (bHiPri ? HighTaskPriority : NormalTaskPriority));
	}

	__forceinline Type SetThreadPriority(Type ThreadAndIndex, Type ThreadPriority)
	{
		assert(
			!(ThreadAndIndex & ~ThreadIndexMask) &&
			!(ThreadPriority & ~ThreadPriorityMask) &&
			(ThreadPriority & ThreadPriorityMask) != ThreadPriorityMask
		);

		return Type(ThreadAndIndex | ThreadPriority);
	}

	__forceinline Type SetTaskPriority(Type ThreadAndIndex, Type TaskPriority)
	{
		assert(
			!(ThreadAndIndex & ~ThreadIndexMask) &&
			!(TaskPriority & ~TaskPriorityMask)
		);

		return Type(ThreadAndIndex | TaskPriority);
	}
}

namespace ESubsequentsMode
{
	enum Type
	{
		/** Necessary when another task will depend on this task. */
		TrackSubsequents,
		/** Can be used to save task graph overhead when firing off a task that will not be a dependency of other tasks. */
		FireAndForget
	};
}

using FGraphEventRef = std::shared_ptr<class FGraphEvent>;
using FGraphEventArray = std::vector<FGraphEventRef>;

// Interface to the task graph system.
class FTaskGraphInterface
{
	friend class FBaseGraphTask;
	virtual void QueueTask(class FBaseGraphTask* Task, ENamedThreads::Type ThreadToExecuteOn, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread) = 0;

public:
	virtual ~FTaskGraphInterface() {}

	static void						Startup(int32 NumThreads); // 这里传的值其实没有用到
	static void						Shutdown();
	static bool						IsRunning();
	static FTaskGraphInterface&		Get();
	virtual ENamedThreads::Type		GetCurrentThreadIfKnown(bool bLocalQueue = false) = 0;
	virtual int32					GetNumWorkerThreads() = 0; // Return the number of worker (non-named) threads PER PRIORITY SET.
	virtual bool					IsThreadProcessingTasks(ENamedThreads::Type ThreadToCheck) = 0;

	// External Thread API.
	virtual void					AttachToThread(ENamedThreads::Type CurrentThread) = 0; // A one time call that "introduces" an external thread to the system. Basically, it just sets up the TLS info 
	virtual void					ProcessThreadUntilIdle(ENamedThreads::Type CurrentThread) = 0; // Requests that a named thread, which must be this thread, run until idle, then return.
	virtual void					ProcessThreadUntilRequestReturn(ENamedThreads::Type CurrentThread) = 0; //Requests that a named thread, which must be this thread, run until an explicit return request is received, then return.
	virtual void					RequestReturn(ENamedThreads::Type CurrentThread) = 0; // Request that the given thread stop when it is idle
	virtual void					WaitUtilTasksComplete(const FGraphEventArray& Tasks, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread) = 0; //Requests that a named thread, which must be this thread, run until a list of tasks is complete.
	virtual void					TriggerEventWhenTasksComplete(class FEvent* InEvent, const FGraphEventArray& Tasks, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread) = 0; //When a set of tasks complete, fire a scoped event
	
	void WaitUntilTaskCompletes(const FGraphEventRef& Task, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread)
	{
		FGraphEventArray Prerequistes;
		Prerequistes.push_back(Task);
		WaitUtilTasksComplete(Prerequistes, CurrentThreadIfKnown);
	}

	void TriggerEventWhenTaskCompletes(FEvent* InEvent, const FGraphEventRef& Task, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread)
	{
		FGraphEventArray Prerequistes;
		Prerequistes.push_back(Task);
		TriggerEventWhenTasksComplete(InEvent, Prerequistes, CurrentThreadIfKnown);
	}

	virtual void AddShutdownCallback(std::function<void()>& Callback) = 0;
};


/************************************************************************/
/* Base Class for all tasks.
*/
/************************************************************************/
class FBaseGraphTask
{
protected:
	FBaseGraphTask(int32 InNumberOfPrerequistitesOutstanding)
		: ThreadToExecuteOn(ENamedThreads::AnyThread)
		, NumberOfPrerequistitiesOutstanding(InNumberOfPrerequistitesOutstanding + 1){}

	void SetThreadToExecuteOn(ENamedThreads::Type InThreadToExecuteOn)
	{
		ThreadToExecuteOn = InThreadToExecuteOn;
	}

	void PrerequisitesComplete(ENamedThreads::Type CurrentThread, int32 NumAlreadyFinishedPrequistes, bool bUnlock = true)
	{
		int32 NumToSub = NumAlreadyFinishedPrequistes + (bUnlock ? 1 : 0);
		if (NumberOfPrerequistitiesOutstanding.Subtract(NumToSub) == NumToSub)
		{
			QueueTask(CurrentThread);
		}
	}

	virtual ~FBaseGraphTask() {}

	void ConditionalQueueTask(ENamedThreads::Type CurrentThread)
	{
		if (NumberOfPrerequistitiesOutstanding.Decrement() == 0)
		{
			QueueTask(CurrentThread);
		}
	}

private:
	friend class FNamedTaskThread;
	friend class FTaskThreadBase;
	friend class FTaskThreadAnyThread;
	friend class FGraphEvent;
	friend class FTaskGraphImplementation;

	virtual void ExecuteTask(std::vector<FBaseGraphTask*>& NewTasks, ENamedThreads::Type CurrentThread) = 0;

	__forceinline void Execute(std::vector<FBaseGraphTask*>& NewTasks, ENamedThreads::Type CurrentThread)
	{
		ExecuteTask(NewTasks, CurrentThread);
	}

	void QueueTask(ENamedThreads::Type CurrentThreadIfKnown)
	{
		FTaskGraphInterface::Get().QueueTask(this, ThreadToExecuteOn, CurrentThreadIfKnown);
	}

	ENamedThreads::Type			ThreadToExecuteOn;
	FThreadSafeCounter			NumberOfPrerequistitiesOutstanding;
};

// FGraphEvent是一堆子tasks等待的一个事件
// TODO: 2018/7/31 添加小存储Task的统一内存分配。
class FGraphEvent
{
public:
	static FGraphEventRef	CreateGraphEvent();
	static FGraphEvent*		CreateGraphEnventWithInlineStorage();

	// Attempts to a new subsequent task. If this event has already fired, false is returned and action must be taken to ensure that 
	// the task will still fire even though this event cannot be a prerequisite (because it is already finished).
	bool AddSubsequent(class FBaseGraphTask* Task)
	{
		return SubsequentList.PushIfNotClosed(Task);
	}

	// Verification function to ensure that nobody was tried to add WaitUntil's outside of the context of execution
	void CheckDontCompleteUntilIsEmpty()
	{
		bool isEmpty = !EventsToWaitFor.size();
		assert(isEmpty);
	}

	// Delay the firing of this event until the given event fires.
	void DontCompleteUntil(FGraphEventRef EventToWaitFor)
	{
		assert(!IsComplete());
		EventsToWaitFor.push_back(EventToWaitFor);
	}

	// Sets the thread that you want to execute the null gather task on. This is useful if the thing waiting for this chain to complete is a single, named thread.
	// 应该是在null task时，作为一个收集等待的task。在named thread中需要等待这个task执行完。会放在named thread中。
	void SetGatherThreadForDontCompleteUntil(ENamedThreads::Type InThreadToDoGatherOn)
	{
		assert(!IsComplete());
		ThreadToDoGatherOn = InThreadToDoGatherOn;
	}

	/**
	*	"Complete" the event. This grabs the list of subsequents and atomically closes it. Then for each subsequent it reduces the number of prerequisites outstanding and if that drops to zero, the task is queued.
	*	@param CurrentThreadIfKnown if the current thread is known, provide it here. Otherwise it will be determined via TLS if any task ends up being queued.
	**/
	void DispatchSubsequents(std::vector<FBaseGraphTask*>& NewTasks, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread);

	bool IsComplete() const
	{
		return SubsequentList.IsClosed();
	}

private:
	friend class std::shared_ptr<FGraphEvent>;

	// Internal function to call the destructor and recycle a graph event
	static void Recycle(FGraphEvent* ToRecycle);

	friend struct FGraphEventAndSmallTaskStorage;
	FGraphEvent(bool bInInline = false)
		: ThreadToDoGatherOn(ENamedThreads::AnyHiPriThreadHiPriTask){}

	~FGraphEvent();

public:
	uint32 AddRef()
	{
		int32 RefCount = ReferenceCount.Increment();
		assert(RefCount > 0);
		return RefCount;
	}

	uint32 Release()
	{
		int32 RefCount = ReferenceCount.Decrement();
		assert(RefCount >= 0);
		if (RefCount == 0)
		{
			Recycle(this);
		}
		return RefCount;
	}
private:
	/** Threadsafe list of subsequents for the event **/
	TClosableLockFreePointerListUnorderedSingleConsumer<FBaseGraphTask, 0>	SubsequentList;
	/** List of events to wait for until firing. This is not thread safe as it is only legal to fill it in within the context of an executing task. **/
	FGraphEventArray														EventsToWaitFor;
	/** Number of outstanding references to this graph event **/
	FThreadSafeCounter														ReferenceCount;
	ENamedThreads::Type														ThreadToDoGatherOn;
};

/**
*	TGraphTask
*	Embeds a user defined task, as exemplified above, for doing the work and provides the functionality for setting up and handling prerequisites and subsequents
**/
// 包裹用户创建的TTask的模板类。继承自FBaseGraphTask基类，引擎中可以统一使用接口管理。
template<typename TTask>
class TGraphTask : public FBaseGraphTask
{
public:
	// TTask的构造器.(使用一个FConstructor是为使用TGraphTask中的storage吗？？？)
	// 使用构造器就是为了这个变长模板函数。可以实现创建不同参数的TTask。如果没有FConstructor，则需要在TGraspTask中添加
	// 静态成员函数。静态成员函数不知是不是可以变长模板函数，应该是可以。但是类模板加上成员函数模板，参数不是一套，有点乱。
	class FConstructor
	{
	public:
		template<typename...T>
		FGraphEventRef ConstructAndDispatchWhenReady(T&&... Args)
		{
			new ((void*)&Owner->TaskStorage) TTask(std::forward<T>(Args)...);
			Owner->Setup(Prerequisites, CurrentThreadIfKnown);
		}

		template<typename... T>
		TGraphTask* ConstructAndHold(T&&... Args)
		{
			new ((void *)&Owner->TaskStorage) TTask(std::forward()<T>(Args)...);
			return Owner->Hold(Prerequisites, CurrentThreadIfKnown);
		}
	private:
		friend class		TGraphTask;
		TGraphTask*			Owner;

		const FGraphEventArray*			Prerequisites;
		ENamedThreads::Type				CurrentThreadIfKnown;

		FConstructor(TGraphTask* InOwner, const FGraphEventArray* InPreQuisties, ENamedThreads::Type InCurrentThreadIfKnown)
			: Owner(InOwner)
			, Prerequisites(InPreQuisties)
			, CurrentThreadIfKnown(InCurrentThreadIfKnown)
		{}
		FConstructor(const FConstructor& Other) = delete;
		void operator=(const FConstructor& Other) = delete;
	};

	static FConstructor CreateTask(const FGraphEventArray* Prerequisties = nullptr, ENamedThreads::Type CurrentThreadIfKnown = ENamedThreads::AnyThread)
	{
		int NumPrereq = Prerequisties ? Prerequisties->size() : 0;
		// TODO: 当task的尺寸比较小时，可以使用统一内存分配，防止存储锁片。
		return FConstructor(new TGraphTask(TTask::GetSubsequentsMode() == ESubsequentsMode::FireAndForget ? NULL : FGraphEvent::CreateGraphEvent(), NumPrereq), Prerequisties, CurrentThreadIfKnown);
	}

private:
	friend class FConstructor;
	friend class FGraphEvent;


	TGraphTask(FGraphEventRef InSubsequents, int32 NumberOfPrerequistiesOutstanding)
		:FBaseGraphTask(NumberOfPrerequistiesOutstanding)
		,TaskConstructed(false)
	{
		Subsequents.swap(InSubsequents);
	}

	virtual ~TGraphTask() final override
	{
		assert(!TaskConstructed);
	}

	/** An aligned bit of storage to hold the embedded task **/
	TAlignedBytes<sizeof(TTask), alignof(TTask)> TaskStorage;
	/** Used to sanity check the state of the object **/
	bool						TaskConstructed;
	/** A reference counted pointer to the completion event which lists the tasks that have me as a prerequisite. **/
	FGraphEventRef				Subsequents;
};