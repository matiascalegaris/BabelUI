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

void _stdcall RegisterCallbacks(int loginCallback, int closeClient)
{
	Babel::CallbacksList callbacks;
	callbacks.closeClient = (Babel::TCloseClient)(closeClient);
	callbacks.login = (Babel::TLogInCallback)(loginCallback);
	BabelTunnel.SetCallbacks(callbacks);
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