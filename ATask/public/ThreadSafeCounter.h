#pragma once

#include "CoreTypes.h"
#include "WindowsPlatformAtomics.h"

class FThreadSafeCounter
{
public:
	using IntegerType = int32;

	FThreadSafeCounter()
	{
		Counter = 0;
	}

	FThreadSafeCounter(const FThreadSafeCounter& Other)
	{
		this->Counter = Other.Counter;
	}

	FThreadSafeCounter(int32 Value)
	{
		Counter = Value;
	}

	// Increment and return new value.
	int32 Increment()
	{
		return FPlatformAtomics::InterlockedIncrement(&Counter);
	}

	// Adds an amount and returns the old value.
	int32 Add(int32 Amount)
	{
		return FPlatformAtomics::InterlockedAdd(&Counter, Amount);
	}

	// Decrement and return new value.
	int32 Decrement()
	{
		return FPlatformAtomics::InterlockedDecrement(&Counter);
	}

	// Subtracts an amount and returns the old value.
	int32 Subtract(int32 Amount)
	{
		return FPlatformAtomics::InterlockedAdd(&Counter, -Amount);
	}

	// Sets the counter to a specific value and returns the old value.
	int32 Set(int32 Value)
	{
		return FPlatformAtomics::InterlockedExchange(&Counter, Value);
	}

	// Resets the counter's value to zero.
	int32 Reset()
	{
		return FPlatformAtomics::InterlockedExchange(&Counter, 0);
	}

	// Gets the current value.
	int32 GetValue() const
	{
		return FPlatformAtomics::AtomicRead(&const_cast<FThreadSafeCounter*>(this)->Counter);
	}
private:
	/** Hidden on purpose as usage wouldn't be thread safe. */
	void operator= (const FThreadSafeCounter& Other) {}
	volatile int32 Counter;
};



