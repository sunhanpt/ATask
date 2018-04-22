#pragma once

struct FWindowsPlatformProcess
{
	static bool SupportsMultithreading();
};

typedef FWindowsPlatformProcess FPlatformProcess;