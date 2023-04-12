#include "Renderer.h"
#include <thread>
#include <chrono>
#include <filesystem>
#include "Core/Logger.hpp"
#include "SharedMemory/Events/EventHandler.hpp"
#include "JSBridge.hpp"
#include "Utils/FileUtils.h"

const char* htmlString();

namespace Babel
{

    using namespace ultralight;
	Renderer::Renderer(int width, int height, bool compressedResources)
	{
        ///
    /// Setup our config. The config can be used to customize various
    /// options within the renderer and WebCore module.
    ///
    /// Our config uses 2x DPI scale and "Arial" as the default font.
    ///
        Config config;
        mLocalPath = GetWorkingPath().native();

        ///
        /// Pass our configuration to the Platform singleton so that the library
        /// can use it.
        ///
        /// The Platform singleton can be used to define various platform-specific
        /// properties and handlers such as file system access, font loaders, and
        /// the gpu driver.
        ///
        Platform::instance().set_config(config);

        ///
        /// Use AppCore's font loader singleton to load fonts from the OS.
        ///
        /// You could replace this with your own to provide your own fonts.
        ///
        Platform::instance().set_font_loader(GetPlatformFontLoader());

        ///
        /// Use AppCore's file system singleton to load file:/// URLs from the OS.
        ///
        /// You could replace this with your own to provide your own file loader
        /// (useful if you need to bundle encrypted / compressed HTML assets).
        ///
        mFileSystem = std::make_unique<BabelFileSystemWin>(GetWorkingPath().c_str(), compressedResources);
        Platform::instance().set_file_system(mFileSystem.get());
        //Platform::instance().set_file_system(GetPlatformFileSystem(GetWorkingPath().u8string().c_str()));

        ///
        /// Register our MyApp instance as a logger so we can handle the
        /// library's LogMessage() event below in case we encounter an error.
        ///
        Platform::instance().set_logger(this);

        ///
        /// Create our Renderer (you should only create this once per application).
        /// 
        /// The Renderer singleton maintains the lifetime of the library and
        /// is required before creating any Views. It should outlive any Views.
        ///
        /// You should set up the Platform singleton before creating this.
        ///
        mRender = ultralight::Renderer::Create();
        ViewConfig view_config;
        view_config.initial_device_scale = 1.0;
        view_config.font_family_standard = "Arial";
        view_config.is_accelerated = false;
        view_config.is_transparent = true;
        ///
        /// Create our View.
        ///
        /// Views are sized containers for loading and displaying web content.
        ///
        mView = mRender->CreateView(width, height, view_config, nullptr);

        ///
        /// Register our MyApp instance as a load listener so we can handle the
        /// View's OnFinishLoading event below.
        ///
        mView->set_load_listener(this);
        mView->set_view_listener(this);

        mView->LoadURL("file:///BabelUI/index.html");
        //mView->LoadHTML(htmlString());
        mView->Focus();
	}
    Renderer::~Renderer()
    {
        mView = nullptr;
        mRender = nullptr;
    }
    void Renderer::OnFinishLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
    {
        if (is_main_frame) {
            mLoadComplete = true;
        }
    }
    void Renderer::OnFailLoading(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const String& url, const String& description, const String& error_domain, int error_code)
    {
        Babel::Logger::Get()->log(description.utf8().data());
    }
    void Renderer::OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
    {
        RefPtr<JSContext> context = caller->LockJSContext();
        SetJSContext(context->ctx());
        JSObject global = JSGlobalObject();
        mCommunicator->RegisterJSApi(global);
    }

    void Renderer::LogMessage(ultralight::LogLevel log_level, const ultralight::String& message)
    {
        Babel::Logger::Get()->log(message.utf8().data());
    }
    ultralight::RefPtr<ultralight::View> Renderer::OnCreateInspectorView(ultralight::View* caller, bool is_local, const ultralight::String& inspected_url)
    {
        ViewConfig view_config;
        view_config.initial_device_scale = 1.0;
        view_config.font_family_standard = "Arial";
        view_config.is_accelerated = false;
        view_config.is_transparent = true;
        mInspectorView = mRender->CreateView(mInspectorWidth, mInspectorHeight, view_config, nullptr);
        return mInspectorView;
    }
    void Renderer::RenderFrame()
    {
        mRender->Render();
    }
    void Renderer::PerformUpdate()
    {
        mRender->Update();
    }

    ultralight::BitmapSurface* Renderer::GetSurface()
    {
        return (BitmapSurface*)mView->surface();
    }

    ultralight::BitmapSurface* Renderer::GetInspectorSurface()
    {
        if (!mInspectorView) return nullptr;
        return (BitmapSurface*)mInspectorView->surface();
    }

    void Renderer::SendMouseEvent(int mouseX, int mouseY, uint8_t evtType, uint8_t button)
    {
        ultralight::MouseEvent mouseEvent;
        mouseEvent.type = (MouseEvent::Type)evtType;
        mouseEvent.x = mouseX;
        mouseEvent.y = mouseY;
        mouseEvent.button = (MouseEvent::Button)button;
        mView->FireMouseEvent(mouseEvent);
    }

    void Renderer::SendInpectorMouseEvent(int mouseX, int mouseY, uint8_t evtType, uint8_t button)
    {
        if (!mInspectorView) return;
        ultralight::MouseEvent mouseEvent;
        mouseEvent.type = (MouseEvent::Type)evtType;
        mouseEvent.x = mouseX;
        mouseEvent.y = mouseY;
        mouseEvent.button = (MouseEvent::Button)button;
        mInspectorView->FireMouseEvent(mouseEvent);
    }

    void Renderer::EnableInspector(int width, int height)
    {
        if (!mInspectorView)
        {
            mInspectorWidth = width;
            mInspectorHeight = height;
            mView->CreateLocalInspectorView();
        }
    }

    void Renderer::SendKeyEvent(ultralight::KeyEvent& evt, bool isInspectorEvent)
    {
        if (isInspectorEvent && mInspectorView.get() != nullptr)
        {
            mInspectorView->FireKeyEvent(evt);
        }
        else
        {
            if (mView->HasInputFocus())
            {
                mView->FireKeyEvent(evt);
            }
        }
    }
}
