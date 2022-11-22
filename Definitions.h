#pragma once
#include <cstdint>
#include "framework.h"


extern "C"
{
	LIBRARY_API int __stdcall TestBabelCall(int input);
	LIBRARY_API bool _stdcall InitializeBabel(int width, int height);

	LIBRARY_API bool _stdcall GetImageBuffer(char* buffer, int size);

	LIBRARY_API void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState);

	LIBRARY_API uint32_t _stdcall NextPowerOf2(uint32_t value);
}