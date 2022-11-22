#include "EventBuffer.hpp"
#include <Windows.h>

enum class LockState : int32_t
{
	Clear = 0,
	Set = 1
};

namespace Babel
{
	EventBuffer::EventBuffer(uint8_t* bufferPtr, int32_t size)
		: mSharedBufferStart(bufferPtr + (sizeof(BufferHeader))),
		mHeader((BufferHeader*)bufferPtr),
		mBufferSize(size),
		mSendItemCount(0)
	{
		mHeader->Lock = 0;
		mHeader->MessageCount = 0;
		mHeader->UsedBytes = 0;
	}
	void EventBuffer::AddEvent(uint8_t* eventData, int32_t eventSize)
	{
		volatile LONG* lock_ptr = (volatile LONG*)&(mHeader->Lock);
		while (InterlockedExchange(lock_ptr, (LONG)LockState::Set) != (LONG)LockState::Clear)
		{
		}
		CopyMemory((void*)(mSharedBufferStart + mHeader->UsedBytes), (void*)eventData, eventSize);
		mHeader->UsedBytes += eventSize;
		mHeader->MessageCount++;
		InterlockedExchange(lock_ptr, (LONG)LockState::Clear);
	}
	int64_t EventBuffer::GetAviableEvents(uint8_t* dest, int32_t masxBufferSize)
	{
		volatile LONG* lock_ptr = (volatile LONG*)&(mHeader->Lock);
		while (InterlockedExchange(lock_ptr, (LONG)LockState::Set) != (LONG)LockState::Clear)
		{
		}
		CopyMemory((void*)dest, (void*)mSharedBufferStart, mHeader->UsedBytes);
		int64_t eventCount = mHeader->MessageCount;
		mHeader->UsedBytes = 0;
		mHeader->MessageCount = 0;
		InterlockedExchange(lock_ptr, (LONG)LockState::Clear);
		return eventCount;
	}
}
