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
		ReturnToLogin,
		RequestDeleteCharacter,
		RequestDeleteCode,
		RemoveCharacterFromList,
		ConfirmDeleteCharacter,
		RequestTransferCharacter,
		UpdateCharacterStats,
		SetUserName,
		SendChatMessage, // send chat info from vb to the message list
		SendConsoleMsgToVB, // send user input from chat input
		ShowVBDialog, // to display vb dialog by name
		UpdateFps,
		SetInvLevel, //configures bonus slot in invetory
		UpdateInvSlot, // vb update inventory slot info
		UpdateSpellSlot,
		SelectInvSlot, //js select an item from inventory
		UseInvSlot,
		SelectSpellSlot,
		UseSpellSlot,
		UpdateInputFocus, // informs vb we have input focus so we don't move while writting
		SendScrollEvent,
		UpdateHp,
		UpdateMana,
		UpdateStamina,
		UpdateDrink,
		UpdateFood,
		UpdateGold,
		UpdateExp,
		ClickLink,
		ClickGold,
		OpenChat,
		UpdateStrAndAgi,
		UpdateMapName,
		UpdateMapNpc,
		UpdateUserPos,
		UpdateGroupMemberPos,
		MoveInvSlot,
		RequestAction,
		UpdateKeySlot,
		UseKey,
		UpdateIntervals,
		StartInterval,
		MoveSpellSlot,
		UpdateSafeState,
		DeleteItem,
		UpdateOnlines,
		UpdateGameTime,
		UpdateIsGameMaster,
		UpdateMagicResistance,
		UpdateMagicAttack,
		UpdateOpenDialog,
		SetWhisperTarget,
		PasteText,
		ScrollSpell,
		ReloadSettings,
		SetRemoteTrackingState,
		RemoteInvSpellState,
		RemoteUserClick,
		UpdateRemoteMousePos,
		TeleportToMinimapPos,
		StartSpellCd,
		UpdateCombatAndglobalChatSettings,
		StartStunTime,
		VBUpdateHotkeySlot,
		JSUpdateHotkeySlot,
		ActivateFeatureToggle,
		ClearToggles,
		UpdateHideHotkeyState,
		SendQuestionResponse,
		ShowQuestion,
		OpenMerchant,
		UpdateMerchantSlot,
		MoveMerchantSlot,
		CloseMerchant,
		ButItem,
		SellItem,
		OpenAoShop,
		BuyAOShopItem,
		UpdateIntSetting,
		OpenLobbyList,
		UpdateLobbyInfo,
		CreateNewScenario,
		JoinScenario
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
		int16_t ShiftState;
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
		int Class;
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

	struct SingleIntEvent : public Event
	{
		int Value;
	};
	struct DoubleIntEvent : public Event
	{
		int Value1;
		int Value2;
	};

	struct TripleIntEvent : public Event
	{
		int Value1;
		int Value2;
		int Value3;
	};

	struct SingleBoolEvent : public Event
	{
		bool Value;
	};

	struct StringInBuffer
	{
		int32_t Size;
		const char* StartPos;
	};
	const char* GetStringPtrInEvent(const char* memPtr, int32_t eventSize, std::vector<StringInBuffer>& result);
	int32_t PrepareDynamicStrings(std::vector<StringInBuffer>& result, size_t maxSize = 255);

	template<typename T>
	const char* GetArrayData(const char* memPtr, int32_t eventSize, int32_t& size, T** dataStart)
	{
		memPtr += eventSize;
		size = *((int32_t*)(memPtr));
		memPtr += sizeof(int32_t);
		*dataStart = (T*)memPtr;
		memPtr += size * sizeof(T);
		return memPtr;
	}

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
