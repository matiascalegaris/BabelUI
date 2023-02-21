#pragma once
#include <cstdint>
#include "framework.h"

extern "C"
{
	LIBRARY_API bool _stdcall InitializeBabel(int width, int height);
	LIBRARY_API bool _stdcall GetImageBuffer(char* buffer, int size);
	LIBRARY_API void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState);
	LIBRARY_API void _stdcall SendKeyEvent(int16_t keyCode, bool shift, int type, bool capsState, bool inspector);
	LIBRARY_API void _stdcall RegisterCallbacks(int loginCallback, int closeClient, int createAccount, 
												int setHost, int ValidateCode, int resendValidationCode, int requestPasswordReset, int newPasswordRequest);
	LIBRARY_API void _stdcall SendErrorMessage(const char * str, int messageType, int action);
	LIBRARY_API void _stdcall SetActiveScreen(const char* str);
	LIBRARY_API void _stdcall SetLoadingMessage(const char* str, int localize);

	LIBRARY_API uint32_t _stdcall NextPowerOf2(uint32_t value);


	LIBRARY_API bool _stdcall CreateDebugWindow(int width, int height);
	LIBRARY_API bool _stdcall GetDebugImageBuffer(char* buffer, int size);
	LIBRARY_API void _stdcall SendDebugMouseEvent(int mouseX, int mouseY, int type, int buttonState);
	

}