#pragma once
#define PLATFORM_WINDOWS 1
#ifdef PLATFORM_WINDOWS
#include "WindowsCoreType.h"
#include "WindowsCriticalSection.h"
#include "WindowsPlatformTls.h"
#include "WindowsPlatformProcess.h"
#endif // __Windows__
