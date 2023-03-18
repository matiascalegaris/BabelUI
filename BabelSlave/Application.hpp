#pragma once
#include <memory>
#include "Renderer.h"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include "Communicator.hpp"
#include "CommonDefines.hpp"

namespace Babel
{
	struct AppSettings {
		int Width{ 0 };
		int Height { 0 };
		bool CompressedResources { false };
		bool EnableDebug { false };
	};

	class Application
	{
	public:
		Application();
		void Initialize(const AppSettings& settings);

		void Run();

		void Stop();
		void EnableDebugWindow(int width, int height);

		const AppSettings& GetSettings() const { return mSettings; }
	private:
		void Update();
		void UpdateRemoteFrame();
	private:
		AppSettings mSettings;
		std::unique_ptr<Renderer> mRenderer;
		std::unique_ptr<SyncData> mSyncData;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<EventHandler> mEventHandler;
		std::unique_ptr<SyncData> mDebugSyncData;
		std::unique_ptr<SharedMemory> mDebugSharedMemory;
		std::unique_ptr<Communicator> mCommunicator;
		bool mRun{ false };
		bool mActiveDebugView{ false };
		int64_t expectedFrameTime{ 10 };
	};
}