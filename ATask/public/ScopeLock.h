/************************************************************************/
/* Scope Lock
*  pengtong.pt 2018-5-21
*/
/************************************************************************/
#include "CriticalSection.h"

class FScopeLock
{
public:
	FScopeLock(FCriticalSection* InSynchObject):
		SynchObject(InSynchObject)
	{
		SynchObject->Lock();
	}
	~FScopeLock()
	{
		SynchObject->UnLock();
	}
private:
	// hidden these methods.
	FScopeLock(); 
	FScopeLock(const FScopeLock& InScopeLock);
	FScopeLock& operator= (const FScopeLock& InScopeLock)
	{
		return *this;
	}
private:
	FCriticalSection* SynchObject;
};