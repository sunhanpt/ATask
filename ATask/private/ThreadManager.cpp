#include "ThreadManager.h"
#include "ScopeLock.h"
#include <assert.h>

void FThreadManager::AddThread(int ThreadId, FRunnableThread* Thread)
{
	FScopeLock ScopeLock(&ThreadsCritical);
	auto& it = Threads.find(ThreadId);
	if (it == Threads.end())
	{
		Threads[ThreadId] = Thread;
	}
}

void FThreadManager::RemoveThread(FRunnableThread* Thread)
{
	assert(!"This method is time-consuming£¡");
}

void FThreadManager::RemoveThread(int ThreadId)
{
	FScopeLock ScopeLock(&ThreadsCritical);
	Threads.erase(ThreadId);
}

const std::string& FThreadManager::GetThreadName(int ThreadId)
{
	static std::string EmpthString("");
	return EmpthString;
}

FThreadManager& FThreadManager::Get()
{
	static FThreadManager Singleton;
	return Singleton;
}