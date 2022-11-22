#pragma once
#include "ProcessHandler.hpp"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include <memory>

namespace Babel
{
	class Tunnel
	{
	public:
		~Tunnel();
		bool Initialize(int width, int height);
		void Terminate();

		SyncData& GetSyncData() { return *mSyncData; }
	private:
		ProcessHandler mProcess;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<SyncData> mSyncData;
	};
}