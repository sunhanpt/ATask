#pragma once
#include <cassert>
#include "../HAL/HAL.h" 
class FScopeLock
{
public:
	FScopeLock(FCriticalSection* InSyncObject): SyncObject(InSyncObject)
	{
		assert(SyncObject);
		SyncObject->Lock();
	}

	~FScopeLock()
	{
		assert(SyncObject);
		SyncObject->UnLock();
	}

private:
	FCriticalSection* SyncObject;

	// ���ó�˽�У����ص���Щ����
	FScopeLock();
	FScopeLock(const FScopeLock& InScopeLock);
	FScopeLock& operator=(const FScopeLock& InScopeLock)
	{
		return *this;
	}
};