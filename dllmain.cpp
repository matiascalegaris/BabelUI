#include "Definitions.h"
#include <Windows.h>
#include "Core/Tunnel.h"
#include "SharedMemory/Events/EventHandler.hpp"

namespace {
	Babel::Tunnel BabelTunnel;
}

bool _stdcall InitializeBabel(int width, int height)
{
	try {
		return BabelTunnel.Initialize(width, height);
	}
	catch (const std::exception& err)
	{
		return false;
	}
}

bool _stdcall GetImageBuffer(char* buffer, int size)
{
	return BabelTunnel.GetSyncData().GetLastBuffer(buffer, size);
}

void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState)
{
	Babel::MouseData mouseData;
	mouseData.PosX = mouseX;
	mouseData.PosY = mouseY;
	mouseData.TypeFlags = (type << 4 & 0xf0) | (buttonState & 0x0f);
	mouseData.Size = sizeof(Babel::MouseData);
	mouseData.EventType = Babel::EventType::MouseData;
	BabelTunnel.GetSyncData().GetApiMessenger().AddEvent((uint8_t*)&mouseData, mouseData.Size);
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
