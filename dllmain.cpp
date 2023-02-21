#include "Definitions.h"
#include <Windows.h>
#include "Core/Tunnel.h"
#include "SharedMemory/Events/EventHandler.hpp"
#include "Core/Logger.hpp"
#include "CallbackDefines.hpp"

namespace {
	Babel::Tunnel BabelTunnel;

	void FillMouseData(Babel::MouseData& mouseData, int mouseX, int mouseY, int type, int buttonState)
	{
		mouseData.PosX = mouseX;
		mouseData.PosY = mouseY;
		mouseData.TypeFlags = (type << 4 & 0xf0) | (buttonState & 0x0f);
		mouseData.Size = sizeof(Babel::MouseData);
	}
}

bool _stdcall InitializeBabel(int width, int height)
{
	try {
		return BabelTunnel.Initialize(width, height);
	}
	catch (const std::exception& )
	{
		return false;
	}
}

bool _stdcall GetImageBuffer(char* buffer, int size)
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

void _stdcall RegisterCallbacks(int loginCallback, int closeClient, int createAccount, int setHost, int validateCode,
								int resendValidationCode, int requestPasswordReset, int newPasswordRequest)
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
	BabelTunnel.SetCallbacks(callbacks);
}

void _stdcall SendErrorMessage(const char* str, int messageType, int action)
{
	Babel::ErrorMessageEvent message;
	message.action = action;
	message.messageType = messageType;
	message.size = strnlen(str, sizeof(message.strData)-1);
	message.Size = sizeof(Babel::ErrorMessageEvent);
	strncpy_s(message.strData, sizeof(message.strData) - 1,str, message.size);
	message.strData[message.size] = 0;
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