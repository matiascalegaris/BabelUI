#include "Communicator.hpp"

#include <Ultralight/Ultralight.h>
#include "SharedMemory/EventBuffer.hpp"
#include "Application.hpp"
#include "Renderer.h"

namespace Babel
{
	using namespace ultralight;
	Communicator::Communicator(EventBuffer& eventBuffer, Renderer& renderer, Application& application) 
		: mEventBuffer(eventBuffer), mRenderer(renderer), mApplication(application)
	{
	}

	void Communicator::RegisterJSApi(ultralight::JSObject& global)
	{
		JSObject Api;
		Api["Login"] = BindJSCallbackWithRetval(&Communicator::LogIn);
		Api["CloseClient"] = BindJSCallback(&Communicator::CloseClient);
		global["BabelUI"] = JSValue(Api);
	}
	void Communicator::HandleEvent(const Event& eventData)
	{
		switch (eventData.EventType)
		{
			case EventType::MouseData:
			{
				const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
				mRenderer.SendMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
				break;
			}
			case EventType::DebugMouseData:
			{
				const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
				mRenderer.SendInpectorMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
				break;
			}
			case EventType::EnableDebugWindow:
			{
				const WindowInfo& windowEvtdata = static_cast<const WindowInfo&>(eventData);
				mApplication.EnableDebugWindow(windowEvtdata.Width, windowEvtdata.Height);
				break;
			}
			case EventType::KeyData:
			{
				const KeyEvent& keyData = static_cast<const KeyEvent&>(eventData);
				HandlekeyData(keyData);
				break;
			}
			case EventType::Close:
				mApplication.Stop();
			default:
				break;
		}
	}

	ultralight::JSValue Communicator::LogIn(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return "invalid params";
		}
		ultralight::String user = args[0];
		ultralight::String password = args[1];

		LoginInfoEvent loginEvent;
		auto usr = user.utf8();
		auto pwd = password.utf8();
		loginEvent.SetUserAndPassword(usr.data(), static_cast<int>(usr.length()), pwd.data(), pwd.length());
		loginEvent.Size = sizeof(LoginInfoEvent);
		loginEvent.EventType = EventType::Login;
		mEventBuffer.AddEvent((uint8_t*)&loginEvent, loginEvent.Size);
		return JSValue(true);
	}
	void Communicator::CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		Event closeClient;
		closeClient.EventType = EventType::CloseClient;
		closeClient.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&closeClient, closeClient.Size);
	}
	void Communicator::HandlekeyData(const KeyEvent& keyData)
	{
		ultralight::KeyEvent evt;
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
			mRenderer.SendKeyEvent(evt, keyData.Inspector);
			break;
		case ultralight::KeyEvent::kType_Char:
		{
			char key = static_cast<char>(keyData.KeyCode);
			evt.text = std::string(1, key).c_str();
			evt.unmodified_text = evt.text;
			mRenderer.SendKeyEvent(evt, keyData.Inspector);
		}
		default:
			break;
		}
	}
}