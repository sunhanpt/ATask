#pragma once

template <typename T>
__forceinline constexpr bool IsAligned(T Val, uint64 Alignment)
{
	return !((uint64)Val & (Alignment - 1));
}