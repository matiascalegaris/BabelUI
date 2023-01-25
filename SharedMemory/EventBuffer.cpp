#include "EventBuffer.hpp"
#include "Core/Logger.hpp"

static int64_t MessagesSent = 0;
static int64_t MessagesHandled = 0;
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
		MessagesSent++;
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
		int64_t eventCount = mHeader->MessageCount;
		if (eventCount > 0)
		{
			CopyMemory((void*)dest, (void*)mSharedBufferStart, mHeader->UsedBytes);
			mHeader->UsedBytes = 0;
			mHeader->MessageCount = 0;
			MessagesHandled += eventCount;
		}
		InterlockedExchange(lock_ptr, (LONG)LockState::Clear);
		return eventCount;
	}
}
