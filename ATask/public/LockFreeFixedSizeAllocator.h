//#pragma once
//#include "CoreTypes.h"
//#include "Template.h"
//#include "WindowsPlatformTLS.h"
//
///** Fake Thread safe counter, used to avoid cluttering code with ifdefs when counters are only used for debugging. */
//class FNoopCounter
//{
//public:
//	FNoopCounter(){}
//	FNoopCounter(const FNoopCounter& Other) {}
//	FNoopCounter(int32 Value) {}
//
//	int32 Increment() { return 0; }
//	int32 Add(int32 Amount) { return 0; }
//	int32 Decrement() { return 0; }
//	int32 Subtract(int32 Amount) { return 0; }
//	int32 Set(int32 Value) { return 0; }
//	int32 Reset() { return 0; }
//	int32 GetValue() const { return 0; }
//};
//
///**
//* Thread safe, lock free pooling allocator of fixed size blocks that
//* never returns free space, even at shutdown
//* alignment isn't handled, assumes FMemory::Malloc will work
//*/
//
//template<int32 SIZE, typename TBundleRecycler, typename TTrackingCounter = FNoopCounter>
//class TLockFreeFixedSizeAllocator_TLSCacheBase : public FNoncopyable
//{
//	enum 
//	{
//		NUM_PER_BUNDLE = 32,
//	};
//
//public:
//	TLockFreeFixedSizeAllocator_TLSCacheBase()
//	{
//		static_assert(SIZE >= sizeof(void*) && SIZE % sizeof(void*) == 0, "Blocks in TLockFreeFixedSizeAllocator must be at least the size of a pointer.");
//		assert(IsInGameThread());
//		TlsSlot = FPlatformTLS::AllocTlsSlotUE();
//		assert(FPlatformTLS::IsValidTlsSlotUE(TlsSlot));
//		FPlatformTLS::SetTlsValueUE(TlsSlot, nullptr);
//	}
//
//	~TLockFreeFixedSizeAllocator_TLSCacheBase()
//	{
//		FPlatformTLS::FreeTlsSlot(TlsSlot);
//		TlsSlot = 0;
//	}
//
//	__forceinline void* Allocate()
//	{
//		FThreadLocalCache& TLS = GetTLS();
//		if (!TLS.PartialBundle)
//		{
//			if (TLS.FullBundle)
//			{
//				TLS.PartialBundle = TLS.FullBundle;
//				TLS.FullBundle = nullptr;
//			}
//			else
//			{
//				TLS.PartialBundle = GlobalFreeListBundles.Pop();
//				if (!TLS.PartialBundle)
//				{
//
//				}
//			}
//		}
//	}
//
//private:
//	struct FThreadLocalCache
//	{
//		void **FullBundle;
//		void **PartialBundle;
//		int NumPartial;
//
//		FThreadLocalCache()
//			: FullBundle(nullptr)
//			, PartialBundle(nullptr)
//			, NumPartial(0)
//		{}
//	};
//
//	FThreadLocalCache& GetTLS()
//	{
//		assert(FPlatformTLS::IsValidTlsSlotUE(TlsSlot));
//		FThreadLocalCache* TLS = (FThreadLocalCache*)FPlatformTLS::GetTlsValueUE(TlsSlot);
//		if (!TLS)
//		{
//			TLS = new FThreadLocalCache();
//			FPlatformTLS::SetTlsValueUE(TlsSlot, TLS);
//		}
//		return *TLS;
//	}
//
//	uint32 TlsSlot;
//	TBundleRecycler GlobalFreeListBundles;
//	TTrackingCounter NumUsed; // for debug
//	TTrackingCounter NumFree; // for debug
//};