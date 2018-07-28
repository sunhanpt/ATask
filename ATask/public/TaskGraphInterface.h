#pragma once
#include "ATypes.h"
#include <cassert>
#include <memory>
#include <vector>
#include <functional>

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