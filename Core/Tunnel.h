#pragma once
#include "ProcessHandler.hpp"
#include "SharedMemory/SharedMemory.h"
#include "SharedMemory/SyncData.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include <memory>
#include "CallbackDefines.hpp"
#include "CommonDefines.hpp"

namespace Babel
{
	class Tunnel : public EventListener
	{
	public:
		~Tunnel();
		bool Initialize(const Settings& settings);
		void Terminate();

		bool InitializeDebugWindows(int width, int height);
		SyncData& GetDebugSyncData() { return *mDebugSyncData; }
		SyncData& GetSyncData() { return *mSyncData; }

		void HandleEvent(const Event& eventData) override;

		void CheckIncommingMessages();
		void SetCallbacks(const CallbacksList& callbacks) { mVBCallbacks = callbacks;  }
		void SetGameplayCallbacks(const GameplayCallbacks& callbacks) { mGameplayVBcallbacks = callbacks; }

	public:
		void SetActiveScreen(const char* name);
		void SetLoadingMessage(const char* message, bool localize);
	private:
		Settings mSettings;
		ProcessHandler mProcess;
		std::unique_ptr<SharedMemory> mSharedMemory;
		std::unique_ptr<SyncData> mSyncData;
		std::unique_ptr<EventHandler> mEventHandler;

		std::unique_ptr<SharedMemory> mDebugSharedMemory;
		std::unique_ptr<SyncData> mDebugSyncData;
		CallbacksList mVBCallbacks;
		GameplayCallbacks mGameplayVBcallbacks;
		std::string mSharedMemName;
	};
}