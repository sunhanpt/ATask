#include "WindowsPlatformProcess.h"

bool FWindowsPlatformProcess::SupportsMultithreading()
{
	return true; // determined by cmd input parameters.
}
