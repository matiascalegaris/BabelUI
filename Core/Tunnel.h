#pragma once
#include "ProcessHandler.hpp"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include <memory>
#include "CallbackDefines.hpp"

namespace Babel
{
	class Tunnel : public EventListener
	{
	public:
		~Tunnel();
		bool Initialize(int width, int height);
		void Terminate();

		bool InitializeDebugWindows(int width, int height);
		SyncData& GetDebugSyncData() { return *mDebugSyncData; }
		SyncData& GetSyncData() { return *mSyncData; }

		void HandleEvent(const Event& eventData) override;

		void CheckIncommingMessages();
		void SetCallbacks(const CallbacksList& callbacks) { mVBCallbacks = callbacks;  }
	private:
		ProcessHandler mProcess;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<SyncData> mSyncData;
		std::unique_ptr<EventHandler> mEventHandler;

		std::unique_ptr<SharedMemory> mDebugSharedMemory;
		std::unique_ptr<SyncData> mDebugSyncData;
		CallbacksList mVBCallbacks;
	};
}