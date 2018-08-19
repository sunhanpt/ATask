#include "LockFreeList.h"
#include "ThreadCoreGlobals.h"

void LockFreeTagCounterHasOverflowed()
{
	FPlatformProcess::Sleep(.001f);
}

void LockFreeLinksExhausted(uint32 TotalNum)
{
	//UE_LOG(LogTemp, Fatal, TEXT("Consumed %d lock free links; there are no more."), TotalNum);
}

static void ChangeMem(int32 Delta)
{
#if 0   // this is not safe; we cannot change stats in the middle of a lock free operation
	static FThreadSafeCounter LockFreeListMem;
	LockFreeListMem.Add(Delta);
	if (GIsRunning)
	{
		SET_MEMORY_STAT(STAT_LockFreeListLinks, LockFreeListMem.GetValue());
	}
#endif
}

void* LockFreeAllocLinks(size_t AllocSize)
{
	ChangeMem(AllocSize);
	return malloc(AllocSize);
}
void LockFreeFreeLinks(size_t AllocSize, void* Ptr)
{
	ChangeMem(-int32(AllocSize));
	free(Ptr);
}


// 实现一个队列。对列真正的数据存在在TLockFreeAllocOnceIndexedAllocator中，真正的存储分成块存储。
// 这里的队列也是分块，但是这里的分块只是逻辑上的，每个块包含一顶数量的单独的bundle。每个块会记录起始的PartialBundle，以及块大小。
// 而块内的数据通过TLink::payLoad指向上一个PartialBundle，而保持连接状态。
// TLS.PartialBundle = Item; 这里TLS.PartialBundle保存当前处理的PartialBundle。PartialBundle真正的意义是指向一个TLockFreeAllocOnceIndexedAllocator的数据单元
class LockFreeLinkAllocator_TLSCache : public FNoncopyable
{
	enum
	{
		NUM_PER_BUNDLE = 64,
	};

	typedef FLockFreeLinkPolicy::TLink TLink;
	typedef FLockFreeLinkPolicy::TLinkPtr TLinkPtr;

public:

	LockFreeLinkAllocator_TLSCache()
	{
		assert(IsInGameThread());
		TlsSlot = FPlatformTLS::AllocTlsSlotUE();
		assert(FPlatformTLS::IsValidTlsSlotUE(TlsSlot));
	}
	/** Destructor, leaks all of the memory **/
	~LockFreeLinkAllocator_TLSCache()
	{
		FPlatformTLS::FreeTlsSlot(TlsSlot);
		TlsSlot = 0;
	}

	/**
	* Allocates a memory block of size SIZE.
	*
	* @return Pointer to the allocated memory.
	* @see Free
	*/
	TLinkPtr Pop()
	{
		FThreadLocalCache& TLS = GetTLS();

		if (!TLS.PartialBundle)
		{
			if (TLS.FullBundle)
			{
				TLS.PartialBundle = TLS.FullBundle;
				TLS.FullBundle = 0;
			}
			else
			{
				TLS.PartialBundle = GlobalFreeListBundles.Pop();
				if (!TLS.PartialBundle)
				{
					int32 FirstIndex = FLockFreeLinkPolicy::LinkAllocator.Alloc(NUM_PER_BUNDLE); // 保证多线程互斥
					for (int32 Index = 0; Index < NUM_PER_BUNDLE; Index++)
					{
						TLink* Event = FLockFreeLinkPolicy::IndexToLink(FirstIndex + Index);
						Event->DoubleNext.Init();
						Event->SingleNext = 0;
						Event->Payload = (void*)UPTRINT(TLS.PartialBundle);
						TLS.PartialBundle = FLockFreeLinkPolicy::IndexToPtr(FirstIndex + Index);
					}
				}
			}
			TLS.NumPartial = NUM_PER_BUNDLE;
		}
		TLinkPtr Result = TLS.PartialBundle;
		TLink* ResultP = FLockFreeLinkPolicy::DerefLink(TLS.PartialBundle);
		TLS.PartialBundle = TLinkPtr(UPTRINT(ResultP->Payload));
		TLS.NumPartial--;
		ResultP->Payload = nullptr;
		assert(!ResultP->DoubleNext.GetPtr() && !ResultP->SingleNext);
		return Result;
	}

	/**
	* Puts a memory block previously obtained from Allocate() back on the free list for future use.
	*
	* @param Item The item to free.
	* @see Allocate
	*/
	void Push(TLinkPtr Item)
	{
		FThreadLocalCache& TLS = GetTLS();
		if (TLS.NumPartial >= NUM_PER_BUNDLE)
		{
			if (TLS.FullBundle)
			{
				GlobalFreeListBundles.Push(TLS.FullBundle);
				//TLS.FullBundle = nullptr;
			}
			TLS.FullBundle = TLS.PartialBundle;
			TLS.PartialBundle = 0;
			TLS.NumPartial = 0;
		}
		TLink* ItemP = FLockFreeLinkPolicy::DerefLink(Item);
		ItemP->DoubleNext.SetPtr(0);
		ItemP->SingleNext = 0;
		ItemP->Payload = (void*)UPTRINT(TLS.PartialBundle);
		TLS.PartialBundle = Item;
		TLS.NumPartial++;
	}

private:

	/** struct for the TLS cache. */
	struct FThreadLocalCache
	{
		TLinkPtr FullBundle;
		TLinkPtr PartialBundle;
		int32 NumPartial;

		FThreadLocalCache()
			: FullBundle(0)
			, PartialBundle(0)
			, NumPartial(0)
		{
		}
	};

	FThreadLocalCache& GetTLS()
	{
		assert(FPlatformTLS::IsValidTlsSlotUE(TlsSlot));
		FThreadLocalCache* TLS = (FThreadLocalCache*)FPlatformTLS::GetTlsValueUE(TlsSlot);
		if (!TLS)
		{
			TLS = new FThreadLocalCache();
			FPlatformTLS::SetTlsValueUE(TlsSlot, TLS);
		}
		return *TLS;
	}

	/** Slot for TLS struct. */
	uint32 TlsSlot;

	/** Lock free list of free memory blocks, these are all linked into a bundle of NUM_PER_BUNDLE. */
	FLockFreePointerListLIFORoot<PLATFORM_CACHE_LINE_SIZE> GlobalFreeListBundles;
};

static LockFreeLinkAllocator_TLSCache GLockFreeLinkAllocator;

void FLockFreeLinkPolicy::FreeLockFreeLink(FLockFreeLinkPolicy::TLinkPtr Item)
{
	GLockFreeLinkAllocator.Push(Item);
}

FLockFreeLinkPolicy::TLinkPtr FLockFreeLinkPolicy::AllocLockFreeLink()
{
	FLockFreeLinkPolicy::TLinkPtr Result = GLockFreeLinkAllocator.Pop();
	// this can only really be a mem stomp
	assert(Result && !FLockFreeLinkPolicy::DerefLink(Result)->DoubleNext.GetPtr() && !FLockFreeLinkPolicy::DerefLink(Result)->Payload && !FLockFreeLinkPolicy::DerefLink(Result)->SingleNext);
	return Result;
}

FLockFreeLinkPolicy::TAllocator FLockFreeLinkPolicy::LinkAllocator;
