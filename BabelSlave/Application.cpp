#include "Application.hpp"

#include <chrono>
#include <thread>

#include "Core/Logger.hpp"

namespace Babel
{
	const int CheckForActiveProcessTime = 30;
	Application::Application()
	{
	}

	void Application::Initialize(const AppSettings& settings)
	{
		mSettings = settings;
		mRenderer = std::make_unique<Renderer>(mSettings.Width, mSettings.Height, mSettings.CompressedResources);
		mSyncData = std::make_unique<SyncData>(mSettings.Width, mSettings.Height, 4, 3);
		mSharedMemory = std::make_unique<SharedMemory>(mSyncData->GetTotalSize());
		mSharedMemory->Connect(("Local\\TestMemShare2" + settings.TunnelName).c_str());
		mSyncData->GetSharedFileViews(*mSharedMemory);
		mCommunicator = std::make_unique<JSBridge>(mSyncData->GetSlaveMessenger(), *mRenderer, *this);
		mEventHandler = std::make_unique<EventHandler>(*mCommunicator, mSyncData->GetApiMessenger());
		mRenderer->SetCommunicator(mCommunicator.get());
	}

	void Application::Run()
	{
		mRun = true;
		mBackgroundTask = std::thread(&Application::TestIfMasterIsAlive, this);
		while (mRun)
		{
			std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
			Update();
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			int64_t frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
			frameTime = expectedFrameTime - frameTime;
			if (frameTime > 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(frameTime));
			}
		}
		mBackgroundTask.join();
	}

	void Application::Stop()
	{
		mRun = false;
		mCloseAppCondition.notify_all();
	}

	void Application::Update()
	{
		mEventHandler->HandleIncomingEvents();
		mRenderer->PerformUpdate();
		mRenderer->RenderFrame();
		UpdateRemoteFrame();
	}
	void Application::UpdateRemoteFrame()
	{
		ultralight::BitmapSurface* bitmap_surface = mRenderer->GetSurface();
		//if (!bitmap_surface->dirty_bounds().IsEmpty())
		{
			auto bmp = bitmap_surface->bitmap();
			void* pixels = bmp->LockPixels();
			uint32_t width = bmp->width();
			uint32_t height = bmp->height();
			uint32_t stride = bmp->row_bytes();
			mSyncData->WriteCurrentImage(pixels, width * height * 4);
			bmp->UnlockPixels();
			bitmap_surface->ClearDirtyBounds();
		}
		if (mActiveDebugView)
		{
			bitmap_surface = mRenderer->GetInspectorSurface();
			if (bitmap_surface != nullptr)
			{
				auto bmp = bitmap_surface->bitmap();
				void* pixels = bmp->LockPixels();
				uint32_t width = bmp->width();
				uint32_t height = bmp->height();
				uint32_t stride = bmp->row_bytes();
				mDebugSyncData->WriteCurrentImage(pixels, width * height * 4);
				bmp->UnlockPixels();
				bitmap_surface->ClearDirtyBounds();
			}			
		}
	}
	void Application::TestIfMasterIsAlive()
	{
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, mSettings.ParentProcessId);
		std::mutex localMutex;
		while (mRun)
		{
			std::unique_lock<std::mutex> l(localMutex);
			mCloseAppCondition.wait_for(l, std::chrono::seconds(CheckForActiveProcessTime), [this]() { return !mRun; });
			DWORD ret = WaitForSingleObject(hProcess, 0);
			mRun = mRun && ret == WAIT_TIMEOUT;
		}
		
	}
	void Application::EnableDebugWindow(int width, int height)
	{
		if (!mSettings.EnableDebug) return;
		mRenderer->EnableInspector(width, height);
		mDebugSyncData = std::make_unique<SyncData>(width, height, 4, 3);
		mDebugSharedMemory = std::make_unique<SharedMemory>(mDebugSyncData->GetTotalSize());
		mDebugSharedMemory->Connect(("Local\\TestMemShare2Debug" + mSettings.TunnelName).c_str());
		mDebugSyncData->GetSharedFileViews(*mDebugSharedMemory);
		mActiveDebugView = true;
	}
}
