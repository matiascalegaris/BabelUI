#include "SyncData.h"
#include <Windows.h>
#include <stdexcept>

#include "SharedMemory.h"

namespace Babel
{
	namespace
	{
		const int32_t MessageEventBufferSize = 1024 * 500;
	}
	SyncData::SyncData(int imageWith, int imageHeight, int imagePixelSize, int bufferCount)
		: mImageSize(imageHeight* imageWith* imagePixelSize), mBufferCount(bufferCount)
	{
	}

	SyncData::~SyncData()
	{
	}

	int SyncData::GetTotalSize()
	{
		return mImageSize * mBufferCount + sizeof(SyncDataHeader) + MessageEventBufferSize * 2;
	}

	bool SyncData::GetLastBuffer(char* dest, int maxSize)
	{
		if (!mFileView)
		{
			return false;
		}
		auto header = (SyncDataHeader*)mFileView;
		if (header->LastWritteBuffer == 0)
		{
			return false;
		}
		CopyMemory(dest, GetImageBufferPtr(header->LastWritteBuffer % 3), maxSize);
		return true;
	}

	void SyncData::WriteCurrentImage(void* data, int size)
	{
		auto header = (SyncDataHeader*)mFileView;
		void* address = GetImageBufferPtr(header->LastWritteBuffer % 3);
		if (address)
		{
			CopyMemory(address, data, size);
			header->LastWritteBuffer += 1;
		}
	}

	void SyncData::GetSharedFileViews(SharedMemory& sharedMemory)
	{
		mFileView = sharedMemory.MapFile(0, 0, FILE_MAP_ALL_ACCESS);
		if (mFileView == nullptr)
		{
			throw std::runtime_error("Failed to get shared memory reference");
			auto err = GetLastError();
			return;
		}
		auto header = (SyncDataHeader*)mFileView;
		header->LastReadBuffer = 0;
		header->LastWritteBuffer = 0;
		uint8_t* MessageBufferStart = (uint8_t*)GetImageBufferPtr(3);

		mApiWriteBuffer = std::make_unique<EventBuffer>(MessageBufferStart, MessageEventBufferSize);
		MessageBufferStart = MessageBufferStart + MessageEventBufferSize;
		mSlaveWriteBuffer = std::make_unique<EventBuffer>(MessageBufferStart, MessageEventBufferSize);
	}

	void* SyncData::GetImageBufferPtr(int bufferIndex)
	{
		if (mFileView == nullptr)
		{
			return nullptr;
		}
		return ((uint8_t*)mFileView + (sizeof(SyncDataHeader) + mImageSize * bufferIndex));
	}
}