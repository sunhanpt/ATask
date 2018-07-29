#pragma once

class FWindowsPlatformProcess
{
public:
	static bool SupportsMultithreading();
};

using FPlatformProcess = FWindowsPlatformProcess;