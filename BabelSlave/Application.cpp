#include "Application.hpp"

#include <chrono>
#include <thread>

#include "Core/Logger.hpp"

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
		case EventType::DebugMouseData:
		{
			const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
			mRenderer->SendInpectorMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
			break;
		}
		case EventType::EnableDebugWindow:
		{
			const WindowInfo& windowEvtdata = static_cast<const WindowInfo&>(eventData);
			EnableDebugWindow(windowEvtdata.Width, windowEvtdata.Height);
			break;
		}
		case EventType::KeyData:
		{
			const KeyEvent& keyData = static_cast<const KeyEvent&>(eventData);
			HandlekeyData(keyData);
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
		if (mActiveDebugView)
		{
			bitmap_surface = mRenderer->GetInspectorSurface();
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
	void Application::EnableDebugWindow(int width, int height)
	{
		mRenderer->EnableInspector(width, height);
		mDebugSyncData = std::make_unique<SyncData>(width, height, 4, 3);
		mDebugSharedMemory = std::make_unique<SharedMemory>(mDebugSyncData->GetTotalSize());
		mDebugSharedMemory->Connect("Local\\TestMemShare2Debug");
		mDebugSyncData->GetSharedFileViews(*mDebugSharedMemory);
		mActiveDebugView = true;
	}
	void Application::HandlekeyData(const KeyEvent& keyData)
	{
		ultralight::KeyEvent evt;
		LOGGER->log("Got ket event " + std::to_string(keyData.Type));
		evt.type = static_cast<ultralight::KeyEvent::Type>(keyData.Type);
		switch (keyData.Type)
		{
		case ultralight::KeyEvent::kType_KeyDown:
			evt.type = ultralight::KeyEvent::kType_RawKeyDown;
		case ultralight::KeyEvent::kType_RawKeyDown:
		case ultralight::KeyEvent::kType_KeyUp:
			evt.virtual_key_code = keyData.KeyCode;
			evt.native_key_code = 0;
			evt.modifiers = 0;
			GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);
			mRenderer->SendKeyEvent(evt, keyData.Inspector);
			break;
		case ultralight::KeyEvent::kType_Char:
		{
			char key = static_cast<char>(keyData.KeyCode);
			evt.text = std::string(1,key).c_str();
			evt.unmodified_text = evt.text;
			mRenderer->SendKeyEvent(evt, keyData.Inspector);
		}
		default:
			break;
		}
	}
}
