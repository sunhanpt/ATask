#pragma once;
#include <queue>
#include "ScopeLock.h"

template<typename TQueueNode>
class FThreadSafeQueue
{
public:
	FThreadSafeQueue() {}
	~FThreadSafeQueue() {}
	//TQueueNode& front()
	//{
	//	FScopeLock ScopeLock(SyncObject);
	//	return InnerQueue.front();
	//}

	//const TQueueNode& front()
	//{
	//	return this->front();
	//}

	bool empty()
	{
		return InnerQueue.empty();
	}

	void push(const TQueueNode& node)
	{
		FScopeLock ScopeLock(SyncObject);
		InnerQueue.push(node);
	}

	TQueueNode& pop()
	{
		FScopeLock ScopeLock(SyncObject);
		TQueueNode& Node = InnerQueue.front();
		InnerQueue.pop();
		return Node;
	}

	const TQueueNode& pop()
	{
		return this->pop();
	}

private:
	std::queue<TQueueNode> InnerQueue;
	FCriticalSection SyncObject;
};