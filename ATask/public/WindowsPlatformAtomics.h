#pragma once
#include "CoreTypes.h"
#include "WindowsPlatform.h"
#include <intrin.h>


struct FWindowsPlatformAtomics
{
	static_assert(sizeof(int8) == sizeof(char) && alignof(int8) == alignof(char), "int8 must be compatible with char");
	static_assert(sizeof(int16) == sizeof(short) && alignof(int16) == alignof(short), "int16 must be compatible with short");
	static_assert(sizeof(int32) == sizeof(long) && alignof(int32) == alignof(long), "int32 must be compatible with long");
	static_assert(sizeof(int64) == sizeof(long long) && alignof(int64) == alignof(long long), "int64 must be compatible with long long");

	static __forceinline int8 InterlockedIncrement(volatile int8* Value)
	{
		return (int8)_InterlockedExchangeAdd8((char*)Value, 1) + 1;
	}

	static __forceinline int16 InterlockedIncrement(volatile int16* Value)
	{
		return (int16)_InterlockedIncrement16((short*)Value);
	}

	static __forceinline int32 InterlockedIncrement(volatile int32* Value)
	{
		return (int32)_InterlockedIncrement((long*)Value);
	}

	static __forceinline int64 InterlockedIncrement(volatile int64* Value)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedIncrement64((long long*)Value);
#else
		// No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue + 1, OldValue) == OldValue)
			{
				return OldValue + 1;
			}
		}
#endif
	}

	static __forceinline int8 InterlockedDecrement(volatile int8* Value)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, -1) - 1;
	}

	static __forceinline int16 InterlockedDecrement(volatile int16* Value)
	{
		return (int16)::_InterlockedDecrement16((short*)Value);
	}

	static __forceinline int32 InterlockedDecrement(volatile int32* Value)
	{
		return (int32)::_InterlockedDecrement((long*)Value);
	}

	static __forceinline int64 InterlockedDecrement(volatile int64* Value)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedDecrement64((long long*)Value);
#else
		// No explicit instruction for 64-bit atomic decrement on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue - 1, OldValue) == OldValue)
			{
				return OldValue - 1;
			}
		}
#endif
	}

	static __forceinline int8 InterlockedAdd(volatile int8* Value, int8 Amount)
	{
		return (int8)::_InterlockedExchangeAdd8((char*)Value, (char)Amount);
	}

	static __forceinline int16 InterlockedAdd(volatile int16* Value, int16 Amount)
	{
		return (int16)::_InterlockedExchangeAdd16((short*)Value, (short)Amount);
	}

	static __forceinline int32 InterlockedAdd(volatile int32* Value, int32 Amount)
	{
		return (int32)::_InterlockedExchangeAdd((long*)Value, (long)Amount);
	}

	static __forceinline int64 InterlockedAdd(volatile int64* Value, int64 Amount)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedExchangeAdd64((int64*)Value, (int64)Amount);
#else
		// No explicit instruction for 64-bit atomic add on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, OldValue + Amount, OldValue) == OldValue)
			{
				return OldValue + Amount;
			}
		}
#endif
	}

	static __forceinline int8 InterlockedExchange(volatile int8* Value, int8 Exchange)
	{
		return (int8)::_InterlockedExchange8((char*)Value, (char)Exchange);
	}

	static __forceinline int16 InterlockedExchange(volatile int16* Value, int16 Exchange)
	{
		return (int16)::_InterlockedExchange16((short*)Value, (short)Exchange);
	}

	static __forceinline int32 InterlockedExchange(volatile int32* Value, int32 Exchange)
	{
		return (int32)::_InterlockedExchange((long*)Value, (long)Exchange);
	}

	static __forceinline int64 InterlockedExchange(volatile int64* Value, int64 Exchange)
	{
#if PLATFORM_64BITS
		return (int64)::_InterlockedExchange64((long long*)Value, (long long)Exchange);
#else
		// No explicit instruction for 64-bit atomic exchange on 32-bit processors; has to be implemented in terms of CMPXCHG8B
		for (;;)
		{
			int64 OldValue = *Value;
			if (_InterlockedCompareExchange64(Value, Exchange, OldValue) == OldValue)
			{
				return OldValue;
			}
		}
#endif
	}

	static __forceinline void* InterlockedExchangePtr(void** Dest, void* Exchange)
	{
#if PLATFORM_64BITS
		return (void*)::_InterlockedExchange64((int64*)(Dest), (int64)(Exchange));
#else
		return (void*)::_InterlockedExchange((long*)(Dest), (long)(Exchange));
#endif
	}

	static __forceinline int8 InterlockedCompareExchange(volatile int8* Dest, int8 Exchange, int8 Comparand)
	{
		return (int8)::_InterlockedCompareExchange8((char*)Dest, (char)Exchange, (char)Comparand);
	}

	static __forceinline int16 InterlockedCompareExchange(volatile int16* Dest, int16 Exchange, int16 Comparand)
	{
		return (int16)::_InterlockedCompareExchange16((short*)Dest, (short)Exchange, (short)Comparand);
	}

	static __forceinline int32 InterlockedCompareExchange(volatile int32* Dest, int32 Exchange, int32 Comparand)
	{
		return (int32)::_InterlockedCompareExchange((long*)Dest, (long)Exchange, (long)Comparand);
	}

	static __forceinline int64 InterlockedCompareExchange(volatile int64* Dest, int64 Exchange, int64 Comparand)
	{
		return (int64)::_InterlockedCompareExchange64(Dest, Exchange, Comparand);
	}

	static __forceinline int8 AtomicRead(volatile const int8* Src)
	{
		return InterlockedCompareExchange((int8*)Src, 0, 0);
	}

	static __forceinline int16 AtomicRead(volatile const int16* Src)
	{
		return InterlockedCompareExchange((int16*)Src, 0, 0);
	}

	static __forceinline int32 AtomicRead(volatile const int32* Src)
	{
		return InterlockedCompareExchange((int32*)Src, 0, 0);
	}

	static __forceinline int64 AtomicRead(volatile const int64* Src)
	{
		return InterlockedCompareExchange((int64*)Src, 0, 0);
	}

	static __forceinline void AtomicStore(volatile int8* Src, int8 Val)
	{
		InterlockedExchange(Src, Val);
	}

	static __forceinline void AtomicStore(volatile int16* Src, int16 Val)
	{
		InterlockedExchange(Src, Val);
	}

	static __forceinline void AtomicStore(volatile int32* Src, int32 Val)
	{
		InterlockedExchange(Src, Val);
	}

	static __forceinline void AtomicStore(volatile int64* Src, int64 Val)
	{
		InterlockedExchange(Src, Val);
	}
};

using FPlatformAtomics = FWindowsPlatformAtomics;