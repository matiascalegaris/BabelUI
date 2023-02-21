#pragma once
#include <AppCore/JSHelpers.h>
#include "SharedMemory/Events/EventHandler.hpp"

namespace Babel
{
	class Renderer;
	class Application;
	// handles the comunication between c++ and js
	class Communicator : public EventListener
	{
	public:
		Communicator(EventBuffer& eventBuffer, Renderer& renderer, Application& application);

		void RegisterJSApi(ultralight::JSObject& global);
		void HandleEvent(const Event& eventData) override;
	private: //js callbacks
		ultralight::JSValue LogIn(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue CreateAccount(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetStoredCredentials(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue ResendValidationCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue ValidateCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue SetHost(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue RequestPasswordReset(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue NewPasswordRequest(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
	private: //c++ events functions
		void HandlekeyData(const KeyEvent& keyData);
		void SendErrorMessage(const ErrorMessageEvent& messageData);
		void SetActiveScreen(const std::string& name);
		void SetLoadingMessage(const std::string& message, bool localize);
	private:
		EventBuffer& mEventBuffer;
		Renderer& mRenderer;
		Application& mApplication;
	};
}