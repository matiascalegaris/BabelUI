#include "Renderer.h"
#include <thread>
#include <chrono>
#include <filesystem>

const char* htmlString();

namespace Babel
{
    namespace {
        std::filesystem::path GetWorkingPath()
        {
            auto path = std::filesystem::current_path();
            path += "/../Recursos/BabelUI/";
            return path;
        }

        std::filesystem::path GetFilePath(const char* fileName)
        {
            auto path = GetWorkingPath();
            path += fileName;
            return path;
        }

        std::wstring LocalPathForFile(const char* fileName)
        {
            auto path =  GetFilePath(fileName).native();
            return std::wstring(L"file:///") + path;
        }
    }
    using namespace ultralight;
	Renderer::Renderer(int width, int height)
	{
        ///
    /// Setup our config. The config can be used to customize various
    /// options within the renderer and WebCore module.
    ///
    /// Our config uses 2x DPI scale and "Arial" as the default font.
    ///
        Config config;
        config.device_scale = 1.0;
        config.font_family_standard = "Arial";

        ///
        /// We need to tell config where our resources are so it can load our
        /// bundled certificate chain and make HTTPS requests.
        ///
        config.resource_path = "../Recursos/BabelUI/";
        //config.resource_path = "D:\\Proyectos/ao20/Recursos/BabelUI/";

        ///
        /// Make sure the GPU renderer is disabled so we can render to an offscreen
        /// pixel buffer surface.
        ///
        config.use_gpu_renderer = false;

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
        Platform::instance().set_file_system(GetPlatformFileSystem("."));

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

        ///
        /// Create our View.
        ///
        /// Views are sized containers for loading and displaying web content.
        ///
        mView = mRender->CreateView(width, height, true, nullptr);

        ///
        /// Register our MyApp instance as a load listener so we can handle the
        /// View's OnFinishLoading event below.
        ///
        mView->set_load_listener(this);

        ///
        /// Load a string of HTML into our View. (For code readability, the string
        /// is defined in the htmlString() function at the bottom of this file)
        ///
        /// **Note**:
        ///   This operation may not complete immediately-- we will call
        ///   Renderer::Update continuously and wait for the OnFinishLoading event
        ///   before rendering our View.
        ///
        /// Views can also load remote URLs, try replacing the code below with:
        ///
        ///    view_->LoadURL("https://en.wikipedia.org");
        ///
        auto fullPath = LocalPathForFile("/index.html");
        mView->LoadURL({fullPath.c_str(), fullPath.length()});
        //mView->LoadURL("file:///D:\\Proyectos/ao20/Recursos/BabelUI/test2.html");
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
    }
    void Renderer::OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ultralight::String& url)
    {
        Ref<JSContext> context = caller->LockJSContext();
        SetJSContext(context.get());
        JSObject global = JSGlobalObject();
        global["GetMessage"] = BindJSCallbackWithRetval(&Renderer::GetMessage);
    }
    void Renderer::LogMessage(ultralight::LogLevel log_level, const ultralight::String16& message)
    {
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
        return (BitmapSurface*)mView->surface();;
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

    ultralight::JSValue Renderer::GetMessage(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
    {
        return JSValue("Test input");
    }
}

const char* htmlString() {
    return R"(
   <html>
  <head>
    <style type="text/css">
      * { -webkit-user-select: none; }
      body { 
        font-family: -apple-system, 'Segoe UI', Ubuntu, Arial, sans-serif; 
        text-align: center;
        background: linear-gradient(#FFF, #DDD);
        padding: 2em;
      }
      body.rainbow {
        background: linear-gradient(90deg, #ff2363, #fff175, #68ff9d, 
                                           #45dce0, #6c6eff, #9e23ff, #ff3091);
        background-size: 1000% 1000%;
        animation: ScrollGradient 10s ease infinite;
      }
      @keyframes ScrollGradient {
        0%   { background-position:0% 50%; }
        50%  { background-position:100% 50%; }
        100% { background-position:0% 50%; }
      }
      #message {
        padding-top: 2em;
        color: white;
        font-weight: bold;
        font-size: 24px;
        text-shadow: 1px 1px rgba(0, 0, 0, 0.4);
      }
    </style>
    <script type="text/javascript">
    function HandleButton(evt) {
      // Call our C++ callback 'GetMessage'
      //var message = GetMessage();
      
      // Display the result in our 'message' div element and apply the
      // rainbow effect to our document's body.
      document.getElementById('message').innerHTML = "hardcoded message";
      document.body.classList.add('rainbow');
    }
    </script>
  </head>
  <body>
    <div> 
    <object type="text/html" data="https://www.ao20.com.ar/wiki" width="100%" height="100%" style="overflow:auto;border:5px ridge blue">
    </object>
 </div>
  </body>
</html>
    )";
}