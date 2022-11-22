#pragma once
#include <Windows.h>

namespace Babel {
class SharedMemory
{
public:
	SharedMemory(DWORD bufferSize) : mBuffSize(bufferSize) {}
	~SharedMemory();
	bool Create(const char *bufferName);
	bool Connect(const char* bufferName);
	void* MapFile(DWORD size, DWORD offset, DWORD accessMode);
	void Close();
private:
	DWORD GetBufferSize();
private:
	HANDLE mMapFile{0};
	DWORD mBuffSize;
};
}