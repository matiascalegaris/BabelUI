#pragma once
#include <memory>
#include "Renderer.h"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include "Communicator.hpp"

namespace Babel
{
	class Application
	{
	public:
		Application();
		void Initialize(int width, int height);

		void Run();

		void Stop();
		void EnableDebugWindow(int width, int height);
	private:
		void Update();
		void UpdateRemoteFrame();
	private:
		std::unique_ptr<Renderer> mRenderer;
		std::unique_ptr<SyncData> mSyncData;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<EventHandler> mEventHandler;
		std::unique_ptr<SyncData> mDebugSyncData;
		std::unique_ptr<SharedMemory> mDebugSharedMemory;
		std::unique_ptr<Communicator> mCommunicator;
		bool mRun{ false };
		bool mActiveDebugView{ false };
		int64_t expectedFrameTime{ 16 };
	};
}