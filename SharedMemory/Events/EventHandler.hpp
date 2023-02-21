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
		CloseClient,
		ErrorMessage,
		SetActiveScreen,
		CreateAccount,
		ResendValidationCode,
		ValidateCode,
		ValidatePrevCode,
		SetHost,
		RequestPasswordReset,
		NewPasswordRequest,
		SetLoadingMessage
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
		int storeCredentials;
		int UserSize;
		int PasswordSize;
		char strData[255];
		
		void SetUserAndPassword(const char *user, int userSize, const char* password, int passwordSize);
	};

	struct NewAccountEvent : public Event
	{
		int UserSize;
		int PasswordSize;
		int NameSize;
		int SurnameSize;
		char strData[512];

		void SetUserAndPassword(const char* user, int userSize, const char* password, int passwordSize,
								const char* userName, int userNameSize, const char* surname, int surnameSize);
	};

	struct ErrorMessageEvent : public Event
	{
		int action;
		int messageType;
		int size;
		char strData[512];
	};

	struct LoadingMessage : public Event
	{
		bool localize;
	};

	struct StringInBuffer
	{
		int32_t Size;
		const char* StartPos;
	};
	const char* GetStringPtrInEvent(const char* memPtr, int32_t eventSize, std::vector<StringInBuffer>& result);
	int32_t PrepareDynamicStrings(std::vector<StringInBuffer>& result, size_t maxSize = 255);

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
