#pragma once
#include <cstdint>
#include <functional>
#include <Windows.h>
#include "Events/EventHandler.hpp"


namespace Babel
{
	template<typename T>
	int32_t VectorSizeOf(const typename std::vector<T>& vec)
	{
		return sizeof(T) * vec.size();
	}

	class EventBuffer
	{
	public:
		EventBuffer(uint8_t* bufferPtr, int32_t size);

		void AddEvent(uint8_t* eventData, int32_t eventSize);
		
		// fillparams let you attach extra info to an event, be carefull with this since it is executed in a critical section
		// this let us add dynamic size events to the buffer in an atomic operation but it also increase the lock and sync times
		void AddEvent(uint8_t* eventData, int32_t eventSize, std::function<int32_t(void*)> fillParams);
		void AddEvent(uint8_t* eventData, int32_t eventSize, std::vector<StringInBuffer>& extraData);

		template<typename T>
		void AddEvent(uint8_t* eventData, int32_t eventSize, std::vector<T>& extraData)
		{
			AddEvent(eventData, eventSize, [&extraData](void* dest) {
				char* writePos = (char*)dest;
				int32_t memSize = VectorSizeOf(extraData);
				int32_t elementCount = extraData.size();
				CopyMemory(writePos, &elementCount, sizeof(int32_t));
				writePos += sizeof(int32_t);
				CopyMemory(writePos, extraData.data(), memSize);
				writePos += memSize;
				int32_t writedData = writePos - (char*)dest;
				return writedData;
			});

		}

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