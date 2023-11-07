#pragma once
#include <cstdint>
#include "framework.h"

extern "C"
{
	LIBRARY_API bool InitializeBabel(void* settings);
	LIBRARY_API bool GetImageBuffer(char* buffer, int size);
	LIBRARY_API void _stdcall SendMouseEvent(int mouseX, int mouseY, int type, int buttonState);
	LIBRARY_API void _stdcall SendScrollEvent(int direction);
	LIBRARY_API void _stdcall SendKeyEvent(int16_t keyCode, int16_t shift, int type, bool capsState, bool inspector);
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
	
	//GameplayInterface
	LIBRARY_API void _stdcall SetUserStats(void* userStats);
	LIBRARY_API void _stdcall SetUserName(const char* name);
	LIBRARY_API void _stdcall SendChatMessage(void* messageData);
	LIBRARY_API void _stdcall RegisterGameplayCallbacks(void* gameplayCallbacks);
	LIBRARY_API void _stdcall UpdateFps(int Fps);
	LIBRARY_API void _stdcall SetInventoryLevel(int Level);
	LIBRARY_API void _stdcall SetInvSlot(void* invData);
	LIBRARY_API void _stdcall SetSpellSlot(void* spellData);
	LIBRARY_API void _stdcall UpdateHpValue(int32_t newHp, int32_t newShield);
	LIBRARY_API void _stdcall UpdateManaValue(int32_t newMana);
	LIBRARY_API void _stdcall UpdateStaminaValue(int32_t newMana);
	LIBRARY_API void _stdcall UpdateDrinkValue(int32_t newMana);
	LIBRARY_API void _stdcall UpdateFoodValue(int32_t newMana);
	LIBRARY_API void _stdcall UpdateGold(int32_t gold, int32_t maxGold);
	LIBRARY_API void _stdcall UpdateExp(int32_t current, int32_t target);
	LIBRARY_API void _stdcall OpenChat(int32_t mode);
	LIBRARY_API void _stdcall UpdateStrAndAgiBuff(uint8_t str, uint8_t agi, uint8_t strState, uint8_t agiState);
	LIBRARY_API void _stdcall UpdateMapInfo(int32_t mapNumber, int32_t miniMapFile, const char * mapName, int16_t npcCount, void* npcList, uint8_t isSafeMap);
	LIBRARY_API void _stdcall UpdateUserPos(int16_t PosX, int16_t PosY, void *mapPos);
	LIBRARY_API void _stdcall UpdateGroupPos(void* mapPos, int16_t groupIndex);
	LIBRARY_API void _stdcall SetKeySlot(void* slotData);
	LIBRARY_API void _stdcall UpdateIntervals(void* slotData);
	LIBRARY_API void _stdcall ActivateInterval(int32_t intervalType);
	LIBRARY_API void _stdcall SetSafeState(int32_t type, int32_t state);
	LIBRARY_API void _stdcall UpdateOnlines(int32_t newValue);
	LIBRARY_API void _stdcall UpdateGameTime(int32_t hour, int32_t minutes);
	LIBRARY_API void _stdcall UpdateIsGameMaster(int32_t state);
	LIBRARY_API void _stdcall UpdateMagicResistance(int32_t value);
	LIBRARY_API void _stdcall UpdateMagicAttack(int32_t value);
	LIBRARY_API void _stdcall SetWhisperTarget(const char* userName);
	LIBRARY_API void _stdcall PasteText(const char* text);
	LIBRARY_API void _stdcall ReloadSettings();
	LIBRARY_API void _stdcall SetRemoteTrackingState(int state);
	LIBRARY_API void _stdcall UpdateInvAndSpellTracking(int selectedTab, int selectedSpell, int firstSpellOnScroll);
	LIBRARY_API void _stdcall HandleRemoteUserClick();
	LIBRARY_API void _stdcall UpdateRemoteMousePos(int posX, int posY);
	LIBRARY_API void _stdcall StartSpellCd(int spellIndex, int cdTime);
	LIBRARY_API void _stdcall UpdateCombatAndGlobalChatSettings(int combatValue, int globalValue);
	LIBRARY_API void _stdcall ActivateStunTimer(int duration);
	LIBRARY_API void _stdcall UpdateHoykeySlot(int slotIndex, void* slotInfo);
	LIBRARY_API void _stdcall ActivateFeatureToggle(const char* toggleName);
	LIBRARY_API void _stdcall ClearToggles();
	LIBRARY_API void _stdcall SetHotkeyHideState(int newState);
	LIBRARY_API void _stdcall ShowQuestion(char* questionText);
	LIBRARY_API void _stdcall OpenMerchant();
	LIBRARY_API void _stdcall UpdateMerchantSlot(void* invData);
	LIBRARY_API void _stdcall OpenAo20Shop(int32_t availableCredits, int32_t itemCount, void* itemList);
	LIBRARY_API void _stdcall OpenLobbyList();
	LIBRARY_API void _stdcall UpdateLobby(void *LobbyInfo);
	LIBRARY_API void _stdcall ShowClanCall(int map, int posX, int posY);
	LIBRARY_API void _stdcall OpenSkillDialog(int availableSkills, uint8_t* skillList, int16_t listSize);
}