#include "EventHandler.hpp"
#include "../EventBuffer.hpp"

namespace Babel
{
	EventHandler::EventHandler(EventListener& listener, EventBuffer& eventBuffer) : mEventBuffer(eventBuffer), mEventListener(listener)
	{
		mLocalBuffer.resize(eventBuffer.GetBufferSize());
	}

	void EventHandler::HandleIncomingEvents()
	{
		auto eventCount = mEventBuffer.GetAviableEvents(mLocalBuffer.data(), mLocalBuffer.size());
		while (eventCount > 0)
		{
			uint8_t* next = (uint8_t*)mLocalBuffer.data();
			Event* evData = (Event*)next;
			mEventListener.HandleEvent(*evData);
			next += evData->Size;
			eventCount--;
		}
	}
}
