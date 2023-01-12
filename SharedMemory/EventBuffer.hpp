#pragma once
#include <cstdint>
#include <Windows.h>

namespace Babel
{
	class EventBuffer
	{
	public:
		EventBuffer(uint8_t* bufferPtr, int32_t size);

		void AddEvent(uint8_t* eventData, int32_t eventSize);

		int64_t GetAviableEvents(uint8_t* dest, int32_t masxBufferSize);

		int64_t GetBufferSize() { return mBufferSize; }
	private:
		struct BufferHeader {
			LONG Lock;
			int32_t MessageCount;
			int32_t UsedBytes;
		};
	private:
		uint8_t* mSharedBufferStart;
		BufferHeader* mHeader;
		int64_t mBufferSize;
		int32_t mSendItemCount;
	};
}