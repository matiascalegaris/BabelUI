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
		void CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
	private: //c++ events functions
		void HandlekeyData(const KeyEvent& keyData);
	private:
		EventBuffer& mEventBuffer;
		Renderer& mRenderer;
		Application& mApplication;
	};
}