#pragma once
#include <Ultralight/Ultralight.h>
#include <Ultralight/platform/Logger.h>
#include <AppCore/Platform.h>
#include <AppCore/JSHelpers.h>

namespace Babel
{
	class Renderer : public ultralight::LoadListener,
		public ultralight::Logger
	{
	public:
		Renderer(int width, int height);
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
		virtual void LogMessage(ultralight::LogLevel log_level, const ultralight::String16& message) override;

		void RenderFrame();
		void PerformUpdate();
		ultralight::BitmapSurface* GetSurface();

		void SendMouseEvent(int mouseX, int mouseY, uint8_t evtType, uint8_t button);
	public:
		ultralight::JSValue GetMessage(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
	private:
		ultralight::RefPtr<ultralight::Renderer> mRender;
		ultralight::RefPtr<ultralight::View> mView;
		bool mLoadComplete = false;
	};
}