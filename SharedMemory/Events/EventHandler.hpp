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
		SetLoadingMessage,
		LoginCharList,
		SelectCharacter,
		LoginCharacter,
		CreateCharacter,
		ReturnToLogin
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

	struct LoginCredentialsEvent : public Event
	{
		int storeCredentials;
		int UserSize;
		int PasswordSize;
		char StrData[255];		
		void SetUserAndPassword(const char *user, int userSize, const char* password, int passwordSize);
	};

	struct CharacterInfo
	{
		char Name[255];
		int Head;
		int Body;
		int Helm;
		int Shield;
		int Weapon;
		int Level;
		int Status;
		int Index;
	};

	struct CharacterListEvent : public Event
	{
		int CharacterCount;
		CharacterInfo CharacterList[10];
	};

	struct NewAccountEvent : public Event
	{
		int UserSize;
		int PasswordSize;
		int NameSize;
		int SurnameSize;
		char StrData[512];

		void SetUserAndPassword(const char* user, int userSize, const char* password, int passwordSize,
								const char* userName, int userNameSize, const char* surname, int surnameSize);
	};

	struct ErrorMessageEvent : public Event
	{
		int Action;
		int MessageType;
		int StrSize;
		char StrData[512];
	};

	struct LoadingMessage : public Event
	{
		bool Localize;
	};

	struct SelectCharacterEvent : public Event
	{
		int CharIndex;
	};

	struct NewCharacterEvent : public Event
	{
		int Head;
		int Gender;
		int Race;
		int Class;
		int HomeCity;
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
