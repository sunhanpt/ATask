#pragma once;

#if defined(_WIN64)
	#define PLATFORM_64BITS 1
#else 
	#define PLATFORM_64BITS 0
#endif
// Intrinsics for 128-bit atomics on Windows platform requires Windows 8 or higher (WINVER>=0x0602)
// http://msdn.microsoft.com/en-us/library/windows/desktop/hh972640.aspx
#define PLATFORM_HAS_128BIT_ATOMICS							( PLATFORM_64BITS && (WINVER >= 0x602))

// Prefetch
#define PLATFORM_CACHE_LINE_SIZE	128

#define MS_ALIGN(n) __declspec(align(n))

#define  FORCEINLINE __forceinline
