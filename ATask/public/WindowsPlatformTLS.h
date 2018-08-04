#pragma once
#include "CoreTypes.h"
#include <processthreadsapi.h>

/**
* Windows implementation of the TLS OS functions.
*/

struct FWindowsPlatformTLS
{
	static __forceinline bool IsValidTlsSlotUE(uint32 SlotIndex)
	{
		return SlotIndex != 0xFFFFFFFF;
	}

	static __forceinline uint32 GetCurrentThreadIdUE(void)
	{
		return GetCurrentThreadId();
	}

	static __forceinline uint32 AllocTlsSlotUE()
	{
		return TlsAlloc();
	}

	static __forceinline void SetTlsValueUE(uint32 SlotIndex, void* Value)
	{
		TlsSetValue(SlotIndex, Value);
	}

	static __forceinline void* GetTlsValueUE(uint32 SlotIndex)
	{
		return TlsGetValue(SlotIndex);
	}

	static __forceinline void FreeTlsSlot(uint32 SlotIndex)
	{
		TlsFree(SlotIndex);
	}
};

using FPlatformTLS = FWindowsPlatformTLS;

