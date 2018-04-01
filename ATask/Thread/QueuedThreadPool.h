#pragma once
#include <stdint.h>
#include "ThreadUtility.h"

class IQueuedWork;

/**
* Interface for queued thread pools.
*
* This interface is used by all queued thread pools. It used as a callback by
* FQueuedThreads and is used to queue asynchronous work for callers.
*/
class FQueuedThreadPool
{
public:
	FQueuedThreadPool() {}
	virtual ~FQueuedThreadPool() {}

public:
	virtual bool			Create(uint32_t InNumQueuedThreads, uint32_t StackSize = (32 * 1024), EThreadPriority ThreadPriority = TPri_Normal) = 0;
	virtual void			Destory() = 0;
	virtual void			QueuedThreadWork(IQueuedWork* InQueuedWork) = 0;
	virtual bool			RetractQueuedWork(IQueuedWork* InQueuedWork) = 0;
	virtual IQueuedWork*	ReturnToPoolOrGetNextJob(class FQueuedThread* InQueuedThread) = 0;
	virtual int32_t			GetNumThreads() const = 0;


	static FQueuedThreadPool* Allocate();

	/**
	*	Stack size for threads created for the thread pool.
	*	Can be overridden by other projects.
	*	If 0 means to use the value passed in the Create method.
	*/
	static uint32_t OverrideStackSize;
};

extern FQueuedThreadPool* GThreadPool;

