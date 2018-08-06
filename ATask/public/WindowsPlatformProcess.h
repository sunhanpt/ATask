#pragma once
class FEvent;
class FWindowsPlatformProcess
{
public:
	static bool SupportsMultithreading();

	static FEvent* GetSynchEventFromPool(bool bIsManualReset);

	static void ReturnSynchEventToPool(FEvent* Event);

	static FEvent* CreateSynchEvent(bool bIsManualReset = false);
};

using FPlatformProcess = FWindowsPlatformProcess;