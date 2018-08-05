#pragma once
#include "CoreTypes.h"
/**
* Interface for "runnable" objects.
*
* A runnable object is an object that is "run" on an arbitrary thread. The call usage pattern is
* Init(), Run(), Exit(). The thread that is going to "run" this object always uses those calling
* semantics. It does this on the thread that is created so that any thread specific uses (TLS, etc.)
* are available in the contexts of those calls. A "runnable" does all initialization in Init().
*
* If initialization fails, the thread stops execution and returns an error code. If it succeeds,
* Run() is called where the real threaded work is done. Upon completion, Exit() is called to allow
* correct clean up.
*/

class FRunnable
{
public:
	virtual ~FRunnable() {}

	virtual bool Init()
	{
		return true;
	}

	virtual uint32 Run() = 0;

	virtual void Stop() {}

	virtual void Exit() {}

	// 单线程时使用
	virtual class FSingleThreadRunnable* GetSingleThreadInterface()
	{
		return nullptr;
	}
};