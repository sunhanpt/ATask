#pragma once
/************************************************************************/
/* Thread Manager
*  pengtong.pt 2018-5-21
*/
/************************************************************************/
#include <unordered_map>
#include "CriticalSection.h"

class FRunnableThread;
class FThreadManager
{
	std::unordered_map<int, FRunnableThread*> Threads;
	FCriticalSection ThreadsCritical;

public:
	void AddThread(int ThreadId, FRunnableThread* Thread);
	void RemoveThread(FRunnableThread* Thread);
	void RemoveThread(int ThreadId);

	const std::string GetThreadName(int ThreadId);

	static FThreadManager& Get();
};