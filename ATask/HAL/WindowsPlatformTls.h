#pragma once
#include "WindowsCoreType.h"
/**
* Windows implementation of the TLS OS functions.
*/
struct FWindowsPlatformTLS
{
	/**
	* Returns the currently executing thread's identifier.
	*
	* @return The thread identifier.
	*/
	static __forceinline uint32 GetCurrentThreadId(void)
	{
		return GetCurrentThreadId();
	}

	/**
	* Allocates a thread local store slot.
	*
	* @return The index of the allocated slot.
	*/
	static __forceinline uint32 AllocTlsSlot(void)
	{
		return TlsAlloc();
	}

	/**
	* Sets a value in the specified TLS slot.
	*
	* @param SlotIndex the TLS index to store it in.
	* @param Value the value to store in the slot.
	*/
	static __forceinline void SetTlsValue(uint32 SlotIndex, void* Value)
	{
		TlsSetValue(SlotIndex, Value);
	}

	/**
	* Reads the value stored at the specified TLS slot.
	*
	* @param SlotIndex The index of the slot to read.
	* @return The value stored in the slot.
	*/
	static __forceinline void* GetTlsValue(uint32 SlotIndex)
	{
		return TlsGetValue(SlotIndex);
	}

	/**
	* Frees a previously allocated TLS slot
	*
	* @param SlotIndex the TLS index to store it in
	*/
	static __forceinline void FreeTlsSlot(uint32 SlotIndex)
	{
		TlsFree(SlotIndex);
	}
};


typedef FWindowsPlatformTLS FPlatformTLS;