#include "dllmain.h"
#include <Windows.h>
#include "Core/Tunnel.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include "Core/Logger.hpp"
#include "CallbackDefines.hpp"

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

void _stdcall SendKeyEvent(int16_t keyCode, bool shift, int type, bool capsState, bool inspector)
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
	callbacks.CloseClient = (Babel::TCloseClient)(closeClient);
	callbacks.Login = (Babel::TLogInCallback)(loginCallback);
	callbacks.CreateAccount = (Babel::TCreateAccountCallback)(createAccount);
	callbacks.SetHost = (Babel::TSetHost)(setHost);
	callbacks.ValidateAccount = (Babel::TValidateAccount)(validateCode);
	callbacks.ResendValidationCode = (Babel::TResendValidationCode)(resendValidationCode);
	callbacks.RequestPasswordReset = (Babel::TRequestPasswordReset)(requestPasswordReset);
	callbacks.NewPasswordRequest = (Babel::TNewPasswordRequest)(newPasswordRequest);
	callbacks.SelectCharacter = (Babel::TSelectCharacter)(selectCharacter);
	callbacks.LoginWithCharacter = (Babel::TLoginCharacterIndex)(loginCharacter);
	callbacks.ReturnToLogin = (Babel::TCloseClient)(returnToLogin);
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

void _stdcall LoginAddCharacter(const char* name, int head, int body, int helm, int shield, int weapon, int level, int status, int index)
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