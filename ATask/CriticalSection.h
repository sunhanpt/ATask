#pragma once
/************************************************************************/
/* Critical Seciton
*  pengtong.pt 2018-5-21
*/
/************************************************************************/
#include <windows.h>

class FCriticalSection
{
public:
	__forceinline FCriticalSection()
	{
		InitializeCriticalSectionAndSpinCount(&CriticalSeciton, 4000);
	}

	__forceinline ~FCriticalSection()
	{
		DeleteCriticalSection(&CriticalSeciton);
	}

	__forceinline void Lock()
	{
		if (TryEnterCriticalSection(&CriticalSeciton) == 0)
		{
			EnterCriticalSection(&CriticalSeciton);
		}
	}

	__forceinline void UnLock()
	{
		LeaveCriticalSection(&CriticalSeciton);
	}
private:
	CRITICAL_SECTION CriticalSeciton;
};