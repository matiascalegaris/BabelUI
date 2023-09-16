#include "dllmain.h"
#include <Windows.h>
#include <chrono>
#include "Core/Tunnel.h"
#include "SharedMemory/Events/GameplayEvents.hpp"
#include "Core/Logger.hpp"
#include "CallbackDefines.hpp"
#include "AOToolsApi.h"
#include "CommonDefines.hpp"


namespace {
	Babel::Tunnel BabelTunnel;

	Babel::CharacterListEvent LoginCharList;

	void FillMouseData(Babel::MouseData& mouseData, int mouseX, int mouseY, int type, int buttonState)
	{
		mouseData.PosX = mouseX;
		mouseData.PosY = mouseY;
		mouseData.TypeFlags = (type << 4 & 0xf0) | (buttonState & 0x0f);
		mouseData.Size = sizeof(Babel::MouseData);
	}
}

bool InitializeBabel(void* settings)
{
	Babel::Settings* settingsPtr = static_cast<Babel::Settings*>(settings);
	try {
		return BabelTunnel.Initialize(*settingsPtr);
	}
	catch (const std::exception& )
	{
		return false;
	}
}

bool GetImageBuffer(char* buffer, int size)
{
	BabelTunnel.CheckIncommingMessages();
	return BabelTunnel.GetSyncData().GetLastBuffer(buffer, size);
}

void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState)
{
	Babel::MouseData mouseData;
	FillMouseData(mouseData, mouseX, mouseY, type, buttonState);
	mouseData.EventType = Babel::EventType::MouseData;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&mouseData, mouseData.Size);
}

void _stdcall SendScrollEvent(int direction)
{
	Babel::SingleIntEvent scrollData;
	scrollData.Value = direction;
	scrollData.EventType = Babel::EventType::SendScrollEvent;
	scrollData.Size = sizeof(scrollData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&scrollData, scrollData.Size);
}

void _stdcall SendKeyEvent(int16_t keyCode, int16_t shift, int type, bool capsState, bool inspector)
{
	Babel::KeyEvent keyData;
	keyData.KeyCode = keyCode;
	keyData.ShiftState = shift;
	keyData.Type = type;
	keyData.CapsState = capsState;
	keyData.Inspector = inspector;
	keyData.Size = sizeof(Babel::KeyEvent);
	keyData.EventType = Babel::EventType::KeyData;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&keyData, keyData.Size);
}

void _stdcall RegisterCallbacks(int loginCallback, int closeClient, int createAccount, int setHost, int validateCode, int resendValidationCode,
								int requestPasswordReset, int newPasswordRequest, int selectCharacter, int loginCharacter, int returnToLogin, int createCharacter,
								int requestDeleteCharater, int confirmDeleteCharacter, int transferCharacter)
{
	Babel::CallbacksList callbacks;
	callbacks.CloseClient = (Babel::TVoidParam)(closeClient);
	callbacks.Login = (Babel::TLogInCallback)(loginCallback);
	callbacks.CreateAccount = (Babel::TCreateAccountCallback)(createAccount);
	callbacks.SetHost = (Babel::TSetHost)(setHost);
	callbacks.ValidateAccount = (Babel::TValidateAccount)(validateCode);
	callbacks.ResendValidationCode = (Babel::TResendValidationCode)(resendValidationCode);
	callbacks.RequestPasswordReset = (Babel::TRequestPasswordReset)(requestPasswordReset);
	callbacks.NewPasswordRequest = (Babel::TNewPasswordRequest)(newPasswordRequest);
	callbacks.SelectCharacter = (Babel::TSelectCharacter)(selectCharacter);
	callbacks.LoginWithCharacter = (Babel::TLoginCharacterIndex)(loginCharacter);
	callbacks.ReturnToLogin = (Babel::TVoidParam)(returnToLogin);
	callbacks.CreateCharacter = (Babel::TCreateCharacter)(createCharacter);
	callbacks.RequestDeleteCharacter = (Babel::TSelectCharacter)(requestDeleteCharater);
	callbacks.ConfirmDeleteCharacter = (Babel::TIntStringF)(confirmDeleteCharacter);
	callbacks.TransferCharacter = (Babel::TIntStringF)(transferCharacter);
	BabelTunnel.SetCallbacks(callbacks);
}

void _stdcall SendErrorMessage(const char* str, int messageType, int action)
{
	Babel::ErrorMessageEvent message;
	message.Action = action;
	message.MessageType = messageType;
	message.StrSize = strnlen(str, sizeof(message.StrData)-1);
	strncpy_s(message.StrData, sizeof(message.StrData) - 1,str, message.StrSize);
	message.Size = sizeof(Babel::ErrorMessageEvent);
	message.StrData[message.StrSize] = 0;
	message.EventType = Babel::EventType::ErrorMessage;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&message, message.Size);
}

void _stdcall SetActiveScreen(const char* str)
{	
	BabelTunnel.SetActiveScreen(str);
}

void _stdcall SetLoadingMessage(const char* str, int localize)
{
	BabelTunnel.SetLoadingMessage(str, localize);
}

void _stdcall LoginCharacterListPrepare(int characterCount)
{
	LoginCharList.CharacterCount = characterCount;
	memset(&LoginCharList.CharacterList, 0, sizeof(LoginCharList.CharacterList));
}

void _stdcall LoginAddCharacter(const char* name, int head, int body, int helm, int shield, int weapon, int level, int status, int index, int classId)
{
	int len = strnlen(name, sizeof(LoginCharList.CharacterList[index].Name));
	strncpy_s(LoginCharList.CharacterList[index].Name, sizeof(LoginCharList.CharacterList[index].Name) - 1l, name, len);
	LoginCharList.CharacterList[index].Name[sizeof(LoginCharList.CharacterList[index].Name) - 1] = 0;
	LoginCharList.CharacterList[index].Head = head;
	LoginCharList.CharacterList[index].Body = body;
	LoginCharList.CharacterList[index].Helm = helm;
	LoginCharList.CharacterList[index].Shield = shield;
	LoginCharList.CharacterList[index].Weapon = weapon;
	LoginCharList.CharacterList[index].Level = level;
	LoginCharList.CharacterList[index].Status = status;
	LoginCharList.CharacterList[index].Index = index;
	LoginCharList.CharacterList[index].Class = classId;
}

void _stdcall RemoveCharacterFromList(int index)
{
	Babel::SelectCharacterEvent eventData;
	eventData.CharIndex = index - 1;
	eventData.EventType = Babel::EventType::RemoveCharacterFromList;
	eventData.Size = sizeof(eventData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

void _stdcall LoginSendCharacters()
{
	LoginCharList.EventType = Babel::EventType::LoginCharList;
	LoginCharList.Size = sizeof(LoginCharList);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&LoginCharList, LoginCharList.Size);
}

void _stdcall RequestDeleteCode()
{
	Babel::Event eventData;
	eventData.EventType = Babel::EventType::RequestDeleteCode;
	eventData.Size = sizeof(eventData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

uint32_t _stdcall NextPowerOf2(uint32_t value)
{
	value--;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	value++;
	return value;
}

bool _stdcall CreateDebugWindow(int width, int height)
{
	try {
		return BabelTunnel.InitializeDebugWindows(width, height);
	}
	catch (const std::exception&)
	{
		return false;
	}
}

bool _stdcall GetDebugImageBuffer(char* buffer, int size)
{
	return BabelTunnel.GetDebugSyncData().GetLastBuffer(buffer, size);
}
void _stdcall SendDebugMouseEvent(int mouseX, int mouseY, int type, int buttonState)
{
	Babel::MouseData mouseData;
	FillMouseData(mouseData, mouseX, mouseY, type, buttonState);
	mouseData.EventType = Babel::EventType::DebugMouseData;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&mouseData, mouseData.Size);

}

uint32_t _stdcall GetTelemetry(const char* str, const uint8_t* data, uint32_t maxSize)
{
	return GetTelemetryData(str, data, maxSize);
}

void _stdcall SetUserStats(void* userStats)
{
	Babel::UserStats* userStatsPtr = static_cast<Babel::UserStats*>(userStats);
	Babel::UpdateUserStats statsData;
	statsData.UserStats = *userStatsPtr;
	//vb6 index start from 1 but c++ and js start from 0 se we need to convert
	statsData.UserStats.Class -= 1;
	statsData.UserStats.Race -= 1;
	statsData.UserStats.Gender -= 1;
	statsData.UserStats.Home -= 1;
	statsData.EventType = Babel::EventType::UpdateCharacterStats;
	statsData.Size = sizeof(statsData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&statsData, statsData.Size);
}

void _stdcall SetUserName(const char* name)
{
	Babel::Event evt;
	evt.EventType = Babel::EventType::SetUserName;
	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = name;
	evt.Size = sizeof(evt) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evt, sizeof(evt), strInfo);

}

void _stdcall SendChatMessage(void* messageData)
{
	Babel::ChatMessage* messageDataPtr = static_cast<Babel::ChatMessage*>(messageData);
	Babel::ChatMessageEvent chatMessage;
	chatMessage.EventType = Babel::EventType::SendChatMessage;
	chatMessage.SenderColor = messageDataPtr->SenderColor;
	chatMessage.TextColor = messageDataPtr->TextColor;
	chatMessage.TextStyle = messageDataPtr->BoldText > 0 ? (uint8_t)Babel::eTextFormatMask::eBold : 0;
	chatMessage.TextStyle |= messageDataPtr->ItalicText > 0 ? (uint8_t)Babel::eTextFormatMask::eItalic : 0;

	std::vector<Babel::StringInBuffer> strInfo(2);
	strInfo[0].StartPos = messageDataPtr->Sender;
	strInfo[1].StartPos = messageDataPtr->Text;
	chatMessage.Size = sizeof(chatMessage) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&chatMessage, sizeof(chatMessage), strInfo);
}

void _stdcall RegisterGameplayCallbacks(void* gameplayCallbacks)
{	
	BabelTunnel.SetGameplayCallbacks(*(static_cast<Babel::GameplayCallbacks*>(gameplayCallbacks)));
}

void _stdcall UpdateFps(int Fps)
{
	Babel::UpdateFps fpsEvt;
	fpsEvt.Fps = Fps;
	fpsEvt.Size = sizeof(fpsEvt);
	fpsEvt.EventType = Babel::EventType::UpdateFps;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&fpsEvt, fpsEvt.Size);
}

void _stdcall SetInventoryLevel(int Level)
{
	Babel::SetInvLevel invEvt;
	invEvt.Level = Level;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::SetInvLevel;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall SetInvSlot(void* invData)
{
	Babel::InvItem* invDataPtr = static_cast<Babel::InvItem*>(invData);
	Babel::UpdateInvSlot slotMessage;
	slotMessage.Slot = invDataPtr->Slot - 1;
	slotMessage.ObjIndex = invDataPtr->ObjIndex;
	slotMessage.GrhIndex = invDataPtr->GrhIndex;
	slotMessage.ObjType = invDataPtr->ObjType;
	slotMessage.Equipped = invDataPtr->Equipped;
	slotMessage.CantUse = invDataPtr->CantUse;
	slotMessage.Amount = invDataPtr->Amount;
	slotMessage.MinHit = invDataPtr->MinHit;
	slotMessage.MaxHit = invDataPtr->MaxHit;
	slotMessage.MinDef = invDataPtr->MinDef;
	slotMessage.MaxDef = invDataPtr->MaxDef;
	slotMessage.Value = invDataPtr->Value;
	slotMessage.Cooldown = invDataPtr->Cooldown;
	slotMessage.CDType = invDataPtr->CDType;
	slotMessage.CDMask = invDataPtr->CDMask;
	slotMessage.Amunition = invDataPtr->Amunition;
	slotMessage.IsBindable = invDataPtr->IsBindable;
	
	slotMessage.EventType = Babel::EventType::UpdateInvSlot;
	std::vector<Babel::StringInBuffer> strInfo(2);
	strInfo[0].StartPos = invDataPtr->Name;
	strInfo[1].StartPos = invDataPtr->Desc;
	slotMessage.Size = sizeof(slotMessage) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&slotMessage, sizeof(slotMessage), strInfo);
}

void _stdcall SetSpellSlot(void* spellData)
{
	Babel::SpellEntry* spellDataPtr = static_cast<Babel::SpellEntry*>(spellData);
	Babel::UpdateSpellSlot slotMessage;
	slotMessage.Slot = spellDataPtr->Slot - 1;
	slotMessage.SpellIndex = spellDataPtr->SpellIndex;
	slotMessage.IconIndex = spellDataPtr->Icon;
	slotMessage.Cooldown = spellDataPtr->Cooldown;
	slotMessage.IsBindable = spellDataPtr->IsBindable;
	slotMessage.EventType = Babel::EventType::UpdateSpellSlot;

	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = spellDataPtr->Name;
	slotMessage.Size = sizeof(slotMessage) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&slotMessage, sizeof(slotMessage), strInfo);
}

void _stdcall UpdateHpValue(int32_t newHp, int32_t newShield)
{
	Babel::UpdateHp eventData;
	eventData.NewHpValue = newHp;
	eventData.NewShieldValue = newShield;
	eventData.EventType = Babel::EventType::UpdateHp;
	eventData.Size = sizeof(eventData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

void _stdcall UpdateManaValue(int32_t newMana)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newMana;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateMana;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateStaminaValue(int32_t newMana)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newMana;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateStamina;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateDrinkValue(int32_t newMana)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newMana;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateDrink;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateFoodValue(int32_t newMana)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newMana;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateFood;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateGold(int32_t gold, int maxGold)
{
	Babel::DoubleIntEvent invEvt;
	invEvt.Value1 = gold;
	invEvt.Value2 = maxGold;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateGold;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateExp(int32_t current, int32_t target)
{
	Babel::DoubleIntEvent invEvt;
	invEvt.Value1 = current;
	invEvt.Value2 = target;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateExp;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall OpenChat(int32_t mode)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = mode;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::OpenChat;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateStrAndAgiBuff(uint8_t str, uint8_t agi, uint8_t strState, uint8_t agiState)
{
	Babel::UpdateAgiAndStr evtData;
	evtData.Str = str;
	evtData.Agi = agi;
	evtData.StrState = strState;
	evtData.AgiState = agiState;
	evtData.EventType = Babel::EventType::UpdateStrAndAgi;
	evtData.Size = sizeof(evtData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, evtData.Size);
}

void _stdcall UpdateMapInfo(int32_t mapNumber, const char* mapName, int16_t npcCount, void* npcList, uint8_t isSafeMap)
{
	Babel::DoubleIntEvent evtData;
	evtData.Value1 = mapNumber;
	evtData.Value2 = isSafeMap;
	evtData.EventType = Babel::EventType::UpdateMapName;

	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = mapName;
	evtData.Size = sizeof(evtData) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData), strInfo);

	Babel::SingleIntEvent mapPointsEvt;
	Babel::QuestNpc* npcDataPtr = static_cast<Babel::QuestNpc*>(npcList);
	int32_t size = sizeof(Babel::QuestNpc) * npcCount;
	mapPointsEvt.Value = npcCount;
	mapPointsEvt.EventType = Babel::EventType::UpdateMapNpc;
	mapPointsEvt.Size = sizeof(mapPointsEvt) + size;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&mapPointsEvt, sizeof(mapPointsEvt), [size, npcList] (void* buffer) {
		if (size > 0)
		{
			CopyMemory(buffer, npcList, size);
		}
		return size;
	});
}

void _stdcall UpdateUserPos(int16_t PosX, int16_t PosY, void* mapPos)
{
	Babel::Position* mapPosPtr = static_cast<Babel::Position*>(mapPos);
	Babel::UpdateUserPosEvt evtData;
	evtData.TileX = PosX;
	evtData.TileY = PosY;
	evtData.MapPos = *mapPosPtr;
	evtData.Size = sizeof(Babel::UpdateUserPosEvt);
	evtData.EventType = Babel::EventType::UpdateUserPos;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, evtData.Size);
}

void _stdcall UpdateGroupPos(void* mapPos, int16_t groupIndex)
{
	Babel::Position* mapPosPtr = static_cast<Babel::Position*>(mapPos);
	Babel::UpdateGroupMemberPosEvt evtData;
	evtData.GroupIndex = groupIndex - 1;
	evtData.MapPos = *mapPosPtr;
	evtData.Size = sizeof(Babel::UpdateGroupMemberPosEvt);
	evtData.EventType = Babel::EventType::UpdateGroupMemberPos;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, evtData.Size);
}

void _stdcall SetKeySlot(void* slotData)
{
	Babel::InvItem* invDataPtr = static_cast<Babel::InvItem*>(slotData);
	Babel::UpdateInvSlot slotMessage;
	slotMessage.Amount = invDataPtr->Amount;
	slotMessage.CantUse = invDataPtr->CantUse;
	slotMessage.Equipped = invDataPtr->Equipped;
	slotMessage.GrhIndex = invDataPtr->GrhIndex;
	slotMessage.ObjType = invDataPtr->ObjType;
	slotMessage.MaxDef = invDataPtr->MaxDef;
	slotMessage.MaxHit = invDataPtr->MaxHit;
	slotMessage.MinDef = invDataPtr->MinDef;
	slotMessage.MaxDef = invDataPtr->MaxDef;
	slotMessage.ObjIndex = invDataPtr->ObjIndex;
	slotMessage.Slot = invDataPtr->Slot - 1;
	slotMessage.Value = invDataPtr->Value;
	slotMessage.Cooldown = invDataPtr->Cooldown;
	slotMessage.CDType = invDataPtr->CDType;
	slotMessage.Amunition = invDataPtr->Amunition;
	slotMessage.IsBindable = invDataPtr->IsBindable;

	slotMessage.EventType = Babel::EventType::UpdateKeySlot;
	std::vector<Babel::StringInBuffer> strInfo(2);
	strInfo[0].StartPos = invDataPtr->Name;
	strInfo[1].StartPos = invDataPtr->Desc;
	slotMessage.Size = sizeof(slotMessage) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&slotMessage, sizeof(slotMessage), strInfo);
}

void _stdcall UpdateIntervals(void* slotData)
{
	Babel::Intervals* intervalsDataPtr = static_cast<Babel::Intervals*>(slotData);
	Babel::UpdateIntervalsEvent intervalsEvent;
	intervalsEvent.Intervals = *intervalsDataPtr;
	intervalsEvent.Size = sizeof(intervalsEvent);
	intervalsEvent.EventType = Babel::EventType::UpdateIntervals;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&intervalsEvent, intervalsEvent.Size);
}

void _stdcall ActivateInterval(int32_t intervalType)
{
	using namespace std::chrono;
	int64_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	Babel::StartIntervalEvent eventData;
	eventData.IntervalType = intervalType;
	eventData.Timestamp = timestamp;
	eventData.Size = sizeof(eventData);
	eventData.EventType = Babel::EventType::StartInterval;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

void _stdcall SetSafeState(int32_t type, int32_t state)
{
	Babel::DoubleIntEvent evtData;
	evtData.Value1 = type;
	evtData.Value2 = state;
	evtData.Size = sizeof(evtData);
	evtData.EventType = Babel::EventType::UpdateSafeState;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, evtData.Size);
}

void _stdcall UpdateOnlines(int32_t newValue)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newValue;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateOnlines;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateGameTime(int32_t hour, int32_t minutes)
{
	Babel::DoubleIntEvent evtData;
	evtData.Value1 = hour;
	evtData.Value2 = minutes;
	evtData.EventType = Babel::EventType::UpdateGameTime;
	evtData.Size = sizeof(evtData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, evtData.Size);
}

void _stdcall UpdateIsGameMaster(int32_t state)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = state;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateIsGameMaster;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateMagicResistance(int32_t value)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = value;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateMagicResistance;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateMagicAttack(int32_t value)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = value;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateMagicAttack;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall SetWhisperTarget(const char* userName)
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::SetWhisperTarget;
	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = userName;
	evtData.Size = sizeof(evtData) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData), strInfo);
}

void _stdcall PasteText(const char* text)
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::PasteText;
	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = text;
	evtData.Size = sizeof(evtData) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData), strInfo);
}

void _stdcall ReloadSettings()
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::ReloadSettings;
	evtData.Size = sizeof(evtData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData));
}

void _stdcall SetRemoteTrackingState(int state)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = state;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::SetRemoteTrackingState;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateInvAndSpellTracking(int selectedTab, int selectedSpell, int firstSpellOnScroll)
{
	Babel::TripleIntEvent invEvt;
	invEvt.Value1 = selectedTab;
	invEvt.Value2 = selectedSpell;
	invEvt.Value3 = firstSpellOnScroll;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::RemoteInvSpellState;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall HandleRemoteUserClick()
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::RemoteUserClick;
	evtData.Size = sizeof(evtData);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData));
}

void _stdcall UpdateRemoteMousePos(int posX, int posY)
{
	Babel::DoubleIntEvent invEvt;
	invEvt.Value1 = posX;
	invEvt.Value2 = posY;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateRemoteMousePos;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall StartSpellCd(int spellIndex, int cdTime)
{
	Babel::DoubleIntEvent invEvt;
	invEvt.Value1 = spellIndex;
	invEvt.Value2 = cdTime;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::StartSpellCd;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall UpdateCombatAndGlobalChatSettings(int combatValue, int globalValue)
{
	Babel::DoubleIntEvent invEvt;
	invEvt.Value1 = combatValue;
	invEvt.Value2 = globalValue;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateCombatAndglobalChatSettings;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall ActivateStunTimer(int duration)
{
	using namespace std::chrono;
	int64_t timestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	Babel::StartIntervalEvent eventData;
	eventData.IntervalType = duration;
	eventData.Timestamp = timestamp;
	eventData.Size = sizeof(eventData);
	eventData.EventType = Babel::EventType::StartStunTime;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

void _stdcall UpdateHoykeySlot(int slotIndex, void* slotInfo)
{
	Babel::HotkeySlot* slotInfoPtr = static_cast<Babel::HotkeySlot*>(slotInfo);
	Babel::UpdateHotkeySlot eventData;
	eventData.SlotIndex = slotIndex;
	eventData.SlotInfo = *slotInfoPtr;
	eventData.SlotInfo.LastKnownslot = eventData.SlotInfo.LastKnownslot - 1;
	eventData.Size = sizeof(eventData);
	eventData.EventType = Babel::EventType::VBUpdateHotkeySlot;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&eventData, eventData.Size);
}

void _stdcall ActivateFeatureToggle(const char* toggleName)
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::ActivateFeatureToggle;
	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = toggleName;
	evtData.Size = sizeof(evtData) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData), strInfo);
}

void _stdcall ClearToggles()
{
	Babel::Event evt;
	evt.EventType = Babel::EventType::ClearToggles;
	evt.Size = sizeof(evt);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evt, evt.Size);
}

void _stdcall SetHotkeyHideState(int newState)
{
	Babel::SingleIntEvent invEvt;
	invEvt.Value = newState;
	invEvt.Size = sizeof(invEvt);
	invEvt.EventType = Babel::EventType::UpdateHideHotkeyState;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&invEvt, invEvt.Size);
}

void _stdcall ShowQuestion(char* questionText)
{
	Babel::Event evtData;
	evtData.EventType = Babel::EventType::ShowQuestion;
	std::vector<Babel::StringInBuffer> strInfo(1);
	strInfo[0].StartPos = questionText;
	evtData.Size = sizeof(evtData) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evtData, sizeof(evtData), strInfo);
}

void _stdcall OpenMerchant()
{
	Babel::Event evt;
	evt.EventType = Babel::EventType::OpenMerchant;
	evt.Size = sizeof(evt);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evt, evt.Size);	
}

void _stdcall UpdateMerchantSlot(void* invData)
{
	Babel::InvItem* invDataPtr = static_cast<Babel::InvItem*>(invData);
	Babel::UpdateInvSlot slotMessage;
	slotMessage.Slot = invDataPtr->Slot - 1;
	slotMessage.ObjIndex = invDataPtr->ObjIndex;
	slotMessage.GrhIndex = invDataPtr->GrhIndex;
	slotMessage.ObjType = invDataPtr->ObjType;
	slotMessage.Equipped = invDataPtr->Equipped;
	slotMessage.CantUse = invDataPtr->CantUse;
	slotMessage.Amount = invDataPtr->Amount;
	slotMessage.MinHit = invDataPtr->MinHit;
	slotMessage.MaxHit = invDataPtr->MaxHit;
	slotMessage.MinDef = invDataPtr->MinDef;
	slotMessage.MaxDef = invDataPtr->MaxDef;
	slotMessage.Value = invDataPtr->Value;
	slotMessage.Cooldown = invDataPtr->Cooldown;
	slotMessage.CDType = invDataPtr->CDType;
	slotMessage.CDMask = invDataPtr->CDMask;
	slotMessage.Amunition = invDataPtr->Amunition;
	slotMessage.IsBindable = invDataPtr->IsBindable;

	slotMessage.EventType = Babel::EventType::UpdateMerchantSlot;
	std::vector<Babel::StringInBuffer> strInfo(2);
	strInfo[0].StartPos = invDataPtr->Name;
	strInfo[1].StartPos = invDataPtr->Desc;
	slotMessage.Size = sizeof(slotMessage) + PrepareDynamicStrings(strInfo);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&slotMessage, sizeof(slotMessage), strInfo);
}

void _stdcall OpenAo20Shop(int32_t availableCredits, int32_t itemCount, void* itemList)
{
	Babel::AOShopItem* shopItemList = static_cast<Babel::AOShopItem*>(itemList);
	std::vector<Babel::AOShopItem> itemBuffer;
	itemBuffer.resize(itemCount);
	CopyMemory(itemBuffer.data(), itemList, VectorSizeOf(itemBuffer));

	Babel::AoShop evt;
	evt.AvailableCredits = availableCredits;
	evt.EventType = Babel::EventType::OpenAoShop;
	evt.Size = sizeof(evt) + + sizeof(int32_t) + VectorSizeOf(itemBuffer);
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&evt, sizeof(evt), itemBuffer);
}
