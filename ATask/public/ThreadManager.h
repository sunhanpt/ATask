#pragma once
/************************************************************************/
/* Thread Manager
*  pengtong.pt 2018-5-21
*/
/************************************************************************/
#include <map>
#include "CriticalSection.h"

class FRunnableThread;
class FThreadManager
{
	std::map<int, FRunnableThread*> Threads;
	FCriticalSection ThreadsCritical;

public:
	void AddThread(int ThreadId, FRunnableThread* Thread);
	void RemoveThread(FRunnableThread* Thread);
	void RemoveThread(int ThreadId);

	const std::string& GetThreadName(int ThreadId);

	static FThreadManager& Get();
};