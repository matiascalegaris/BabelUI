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

	class EventListener
	{
	public:
		virtual void HandleEvent(const Event& evenData) = 0;
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
