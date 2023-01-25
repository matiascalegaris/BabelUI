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
		uint8_t* next = (uint8_t*)mLocalBuffer.data();
		while (eventCount > 0)
		{
			Event* evData = (Event*)next;
			mEventListener.HandleEvent(*evData);
			next += evData->Size;
			eventCount--;
		}
	}
	void LoginInfoEvent::SetUserAndPassword(const char* user, int userSize, const char* password, int passwordSize)
	{
		this->userSize = userSize;
		this->passwordSize = passwordSize;
		memcpy_s(strData, sizeof(strData), user, userSize);
		memcpy_s(strData+userSize, sizeof(strData)-userSize, password, passwordSize);
	}
}
