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
		auto eventCount = mEventBuffer.GetAviableEvents(mLocalBuffer.data(), (int32_t)mLocalBuffer.size());
		uint8_t* next = (uint8_t*)mLocalBuffer.data();
		while (eventCount > 0)
		{
			Event* evData = (Event*)next;
			mEventListener.HandleEvent(*evData);
			next += evData->Size;
			eventCount--;
		}
	}

	void LoginCredentialsEvent::SetUserAndPassword(const char* user, int userSize, const char* password, int passwordSize)
	{
		this->UserSize = userSize;
		this->PasswordSize = passwordSize;
		memcpy_s(strData, sizeof(strData), user, userSize);
		memcpy_s(strData + userSize, sizeof(strData)-userSize, password, passwordSize);
	}

	void NewAccountEvent::SetUserAndPassword(const char* user, int userSize, const char* password, int passwordSize,
											 const char* userName, int userNameSize, const char* surname, int surnameSize)
	{
		this->UserSize = userSize;
		this->PasswordSize = passwordSize;
		this->NameSize = userNameSize;
		this->SurnameSize = surnameSize;
		int32_t offset = 0;
		memcpy_s(strData, sizeof(strData), user, userSize);
		offset += userSize;
		memcpy_s(strData + offset, sizeof(strData) - offset, password, passwordSize);
		offset += passwordSize;
		memcpy_s(strData + offset, sizeof(strData) - offset, userName, userNameSize);
		offset += userNameSize;
		memcpy_s(strData + offset, sizeof(strData) - offset, surname, passwordSize);
	}

	const char* GetStringPtrInEvent(const char* memPtr, int32_t eventSize, std::vector<StringInBuffer>& result)
	{
		memPtr += eventSize;
		for (int i = 0; i < result.size(); i++)
		{
			result[i].Size = *((int32_t*)(memPtr));
			memPtr += sizeof(int32_t);
			result[i].StartPos = memPtr;
			memPtr += result[i].Size;
		}
		return memPtr;
	}

	int32_t PrepareDynamicStrings(std::vector<StringInBuffer>& result, size_t maxSize)
	{
		int32_t totalSize = 0;
		for (int i = 0; i < result.size(); i++)
		{
			result[i].Size = strnlen(result[i].StartPos, maxSize);
			totalSize += result[i].Size + sizeof(int32_t);
		}
		return totalSize;
	}

}
