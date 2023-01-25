#pragma once
#include <cstdint>
#include <vector>


namespace Babel
{
	enum class EventType : uint32_t
	{
		MouseData,
		KeyData,
		Close,
		DebugMouseData,
		EnableDebugWindow,
		Login,
		CloseClient
	};

	struct Event
	{
		int32_t Size;
		EventType EventType;
	};
	

	struct MouseData : public Event
	{
		uint8_t TypeFlags;
		int PosX;
		int PosY;
	};

	struct KeyEvent : public Event
	{
		int16_t KeyCode;
		bool CapsState;
		bool ShiftState;
		int8_t Type;
		bool Inspector;
	};

	struct WindowInfo : public Event
	{
		int Width;
		int Height;
	};

	struct LoginInfoEvent : public Event
	{
		int userSize;
		int passwordSize;
		char strData[255];
		
		void SetUserAndPassword(const char *user, int userSize, const char* password, int passwordSize);
	};

	class EventListener
	{
	public:
		virtual void HandleEvent(const Event& eventData) = 0;
	};
	class EventBuffer;

	class EventHandler
	{
	public:
		EventHandler(EventListener& listener, EventBuffer& eventBuffer);
		void HandleIncomingEvents();
	private:
		EventBuffer& mEventBuffer;
		EventListener& mEventListener;
		std::vector<uint8_t> mLocalBuffer;
	};
}
