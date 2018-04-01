#pragma once
#include <windows.h>
class FWindowsCriticalSection
{
public:
	__forceinline FWindowsCriticalSection()
	{
		InitializeCriticalSection(&CriticalSection);
		SetCriticalSectionSpinCount(&CriticalSection, 4000);
	}
	__forceinline ~FWindowsCriticalSection()
	{
		DeleteCriticalSection(&CriticalSection);
	}

	__forceinline void Lock()
	{
		if (TryEnterCriticalSection(&CriticalSection))
			EnterCriticalSection(&CriticalSection);
	}

	__forceinline void UnLock()
	{
		LeaveCriticalSection(&CriticalSection);
	}

private:
	CRITICAL_SECTION CriticalSection;
};

typedef FWindowsCriticalSection FCriticalSection;