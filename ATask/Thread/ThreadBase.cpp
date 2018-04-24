/************************************************************************/
/*	Implement RunnableThread.h & ThreadManager.h
*/
/************************************************************************/
#include "RunnableThread.h"
#include "ThreadManager.h"
#include "FScopeLock.h"

////////////////////////////////////*Thread Manager*//////////////////////////////////////

void FThreadManager::AddThread(uint32 ThreadId, class FRunnableThread* Thread)
{
	FScopeLock ThreadsLock(&ThreadsCritical);
	// Some platforms do not support TLS
	if (Threads.end() != Threads.find(ThreadId))
	{
		Threads.insert(std::make_pair(ThreadId, Thread));
	}
}

void FThreadManager::RemoveThread(FRunnableThread* Thread)
{
	FScopeLock ThreadsLock(&ThreadsCritical);
	for (auto it = Threads.begin(); it != Threads.end(); it++)
	{
		if (it->second == Thread)
		{
			Threads.erase(it);
			break;
		}
	}
}

void FThreadManager::Tick()
{
	if (!FPlatformProcess::SupportsMultithreading())
	{
		FScopeLock ThreadsLock(&ThreadsCritical);

		// Tick all registered threads.
		for (auto& ThreadPair : Threads)
		{
			ThreadPair.second->Tick();
		}
	}
}

const std::string& FThreadManager::GetThreadName(uint32 ThreadId)
{
	static std::string NoThreadName;
	FScopeLock ThreadsLock(&ThreadsCritical);
	auto it = Threads.find(ThreadId);
	if (it != Threads.end())
	{
		return it->second->GetThreadName();
	}
	return NoThreadName;
}

FThreadManager& FThreadManager::Get()
{
	static FThreadManager Singleton;
	return Singleton;
}


//////////////////////////////////////////////////////////////////////////