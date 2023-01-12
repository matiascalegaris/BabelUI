#pragma once
#include <memory>
#include "Renderer.h"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include "SharedMemory/Events/EventHandler.hpp"

namespace Babel
{
	class Application : public EventListener
	{
	public:
		Application();
		void Initialize(int width, int height);

		void Run();

	public:
		void HandleEvent(const Event& evenData) override;
	private:
		void Update();
		void UpdateRemoteFrame();

		void EnableDebugWindow(int width, int height);

		void HandlekeyData(const KeyEvent& keyData);
	private:
		std::unique_ptr<Renderer> mRenderer;
		std::unique_ptr<SyncData> mSyncData;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<EventHandler> mEventHandler;
		std::unique_ptr<SyncData> mDebugSyncData;
		std::unique_ptr<SharedMemory> mDebugSharedMemory;
		bool mRun{ false };
		bool mActiveDebugView{ false };
		int64_t expectedFrameTime{ 16 };
	};
}