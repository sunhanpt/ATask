#pragma once
#include "CoreTypes.h"
#include "WindowsPlatformTLS.h"

extern bool			GISGameThreadIdInitialized;

extern int32		GGameThreadId;
extern int32		GRenderThreadId;

__forceinline bool IsInGameThread()
{
	if (GISGameThreadIdInitialized)
	{
		const int32 CurrentThreadId = FPlatformTLS::GetCurrentThreadIdUE();
		return CurrentThreadId == GGameThreadId;
	}
	return false;
}