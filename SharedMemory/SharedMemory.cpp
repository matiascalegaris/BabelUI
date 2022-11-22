#include "SharedMemory.h"

namespace Babel {
	SharedMemory::~SharedMemory()
	{
		Close();
	}

	bool SharedMemory::Create(const char* bufferName)
	{
		mMapFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,    // use paging file
			NULL,                    // default security
			PAGE_READWRITE,          // read/write access
			0,                       // maximum object size (high-order DWORD)
			GetBufferSize(),         // maximum object size (low-order DWORD)
			bufferName);             // name of mapping object
		return true;
	}

	bool SharedMemory::Connect(const char* bufferName)
	{
		mMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS,   // read/write access
			FALSE,                 // do not inherit the name
			bufferName);               // name of mapping object

		if (mMapFile == nullptr) return false;
	}

	void* SharedMemory::MapFile(DWORD size, DWORD offset, DWORD accessMode)
	{
		return MapViewOfFile(mMapFile, accessMode, 0,offset, size);
	}

	void SharedMemory::Close()
	{
		if (mMapFile)
		{
			UnmapViewOfFile(mMapFile);
			mMapFile = nullptr;
		}
		if (mMapFile)
		{
			CloseHandle(mMapFile);
			mMapFile = nullptr;
		}
	}
	DWORD SharedMemory::GetBufferSize()
	{
		return mBuffSize;
	}
}