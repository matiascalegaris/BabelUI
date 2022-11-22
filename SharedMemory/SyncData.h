#pragma once
#include <vector>
#include <memory>
#include "EventBuffer.hpp"

namespace Babel
{
	struct ImageBuffer
	{
		int Width;
		int Height;
		int8_t PixelSize;
		std::vector<uint8_t> Data;
	};

	struct SyncDataHeader
	{
		int LastWritteBuffer;
		int LastReadBuffer;
	};

	class SharedMemory;
	class SyncData
	{
	public:
		SyncData(int imageWith, int imageHeight, int imagePixelSize, int bufferCount);
		~SyncData();
		int GetTotalSize();
		bool GetLastBuffer(char* dest, int maxSize);
		void WriteCurrentImage(void* data, int size);
		void GetSharedFileViews(SharedMemory& sharedMemory);
		EventBuffer& GetApiMessenger() { return *mApiWriteBuffer; }
		EventBuffer& GetSlaveMessenger() { return *mSlaveWriteBuffer; }
	private:
		void* GetImageBufferPtr(int bufferIndex);
		SyncDataHeader mSyncHeader;
		std::unique_ptr<EventBuffer> mApiWriteBuffer;
		std::unique_ptr<EventBuffer> mSlaveWriteBuffer;
		int mImageSize;
		int mBufferCount;
		void* mFileView;
	};
}