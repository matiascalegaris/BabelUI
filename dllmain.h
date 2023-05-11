#pragma once
#include <cstdint>
#include "framework.h"

extern "C"
{
	LIBRARY_API bool InitializeBabel(void* settings);
	LIBRARY_API bool GetImageBuffer(char* buffer, int size);
	LIBRARY_API void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState);
	LIBRARY_API void _stdcall SendKeyEvent(int16_t keyCode, bool shift, int type, bool capsState, bool inspector);
	LIBRARY_API void _stdcall RegisterCallbacks(int loginCallback, int closeClient, int createAccount, int setHost, int ValidateCode, int resendValidationCode,
												int requestPasswordReset, int newPasswordRequest, int selectCharacter, int loginCharacter, int returnToLogin, int createCharacter,
												int requestDeleteCharater, int confirmDeleteCharacter, int transferCharacter);
	LIBRARY_API void _stdcall SendErrorMessage(const char * str, int MessageType, int Action);
	LIBRARY_API void _stdcall SetActiveScreen(const char* str);
	LIBRARY_API void _stdcall SetLoadingMessage(const char* str, int localize);
	LIBRARY_API void _stdcall LoginCharacterListPrepare(int characterCount);
	LIBRARY_API void _stdcall LoginAddCharacter(const char* name, int head,	int body, int helm,	int shield,	int weapon,	int level,	int status,	int index, int classId);
	LIBRARY_API void _stdcall RemoveCharacterFromList(int index);
	LIBRARY_API void _stdcall LoginSendCharacters();
	LIBRARY_API void _stdcall RequestDeleteCode();

	LIBRARY_API uint32_t _stdcall NextPowerOf2(uint32_t value);


	LIBRARY_API bool _stdcall CreateDebugWindow(int width, int height);
	LIBRARY_API bool _stdcall GetDebugImageBuffer(char* buffer, int size);
	LIBRARY_API void _stdcall SendDebugMouseEvent(int mouseX, int mouseY, int type, int buttonState);

	LIBRARY_API uint32_t _stdcall GetTelemetry(const char* str, const uint8_t* data, uint32_t maxSize);
	

}