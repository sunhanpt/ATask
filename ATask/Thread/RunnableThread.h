#pragma once
#include "../HAL/HAL.h"
#include "ThreadUtility.h"
#include <string>
#include <vector>
/**
* Interface for runnable threads.
*
* This interface specifies the methods used to manage a thread's life cycle.
*/
class FRunnableThread
{
	friend class FThreadManager;

	/** Index of TLS slot for FRunnableThread pointer. */
	static unsigned int RunnableTlsSlot;

public:
	/** Gets a new Tls slot for storing the runnable thread pointer. */
	static unsigned int GetTlsSlot();

	/**
	* Factory method to create a thread with the specified stack size and thread priority.
	*
	* @param InRunnable The runnable object to execute
	* @param ThreadName Name of the thread
	* @param bAutoDeleteSelf Whether to delete this object on exit
	* @param bAutoDeleteRunnable Whether to delete the runnable object on exit
	* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
	* @param InThreadPri Tells the thread whether it needs to adjust its priority or not. Defaults to normal priority
	* @return The newly created thread or nullptr if it failed
	*/
		static FRunnableThread* Create(
			class FRunnable* InRunnable,
			const TCHAR* ThreadName,
			bool bAutoDeleteSelf,
			bool bAutoDeleteRunnable = false,
			uint32 InStackSize = 0,
			EThreadPriority InThreadPri = TPri_Normal,
			uint64 InThreadAffinityMask = 0);

	/**
	* Factory method to create a thread with the specified stack size and thread priority.
	*
	* @param InRunnable The runnable object to execute
	* @param ThreadName Name of the thread
	* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
	* @param InThreadPri Tells the thread whether it needs to adjust its priority or not. Defaults to normal priority
	* @return The newly created thread or nullptr if it failed
	*/
	static FRunnableThread* Create(
		class FRunnable* InRunnable,
		const TCHAR* ThreadName,
		uint32 InStackSize = 0,
		EThreadPriority InThreadPri = TPri_Normal,
		uint64 InThreadAffinityMask = FPlatformAffinity::GetNoAffinityMask());

	/**
	* Changes the thread priority of the currently running thread
	*
	* @param NewPriority The thread priority to change to
	*/
	virtual void SetThreadPriority(EThreadPriority NewPriority) = 0;

	/**
	* Tells the thread to either pause execution or resume depending on the
	* passed in value.
	*
	* @param bShouldPause Whether to pause the thread (true) or resume (false)
	*/
	virtual void Suspend(bool bShouldPause = true) = 0;

	/**
	* Tells the thread to exit. If the caller needs to know when the thread has exited, it should use the bShouldWait value.
	* It's highly recommended not to kill the thread without waiting for it.
	* Having a thread forcibly destroyed can cause leaks and deadlocks.
	*
	* The kill method is calling Stop() on the runnable to kill the thread gracefully.
	*
	* @param bShouldWait	If true, the call will wait infinitely for the thread to exit.
	* @return Always true
	*/
	virtual bool Kill(bool bShouldWait = true) = 0;

	/** Halts the caller until this thread is has completed its work. */
	virtual void WaitForCompletion() = 0;

	/**
	* Thread ID for this thread
	*
	* @return ID that was set by CreateThread
	* @see GetThreadName
	*/
	const uint32 GetThreadID() const
	{
		return ThreadID;
	}

	/**
	* Retrieves the given name of the thread
	*
	* @return Name that was set by CreateThread
	* @see GetThreadID
	*/
	const std::string& GetThreadName() const
	{
		return ThreadName;
	}

	/** Default constructor. */
	FRunnableThread();

	/** Virtual destructor */
	virtual ~FRunnableThread();

protected:
	/**
	* Creates the thread with the specified stack size and thread priority.
	*
	* @param InRunnable The runnable object to execute
	* @param ThreadName Name of the thread
	* @param InStackSize The size of the stack to create. 0 means use the current thread's stack size
	* @param InThreadPri Tells the thread whether it needs to adjust its priority or not. Defaults to normal priority
	* @return True if the thread and all of its initialization was successful, false otherwise
	*/
	virtual bool CreateInternal(FRunnable* InRunnable, const TCHAR* InThreadName,
		uint32 InStackSize = 0,
		EThreadPriority InThreadPri = TPri_Normal, uint64 InThreadAffinityMask = 0) = 0;
	
	/** Stores this instance in the runnable thread TLS slot. */
	void SetTls();

	/** Deletes all FTlsAutoCleanup objects created for this thread. */
	void FreeTls();

	/**
	* @return a runnable thread that is executing this runnable, if return value is nullptr, it means the running thread can be game thread or a thread created outside the runnable interface
	*/
	static FRunnableThread* GetRunnableThread()
	{
		FRunnableThread* RunnableThread = (FRunnableThread*)FPlatformTLS::GetTlsValue(RunnableTlsSlot);
		return RunnableThread;
	}

	/** Holds the name of the thread. */
	std::string				ThreadName;

	/** The runnable object to execute on this thread. */
	class FRunnable*		Runnable;

	class FEvent*			ThreadInitSyncEvent;

	/** The Affinity to run the thread with. */
	uint64 ThreadAffinityMask;

	/** An array of FTlsAutoCleanup based instances that needs to be deleted before the thread will die. */
	std::vector<class FTlsAutoCleanup*> TlsInstances;

	/** The priority to run the thread at. */
	EThreadPriority ThreadPriority;

	/** ID set during thread creation. */
	uint32 ThreadID;

private:
	/** Used by the thread manager to tick threads in single-threaded mode */
	virtual void Tick() {}
};