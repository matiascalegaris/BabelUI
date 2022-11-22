#include "Application.hpp"

#include <chrono>
#include <thread>



namespace Babel
{
	Application::Application()
	{
	}

	void Application::Initialize(int width, int height)
	{
		mRenderer = std::make_unique<Renderer>(width, height);
		mSyncData = std::make_unique<SyncData>(width, height, 4, 3);
		mSharedMemory = std::make_unique<SharedMemory>(mSyncData->GetTotalSize());
		mSharedMemory->Connect("Local\\TestMemShare2");
		mSyncData->GetSharedFileViews(*mSharedMemory);
		mEventHandler = std::make_unique<EventHandler>(*this, mSyncData->GetApiMessenger());
	}

	void Application::Run()
	{
		mRun = true;
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
	}

	void Application::HandleEvent(const Event& eventData)
	{
		switch (eventData.EventType)
		{
		case EventType::MouseData:
		{
			const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
			mRenderer->SendMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
			break;
		}
		case EventType::Close:
			mRun = false;
		default:
			break;
		}
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
	}
}
