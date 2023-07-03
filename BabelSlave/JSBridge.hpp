#pragma once
#include <AppCore/JSHelpers.h>
#include "SharedMemory/Events/GameplayEvents.hpp"
#include "AoResources/Resources.hpp"

namespace Babel
{
	class Renderer;
	class Application;
	
	// handles the comunication between c++ and js
	class JSBridge : public EventListener
	{
	public:
		JSBridge(EventBuffer& eventBuffer, Renderer& renderer, Application& application);

		void RegisterJSApi(ultralight::JSObject& global);
		void HandleEvent(const Event& eventData) override;
	private: //js callbacks
		ultralight::JSValue LogIn(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue CreateAccount(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetStoredCredentials(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue ResendValidationCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue ValidateCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue SetHost(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue RequestPasswordReset(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue NewPasswordRequest(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetCharacterDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetHeadDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetGrhDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue SelectCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue LoginCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue CreateCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetStoredLocale(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue EnableDebug(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void ExitCharacterSelection(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue RequestDeleteCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue ConfirmDeleteCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue RequestCharacterTransfer(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);

		//Gameplay
		void SendChatMsg(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void OpenVBDialog(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UpdateSelectedInvSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UseInvSlotIndex(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UpdateSelectedSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UseSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UpdateInputFocus(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void ClickLink(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void ClickGold(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void MoveInvItem(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void RequestAction(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UseKey(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void MoveSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void DeleteItem(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void UpdateOpenDialog(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetSpellInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		ultralight::JSValue GetItemInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void LogError(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void InformSpellListScroll(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);
		void ClickMiniMapPos(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args);

	private: //c++ events functions
		void HandlekeyData(const KeyEvent& keyData);
		void SendErrorMessage(const ErrorMessageEvent& messageData);
		void SetActiveScreen(const std::string& name);
		void SetLoadingMessage(const std::string& message, bool localize);
		void HandleLoginCharList(const CharacterListEvent& messageData);
		void DeleteCharacterFromList(int characterIndex);
		void RequestDeleteCode();

		//gameplay
		void UpdateStats(const UserStats& userStats);
		void UpdateUserName(const std::string& name);
		void SendChatMessage(const std::string& sender, const Color& senderColor, const std::string& text, const Color& textColor, uint8_t textStyle);
		void UpdateFps(int Fps);
		void UpdateInvLevel(int level);
		void UpdateInvSlot(const std::string& objName, const std::string& objDesc, const Babel::UpdateInvSlot& slotInfo);
		void UpdateSpellSlot(const std::string& objName, const Babel::UpdateSpellSlot& slotInfo);
		void UpdateHp(int32_t newHp, int32_t newShield);
		void UpdateMana(int32_t newMana);
		void UpdateStamina(int32_t newStamina);
		void UpdateDrink(int32_t newDrink);
		void UpdateFood(int32_t newFood);
		void UpdateGold(int32_t newGold);
		void UpdateExp(int32_t current, int32_t maxExp);
		void OpenChat(int32_t mode);
		void UpdateStrAndAgi(const UpdateAgiAndStr& updatedInfo);
		void UpdateMapName(int32_t mapNumber, const std::string& mapName, int32_t isSafe);
		void UpdateMapNpc(int32_t npcCount, const Babel::QuestNpc* npcList);
		void UpdateUserPos(const Babel::UpdateUserPosEvt& updatePos);
		void UpdateGroupMemberPos(const Babel::UpdateGroupMemberPosEvt& memberPos);
		void UpdateKeySlot(const std::string& objName, const std::string& objDesc, const Babel::UpdateInvSlot& slotInfo);
		void UpdateInterval(const Intervals& intervals);
		void StartInterval(int32_t intervalType, int64_t timestamp);
		void UpdatesafeState(int32_t type, int32_t state);
		void UpdateOnlines(int32_t onlines);
		void UpdateGameTime(int32_t hour, int32_t minutes);
		void UpdateIsGameMaster(int32_t state);
		void UpdateMagicResistance(int32_t value);
		void UpdateMagicAttack(int32_t value);
		void UpdateWhisperTarget(const std::string& target);
		void HandleTextPaste(const std::string& data);
		void UpdateGameSettings();
		void SetRemoteTrackingState(int newState);
		void SetRemoteInvstate(int selectedTab, int selectedSpell, int firstListSpellInView);
		void RemoteUserClick();
		void UpdateRemoteMousePos(int posX, int posY);

	private:
		void CallJsFunction(JSContextRef& context, const char* functionName, const JSValueRef* args, int argCount);
		EventBuffer& mEventBuffer;
		Renderer& mRenderer;
		Application& mApplication;
		std::unique_ptr<AO::Resources> mResources;
	};
}