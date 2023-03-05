#include "Communicator.hpp"
#include <locale>
#include <codecvt>
#include <Ultralight/Ultralight.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include "SharedMemory/EventBuffer.hpp"
#include "Application.hpp"
#include "Renderer.h"
#include "Utils/IniReader.h"
#include "Utils/FileUtils.h"
#include "Utils/Encoding.h"
#include "AoResources/Resources.hpp"

namespace Babel
{
	namespace {
		std::string ascii_to_utf8(const std::string& ascii_string) 
		{
			int num_wchars = MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, NULL, 0);
			std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
			MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, wide_string.get(), num_wchars);

			int num_chars = WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
			std::unique_ptr<char[]> utf8_string(new char[num_chars]);
			WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, utf8_string.get(), num_chars, NULL, NULL);

			return std::string(utf8_string.get());
		}

		std::string utf8_to_ascii(const std::string& utf8_string) 
		{
			int num_wchars = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, NULL, 0);
			std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
			MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, wide_string.get(), num_wchars);

			int num_chars = WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
			std::unique_ptr<char[]> ascii_string(new char[num_chars]);
			WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, ascii_string.get(), num_chars, NULL, NULL);

			return std::string(ascii_string.get());
		}

		void SetObjectString(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, const char* value)
		{			
			JSRetainPtr<JSStringRef> valueStr =
				adopt(JSStringCreateWithUTF8CString(value));
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			auto jvalue = JSValueMakeString(ctx, valueStr.get());
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
		}
		template<typename T>
		void SetObjectNumber(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, T& value)
		{
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			auto jvalue = JSValueMakeNumber(ctx, value);
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
		}

		void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, JSObjectRef& value)
		{
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), value, 0, nullptr);
		}

		void SetGrhJsObject(JSContextRef& ctx, JSObjectRef& objectRef, const AO::GrhDetails& grhData)
		{
			SetObjectNumber(ctx, objectRef, "imageNumber", grhData.ImageNumber);
			SetObjectNumber(ctx, objectRef, "startX", grhData.StartPos.X);
			SetObjectNumber(ctx, objectRef, "startY", grhData.StartPos.Y);
			SetObjectNumber(ctx, objectRef, "width", grhData.EndPos.X);
			SetObjectNumber(ctx, objectRef, "height", grhData.EndPos.Y);
		}
	}
	using namespace ultralight;
	Communicator::Communicator(EventBuffer& eventBuffer, Renderer& renderer, Application& application) 
		: mEventBuffer(eventBuffer), mRenderer(renderer), mApplication(application)
	{
		mResources = std::make_unique<AO::Resources>();
	}

	void Communicator::RegisterJSApi(ultralight::JSObject& global)
	{
		JSObject Api;
		Api["Login"] = BindJSCallbackWithRetval(&Communicator::LogIn);
		Api["CloseClient"] = BindJSCallback(&Communicator::CloseClient);
		Api["GetCredentials"] = BindJSCallbackWithRetval(&Communicator::GetStoredCredentials);
		Api["CreateAccount"] = BindJSCallbackWithRetval(&Communicator::CreateAccount);
		Api["ResendValidationCode"] = BindJSCallbackWithRetval(&Communicator::ResendValidationCode);
		Api["ValidateCode"] = BindJSCallbackWithRetval(&Communicator::ValidateCode);
		Api["SetHost"] = BindJSCallbackWithRetval(&Communicator::SetHost);
		Api["RequestPasswordReset"] = BindJSCallbackWithRetval(&Communicator::RequestPasswordReset);
		Api["NewPasswordRequest"] = BindJSCallbackWithRetval(&Communicator::NewPasswordRequest);
		Api["GetCharacterDrawInfo"] = BindJSCallbackWithRetval(&Communicator::GetCharacterDrawInfo);
		Api["SelectCharacter"] = BindJSCallbackWithRetval(&Communicator::SelectCharacter);
		Api["LoginCharacter"] = BindJSCallbackWithRetval(&Communicator::LoginCharacter);
		Api["GetHeadDrawInfo"] = BindJSCallbackWithRetval(&Communicator::GetHeadDrawInfo);
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
			case EventType::ErrorMessage:
			{
				const ErrorMessageEvent& errorData = static_cast<const ErrorMessageEvent&>(eventData);
				SendErrorMessage(errorData);
				break;
			}
			case EventType::Close:
				mApplication.Stop();
				break;
			case EventType::SetActiveScreen:
			{
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
				auto size = output - (char*)(&eventData);
				std::string name(strInfo[0].StartPos, strInfo[0].Size);
				SetActiveScreen(name);
			}
				break;
			case EventType::SetLoadingMessage:
			{
				const LoadingMessage& loadingData = static_cast<const LoadingMessage&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(LoadingMessage), strInfo);
				auto size = output - (char*)(&eventData);
				std::string message(strInfo[0].StartPos, strInfo[0].Size);
				SetLoadingMessage(message, loadingData.Localize);
			}
			break;
			case EventType::LoginCharList:
			{
				const CharacterListEvent& charListEvent = static_cast<const CharacterListEvent&>(eventData);
				HandleLoginCharList(charListEvent);
			}
			break;
			default:
				break;
		}
	}

	ultralight::JSValue Communicator::LogIn(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 3)
		{
			return "invalid params";
		}
		ultralight::String user = args[0];
		ultralight::String password = args[1];
		bool storeCredentials = args[2];

		LoginCredentialsEvent loginEvent;
		auto usr = user.utf8();
		auto pwd = password.utf8();
		loginEvent.SetUserAndPassword(usr.data(), static_cast<int>((int)usr.length()), pwd.data(), (int)pwd.length());
		loginEvent.storeCredentials = storeCredentials;
		loginEvent.Size = sizeof(LoginCredentialsEvent);
		loginEvent.EventType = EventType::Login;
		mEventBuffer.AddEvent((uint8_t*)&loginEvent, loginEvent.Size);
		return JSValue(true);
	}
	ultralight::JSValue Communicator::CreateAccount(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 4)
		{
			return "invalid params";
		}
		ultralight::String user = args[0];
		ultralight::String password = args[1];
		ultralight::String uname = args[2];
		ultralight::String usurname = args[3];

		NewAccountEvent newAccountEvent;
		auto usr = user.utf8();
		auto pwd = password.utf8();
		auto name = uname.utf8();
		auto surname = usurname.utf8();
		newAccountEvent.SetUserAndPassword(usr.data(), static_cast<int>((int)usr.length()), pwd.data(), (int)pwd.length(),
										  name.data(), (int)name.length(), surname.data(), (int)surname.length());
		newAccountEvent.Size = sizeof(NewAccountEvent);
		newAccountEvent.EventType = EventType::CreateAccount;
		mEventBuffer.AddEvent((uint8_t*)&newAccountEvent, newAccountEvent.Size);
		return JSValue(true);
	}
	void Communicator::CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		Event closeClient;
		closeClient.EventType = EventType::CloseClient;
		closeClient.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&closeClient, closeClient.Size);
	}
	ultralight::JSValue Communicator::GetStoredCredentials(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		auto path = GetFilePath("OUTPUT/Cuenta.ini");
		INIReader Reader(path.u8string());
		auto account = Reader.Get("CUENTA", "Nombre", "");
		auto password = Reader.Get("CUENTA", "Password", "");
		password = Decode(password, "9256");

		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSRetainPtr<JSStringRef> user =
			adopt(JSStringCreateWithUTF8CString(account.c_str()));
		JSRetainPtr<JSStringRef> pwd =
			adopt(JSStringCreateWithUTF8CString(password.c_str()));
		JSRetainPtr<JSStringRef> userParam =
			adopt(JSStringCreateWithUTF8CString("user"));
		JSRetainPtr<JSStringRef> pwdParam =
			adopt(JSStringCreateWithUTF8CString("password"));
		auto juser = JSValueMakeString(ctx, user.get());
		auto jpwd = JSValueMakeString(ctx, pwd.get());
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectSetProperty(ctx, ret, userParam.get(), juser, 0, nullptr);
		JSObjectSetProperty(ctx, ret, pwdParam.get(), jpwd, 0, nullptr);
		return ret;
	}
	ultralight::JSValue Communicator::ResendValidationCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String email = args[0];
		auto usr = utf8_to_ascii(email.utf8().data());
		
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::ResendValidationCode;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = usr.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue Communicator::ValidateCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return "invalid params";
		}
		ultralight::String jemail = args[0];
		ultralight::String jcode = args[1];

		auto usr = utf8_to_ascii(jemail.utf8().data());
		auto code = utf8_to_ascii(jcode.utf8().data());

		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::ValidateCode;
		std::vector<StringInBuffer> strInfo(2);
		strInfo[0].StartPos = usr.c_str();
		strInfo[1].StartPos = code.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue Communicator::SetHost(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String jenv= args[0];
		auto envornment = utf8_to_ascii(jenv.utf8().data());
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::SetHost;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = envornment.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue Communicator::RequestPasswordReset(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String email = args[0];

		auto usr = utf8_to_ascii(email.utf8().data());
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::RequestPasswordReset;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = usr.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue Communicator::NewPasswordRequest(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 3)
		{
			return "invalid params";
		}
		ultralight::String jemail = args[0];
		ultralight::String jcode = args[1];
		ultralight::String jpassword = args[2];

		auto usr = utf8_to_ascii(jemail.utf8().data());
		auto code = utf8_to_ascii(jcode.utf8().data());
		auto pwd = utf8_to_ascii(jpassword.utf8().data());

		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::NewPasswordRequest;
		std::vector<StringInBuffer> strInfo(3);
		strInfo[0].StartPos = usr.c_str();
		strInfo[1].StartPos = code.c_str();
		strInfo[2].StartPos = pwd.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue Communicator::GetCharacterDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 5)
		{
			return "invalid params";
		}

		int body = args[0];
		int head = args[1];
		int helm = args[2];
		int shield = args[3];
		int weapon = args[4];
		AO::CharacterRenderInfo charData;
		mResources->GetBodyInfo(charData, body, head, helm, shield, weapon);

		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSObjectRef character = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef bodyObj = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef bodyGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef headGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef helmGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef waponGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef shieldGrh = JSObjectMake(ctx, nullptr, nullptr);
		SetGrhJsObject(ctx, bodyGrh, charData.Body.image);
		SetChildObject(ctx, bodyObj, "body", bodyGrh);
		SetObjectNumber(ctx, bodyObj, "HeadOffsetY", charData.Body.HeadOffset.Y);
		SetObjectNumber(ctx, bodyObj, "HeadOffsetX", charData.Body.HeadOffset.X);
		SetChildObject(ctx, character, "body", bodyObj);
		SetGrhJsObject(ctx, headGrh, charData.Head);
		SetGrhJsObject(ctx, helmGrh, charData.Helm);
		SetGrhJsObject(ctx, waponGrh, charData.Weapon);
		SetGrhJsObject(ctx, shieldGrh, charData.Shield);
		SetChildObject(ctx, character, "head", headGrh);
		SetChildObject(ctx, character, "helm", helmGrh);
		SetChildObject(ctx, character, "weapon", waponGrh);
		SetChildObject(ctx, character, "shield", shieldGrh);
		return character;
	}

	ultralight::JSValue Communicator::GetHeadDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		AO::GrhDetails headInfo;

		mResources->GetHeadInfo(headInfo, args[0]);
		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSObjectRef headGrh = JSObjectMake(ctx, nullptr, nullptr);
		SetGrhJsObject(ctx, headGrh, headInfo);
		return headGrh;
	}

	ultralight::JSValue Communicator::SelectCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		SelectCharacterEvent selectCharEvent;
		selectCharEvent.CharIndex = args[0];
		selectCharEvent.EventType = EventType::SelectCharacter;
		selectCharEvent.Size = sizeof(SelectCharacterEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectCharEvent, sizeof(selectCharEvent));
		return ultralight::JSValue();
	}

	ultralight::JSValue Communicator::LoginCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		SelectCharacterEvent selectCharEvent;
		selectCharEvent.CharIndex = args[0];
		selectCharEvent.EventType = EventType::LoginCharacter;
		selectCharEvent.Size = sizeof(SelectCharacterEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectCharEvent, sizeof(selectCharEvent));
		return ultralight::JSValue();
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
	void Communicator::SendErrorMessage(const ErrorMessageEvent& messageData)
	{
		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.ErrorMessage"));

		// Evaluate the string "ShowMessage"
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func)) 
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {
				std::string utf8_string = ascii_to_utf8(messageData.StrData);
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(utf8_string.c_str()));

				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()),
											JSValueMakeNumber(ctx, static_cast<double>(messageData.MessageType)),
											JSValueMakeNumber(ctx, static_cast<double>(messageData.Action)) };

				// Count the number of arguments in the array.
				size_t num_args = sizeof(args) / sizeof(JSValueRef*);

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void Communicator::SetActiveScreen(const std::string& name)
	{
		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetActiveDialog"));

		// Evaluate the string "ShowMessage"
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(name.c_str()));

				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()) };

				// Count the number of arguments in the array.
				size_t num_args = 1;

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void Communicator::SetLoadingMessage(const std::string& message, bool localize)
	{
		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetLoadingMessage"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(ascii_to_utf8(message).c_str()));
				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()),
											JSValueMakeBoolean(ctx, localize)};

				// Count the number of arguments in the array.
				size_t num_args = 1;

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void Communicator::HandleLoginCharList(const CharacterListEvent& messageData)
	{
		Ref<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context.get();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetCharacter"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.
				
				for (int i = 0; i < messageData.CharacterCount; i++)
				{
					JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
					SetObjectString(ctx, ret, "name", ascii_to_utf8(messageData.CharacterList[i].Name).c_str());
					SetObjectNumber(ctx, ret, "head", messageData.CharacterList[i].Head);
					SetObjectNumber(ctx, ret, "body", messageData.CharacterList[i].Body);
					SetObjectNumber(ctx, ret, "helm", messageData.CharacterList[i].Helm);
					SetObjectNumber(ctx, ret, "shield", messageData.CharacterList[i].Shield);
					SetObjectNumber(ctx, ret, "weapon", messageData.CharacterList[i].Weapon);
					SetObjectNumber(ctx, ret, "level", messageData.CharacterList[i].Level);
					SetObjectNumber(ctx, ret, "status", messageData.CharacterList[i].Status);
					SetObjectNumber(ctx, ret, "index", messageData.CharacterList[i].Index);
					const JSValueRef args[] = { ret };
					size_t num_args = 1;
					JSValueRef exception = 0;
					JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
						num_args, args,
						&exception);
				}
			}
		}
	}
}