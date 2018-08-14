#pragma once

#include "CoreTypes.h"

struct FMemory
{
	/**
	* Set up TLS caches on the current thread. These are the threads that we can trim.
	*/
	static void SetupTLSCachesOnCurrentThread();
};