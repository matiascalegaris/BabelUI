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
		AddEvent(eventData, eventSize, {});
	}

	void EventBuffer::AddEvent(uint8_t* eventData, int32_t eventSize, std::function<int32_t(void*)> fillParams)
	{
		volatile LONG* lock_ptr = (volatile LONG*)&(mHeader->Lock);
		MessagesSent++;
		while (InterlockedExchange(lock_ptr, (LONG)LockState::Set) != (LONG)LockState::Clear)
		{
		}
		CopyMemory((void*)(mSharedBufferStart + mHeader->UsedBytes), (void*)eventData, eventSize);
		mHeader->UsedBytes += eventSize;
		if (fillParams)
		{
			mHeader->UsedBytes += fillParams((void*)(mSharedBufferStart + mHeader->UsedBytes));
		}
		mHeader->MessageCount++;
		
		InterlockedExchange(lock_ptr, (LONG)LockState::Clear);
	}

	void EventBuffer::AddEvent(uint8_t* eventData, int32_t eventSize, std::vector<StringInBuffer>& extraData)
	{
		AddEvent(eventData, eventSize, [&extraData](void* dest) {
			char* writePos = (char*)dest;
			for (size_t i = 0; i < extraData.size(); i++)
			{
				CopyMemory(writePos, &extraData[i].Size, sizeof(int32_t));
				writePos += sizeof(int32_t);
				if (extraData[i].Size > 0)
				{
					CopyMemory(writePos, (void*)extraData[i].StartPos, extraData[i].Size);
					writePos += extraData[i].Size;
				}				
			}
			int32_t writedData = writePos - (char*)dest;
			return writedData;
		});
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
