#pragma once
#include <Ultralight/Ultralight.h>
#include <Ultralight/platform/Logger.h>
#include <AppCore/Platform.h>
#include <AppCore/JSHelpers.h>
#include "FileSystem.h"

namespace Babel
{
	class Communicator;
	class Renderer : public ultralight::LoadListener,
		public ultralight::ViewListener,
		public ultralight::Logger
	{
	public:
		Renderer(int width, int height, bool compressedResources);
		virtual ~Renderer();

		virtual void OnFinishLoading(ultralight::View* caller,
			uint64_t frame_id, bool is_main_frame, const ultralight::String& url) override;
		void OnFailLoading(ultralight::View* caller,
			uint64_t frame_id,
			bool is_main_frame,
			const ultralight::String& url,
			const ultralight::String& description,
			const ultralight::String& error_domain,
			int error_code) override;
		void OnDOMReady(ultralight::View* caller,
			uint64_t frame_id,
			bool is_main_frame,
			const ultralight::String& url) override;
		virtual void LogMessage(ultralight::LogLevel log_level, const ultralight::String& message) override;
		virtual ultralight::RefPtr<ultralight::View> OnCreateInspectorView(ultralight::View* caller, bool is_local,
			const ultralight::String& inspected_url) override;
		void RenderFrame();
		void PerformUpdate();
		ultralight::BitmapSurface* GetSurface();
		ultralight::BitmapSurface* GetInspectorSurface();
		void SendMouseEvent(int mouseX, int mouseY, uint8_t evtType, uint8_t button);
		void SendInpectorMouseEvent(int mouseX, int mouseY, uint8_t evtType, uint8_t button);
		void EnableInspector(int width, int height);
		void SendKeyEvent(ultralight::KeyEvent& evt, bool isInspectorEvent);
		void SetCommunicator(Communicator* communicator) { mCommunicator = communicator; }
		ultralight::RefPtr<ultralight::View>& GetMainView() { return mView; }
	private:
		ultralight::RefPtr<ultralight::Renderer> mRender;
		ultralight::RefPtr<ultralight::View> mView;
		ultralight::RefPtr<ultralight::View> mInspectorView;
		Communicator* mCommunicator;
		std::unique_ptr<BabelFileSystemWin> mFileSystem;
		std::wstring mLocalPath;
		bool mLoadComplete = false;
		int mInspectorWidth;
		int mInspectorHeight;
	};
}