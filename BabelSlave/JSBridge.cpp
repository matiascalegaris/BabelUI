#include "JSBridge.hpp"
#include <locale>
#include <codecvt>
#include <Ultralight/Ultralight.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include "SharedMemory/EventBuffer.hpp"
#include "Application.hpp"
#include "Renderer.h"
#include "Utils/IniReader.h"
#include "Utils/FileUtils.h"
#include "Utils/Encoding.h"
#include "AoResources/Resources.hpp"
#include "Core/Logger.hpp"
#include "Utils/StringUtils.hpp"

namespace Babel
{
	namespace {
		enum VB6Modifiers
		{
			vbShift = 1,
			vbCtrl = 2,
			vbAlt = 4
		};
		std::string ascii_to_utf8(const std::string& ascii_string) 
		{
			int num_wchars = MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, NULL, 0);
			std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
			MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, wide_string.get(), num_wchars);

			int num_chars = WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
			std::unique_ptr<char[]> utf8_string(new char[num_chars]);
			WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, utf8_string.get(), num_chars, NULL, NULL);

			return std::string(utf8_string.get());
		}

		std::string utf8_to_ascii(const std::string& utf8_string) 
		{
			int num_wchars = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, NULL, 0);
			std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
			MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, wide_string.get(), num_wchars);

			int num_chars = WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
			std::unique_ptr<char[]> ascii_string(new char[num_chars]);
			WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, ascii_string.get(), num_chars, NULL, NULL);

			return std::string(ascii_string.get());
		}

		void SetObjectString(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, const char* value)
		{			
			JSRetainPtr<JSStringRef> valueStr =
				adopt(JSStringCreateWithUTF8CString(value));
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			auto jvalue = JSValueMakeString(ctx, valueStr.get());
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
		}
		template<typename T>
		void SetObjectNumber(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, T& value)
		{
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			auto jvalue = JSValueMakeNumber(ctx, value);
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
		}

		void SetObjectBoolean(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, bool value)
		{
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			auto jvalue = JSValueMakeBoolean(ctx, value);
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
		}

		void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, JSObjectRef& value)
		{
			JSRetainPtr<JSStringRef> paramStr =
				adopt(JSStringCreateWithUTF8CString(paramName));
			auto jparam = JSValueMakeString(ctx, paramStr.get());
			JSObjectSetProperty(ctx, objectRef, paramStr.get(), value, 0, nullptr);
		}

		void SetGrhJsObject(JSContextRef& ctx, JSObjectRef& objectRef, const AO::GrhDetails& grhData)
		{
			SetObjectNumber(ctx, objectRef, "imageNumber", grhData.ImageNumber);
			SetObjectNumber(ctx, objectRef, "startX", grhData.StartPos.X);
			SetObjectNumber(ctx, objectRef, "startY", grhData.StartPos.Y);
			SetObjectNumber(ctx, objectRef, "width", grhData.EndPos.X);
			SetObjectNumber(ctx, objectRef, "height", grhData.EndPos.Y);
		}

		void SetSpellObject(JSContextRef& ctx, JSObjectRef& objectRef, const AO::SpellData& spellData)
		{
			SetObjectString(ctx, objectRef, "name", ascii_to_utf8(spellData.Name).c_str());
			SetObjectString(ctx, objectRef, "description", ascii_to_utf8(spellData.Description).c_str());
			SetObjectNumber(ctx, objectRef, "cooldown", spellData.Cooldown);
			SetObjectNumber(ctx, objectRef, "iconIndex", spellData.IconIndex);
			SetObjectNumber(ctx, objectRef, "requiredMana", spellData.RequiredMana);
			SetObjectNumber(ctx, objectRef, "requiredSkill", spellData.RequiredSkill);
			SetObjectNumber(ctx, objectRef, "requiredStamina", spellData.RequiredStamina);
		}

		void SetObjectInfo(JSContextRef& ctx, JSObjectRef& objectRef, const AO::ObjectData& objData)
		{
			SetObjectString(ctx, objectRef, "name", ascii_to_utf8(objData.Name).c_str());
			SetObjectNumber(ctx, objectRef, "grhIndex", objData.grhindex);
			SetObjectNumber(ctx, objectRef, "cooldown", objData.Cooldown);
			SetObjectNumber(ctx, objectRef, "minDef", objData.MinDef);
			SetObjectNumber(ctx, objectRef, "maxDef", objData.MaxDef);
			SetObjectNumber(ctx, objectRef, "minHit", objData.MinHit);
			SetObjectNumber(ctx, objectRef, "maxHit", objData.MaxHit);
			SetObjectNumber(ctx, objectRef, "objType", objData.ObjType);
			SetObjectString(ctx, objectRef, "createsLight", ascii_to_utf8(objData.CreatesLight).c_str());
			SetObjectNumber(ctx, objectRef, "reatesGRH", objData.CreateFloorParticle);
			SetObjectString(ctx, objectRef, "createsLight", ascii_to_utf8(objData.CreatesGRH).c_str());
			SetObjectNumber(ctx, objectRef, "roots", objData.Roots);
			SetObjectNumber(ctx, objectRef, "wood", objData.Wood);
			SetObjectNumber(ctx, objectRef, "elvenWood", objData.ElvenWood);
			SetObjectNumber(ctx, objectRef, "wolfSkin", objData.WolfSkin);
			SetObjectNumber(ctx, objectRef, "brownBearSkin", objData.BrownBearSkin);
			SetObjectNumber(ctx, objectRef, "polarBearSkin", objData.PolarBearSkin);
			SetObjectNumber(ctx, objectRef, "lingH", objData.LingH);
			SetObjectNumber(ctx, objectRef, "lingP", objData.LingP);
			SetObjectNumber(ctx, objectRef, "lingO", objData.LingO);
			SetObjectNumber(ctx, objectRef, "destroy", objData.Destroy);
			SetObjectNumber(ctx, objectRef, "projectile", objData.Projectile);
			SetObjectNumber(ctx, objectRef, "ammunition", objData.Ammunition);
			SetObjectNumber(ctx, objectRef, "blacksmithingSkill", objData.BlacksmithingSkill);
			SetObjectNumber(ctx, objectRef, "potionSkill", objData.PotionSkill);
			SetObjectNumber(ctx, objectRef, "tailoringSkill", objData.TailoringSkill);
			SetObjectNumber(ctx, objectRef, "value", objData.Value);			
			SetObjectNumber(ctx, objectRef, "grabbable", objData.Grabbable);
			SetObjectNumber(ctx, objectRef, "Key", objData.Key);
			SetObjectNumber(ctx, objectRef, "cdType", objData.CdType);
			SetObjectNumber(ctx, objectRef, "spellIndex", objData.SpellIndex);
		}

		void SetColorObject(JSContextRef& ctx, JSObjectRef& objectRef, const Babel::Color& colorData)
		{
			SetObjectNumber(ctx, objectRef, "R", colorData.R);
			SetObjectNumber(ctx, objectRef, "G", colorData.G);
			SetObjectNumber(ctx, objectRef, "B", colorData.B);
		}

		uint32_t ConvertKeyModifiers(int16_t vbModifiers)
		{
			uint32_t ret = 0;
			ret = ret | ((vbModifiers & vbShift) > 0 ? ultralight::KeyEvent::kMod_ShiftKey : 0);
			ret = ret | ((vbModifiers & vbCtrl) > 0 ? ultralight::KeyEvent::kMod_CtrlKey : 0);
			ret = ret | ((vbModifiers & vbAlt) > 0 ? ultralight::KeyEvent::kMod_AltKey : 0);
			return ret;
		}
	}

	using namespace ultralight;
	JSBridge::JSBridge(EventBuffer& eventBuffer, Renderer& renderer, Application& application) 
		: mEventBuffer(eventBuffer), mRenderer(renderer), mApplication(application)
	{
		mResources = std::make_unique<AO::Resources>(application.GetSettings().CompressedResources);
	}

	void JSBridge::RegisterJSApi(ultralight::JSObject& global)
	{
		JSObject Api;
		Api["Login"] = BindJSCallbackWithRetval(&JSBridge::LogIn);
		Api["CloseClient"] = BindJSCallback(&JSBridge::CloseClient);
		Api["GetCredentials"] = BindJSCallbackWithRetval(&JSBridge::GetStoredCredentials);
		Api["CreateAccount"] = BindJSCallbackWithRetval(&JSBridge::CreateAccount);
		Api["ResendValidationCode"] = BindJSCallbackWithRetval(&JSBridge::ResendValidationCode);
		Api["ValidateCode"] = BindJSCallbackWithRetval(&JSBridge::ValidateCode);
		Api["SetHost"] = BindJSCallbackWithRetval(&JSBridge::SetHost);
		Api["RequestPasswordReset"] = BindJSCallbackWithRetval(&JSBridge::RequestPasswordReset);
		Api["NewPasswordRequest"] = BindJSCallbackWithRetval(&JSBridge::NewPasswordRequest);
		Api["GetCharacterDrawInfo"] = BindJSCallbackWithRetval(&JSBridge::GetCharacterDrawInfo);
		Api["SelectCharacter"] = BindJSCallbackWithRetval(&JSBridge::SelectCharacter);
		Api["LoginCharacter"] = BindJSCallbackWithRetval(&JSBridge::LoginCharacter);
		Api["GetHeadDrawInfo"] = BindJSCallbackWithRetval(&JSBridge::GetHeadDrawInfo);
		Api["GetGrhDrawInfo"] = BindJSCallbackWithRetval(&JSBridge::GetGrhDrawInfo);		
		Api["ExitCharacterSelection"] = BindJSCallback(&JSBridge::ExitCharacterSelection);
		Api["CreateCharacter"] = BindJSCallbackWithRetval(&JSBridge::CreateCharacter);
		Api["GetStoredLocale"] = BindJSCallbackWithRetval(&JSBridge::GetStoredLocale);
		Api["EnableDebug"] = BindJSCallbackWithRetval(&JSBridge::EnableDebug);
		Api["RequestDeleteCharacter"] = BindJSCallbackWithRetval(&JSBridge::RequestDeleteCharacter);
		Api["ConfirmDeleteCharacter"] = BindJSCallbackWithRetval(&JSBridge::ConfirmDeleteCharacter);
		Api["RequestCharacterTransfer"] = BindJSCallbackWithRetval(&JSBridge::RequestCharacterTransfer);
		Api["SendChat"] = BindJSCallback(&JSBridge::SendChatMsg);
		Api["OpenVBDialog"] = BindJSCallback(&JSBridge::OpenVBDialog);
		Api["UpdateSelectedInvSlot"] = BindJSCallback(&JSBridge::UpdateSelectedInvSlot);
		Api["UseInvSlotIndex"] = BindJSCallback(&JSBridge::UseInvSlotIndex);
		Api["UpdateSelectedSpellSlot"] = BindJSCallback(&JSBridge::UpdateSelectedSpellSlot);
		Api["UseSpellSlot"] = BindJSCallback(&JSBridge::UseSpellSlot);
		Api["UpdateInputFocus"] = BindJSCallback(&JSBridge::UpdateInputFocus);
		Api["OpenLink"] = BindJSCallback(&JSBridge::ClickLink);
		Api["GoldClick"] = BindJSCallback(&JSBridge::ClickGold);
		Api["MoveInvItem"] = BindJSCallback(&JSBridge::MoveInvItem);
		Api["RequestAction"] = BindJSCallback(&JSBridge::RequestAction);
		Api["UseKeySlotIndex"] = BindJSCallback(&JSBridge::UseKey);
		Api["MoveSpellSlot"] = BindJSCallback(&JSBridge::MoveSpellSlot);
		Api["DeleteItem"] = BindJSCallback(&JSBridge::DeleteItem);
		Api["UpdateOpenDialog"] = BindJSCallback(&JSBridge::UpdateOpenDialog);
		Api["GetSpellInfo"] = BindJSCallbackWithRetval(&JSBridge::GetSpellInfo);
		Api["GetItemInfo"] = BindJSCallbackWithRetval(&JSBridge::GetItemInfo);
		Api["LogError"] = BindJSCallback(&JSBridge::LogError);
		Api["InformSpellListScroll"] = BindJSCallback(&JSBridge::InformSpellListScroll);
		Api["ClickMiniMapPos"] = BindJSCallback(&JSBridge::ClickMiniMapPos);
		Api["UpdateCombatAndGlobatChatState"] = BindJSCallback(&JSBridge::UpdateCombatAndGlobatChatState);
		Api["Copytext"] = BindJSCallback(&JSBridge::Copytext);
		
		global["BabelUI"] = JSValue(Api);
	}
	void JSBridge::HandleEvent(const Event& eventData)
	{
		switch (eventData.EventType)
		{
			case EventType::MouseData:
			{
				const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
				mRenderer.SendMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
				break;
			}
			case EventType::SendScrollEvent:
			{
				const SingleIntEvent& mouseEvtdata = static_cast<const SingleIntEvent&>(eventData);
				mRenderer.SendScrollEvent(mouseEvtdata.Value * 0.5);
				break;
			}
			case EventType::DebugMouseData:
			{
				const MouseData& mouseEvtdata = static_cast<const MouseData&>(eventData);
				mRenderer.SendInpectorMouseEvent(mouseEvtdata.PosX, mouseEvtdata.PosY, (mouseEvtdata.TypeFlags >> 4) & 0x0f, mouseEvtdata.TypeFlags & 0x0f);
				break;
			}
			case EventType::EnableDebugWindow:
			{
				const WindowInfo& windowEvtdata = static_cast<const WindowInfo&>(eventData);
				mApplication.EnableDebugWindow(windowEvtdata.Width, windowEvtdata.Height);
				break;
			}
			case EventType::KeyData:
			{
				const KeyEvent& keyData = static_cast<const KeyEvent&>(eventData);
				HandlekeyData(keyData);
				break;
			}
			case EventType::ErrorMessage:
			{
				const ErrorMessageEvent& errorData = static_cast<const ErrorMessageEvent&>(eventData);
				SendErrorMessage(errorData);
				break;
			}
			case EventType::Close:
				mApplication.Stop();
				break;
			case EventType::SetActiveScreen:
			{
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
				auto size = output - (char*)(&eventData);
				std::string name(strInfo[0].StartPos, strInfo[0].Size);
				SetActiveScreen(name);
			}
				break;
			case EventType::SetLoadingMessage:
			{
				const LoadingMessage& loadingData = static_cast<const LoadingMessage&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(LoadingMessage), strInfo);
				auto size = output - (char*)(&eventData);
				std::string message(strInfo[0].StartPos, strInfo[0].Size);
				SetLoadingMessage(message, loadingData.Localize);
			}
			break;
			case EventType::LoginCharList:
			{
				const CharacterListEvent& charListEvent = static_cast<const CharacterListEvent&>(eventData);
				HandleLoginCharList(charListEvent);
			}
			break;
			case EventType::RequestDeleteCode:
			{
				RequestDeleteCode();
			}
			break;
			case EventType::RemoveCharacterFromList:
			{
				const SelectCharacterEvent& charEvent = static_cast<const SelectCharacterEvent&>(eventData);
				DeleteCharacterFromList(charEvent.CharIndex);
			}
			break;
			case EventType::UpdateCharacterStats:
			{
				const UpdateUserStats& userStatsEvent = static_cast<const UpdateUserStats&>(eventData);
				UpdateStats(userStatsEvent.UserStats);
			}
			break;
			case EventType::SetUserName:
			{
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Event), strInfo);
				auto size = output - (char*)(&eventData);
				std::string name(strInfo[0].StartPos, strInfo[0].Size);
				UpdateUserName(name);
			}
			break;
			case EventType::SendChatMessage:
			{
				const ChatMessageEvent& chatEvent = static_cast<const ChatMessageEvent&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(2);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(ChatMessageEvent), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == chatEvent.Size);
				std::string sender(strInfo[0].StartPos, strInfo[0].Size);
				std::string text(strInfo[1].StartPos, strInfo[1].Size);
				SendChatMessage(sender, chatEvent.SenderColor, text, chatEvent.TextColor, chatEvent.TextStyle);
			}
			break;
			case EventType::UpdateFps:
			{
				const Babel::UpdateFps& fpsEvent = static_cast<const Babel::UpdateFps&>(eventData);
				UpdateFps(fpsEvent.Fps);
			}
			break;
			case EventType::SetInvLevel:
			{
				const Babel::SetInvLevel& invLevelEvent = static_cast<const Babel::SetInvLevel&>(eventData);
				UpdateInvLevel(invLevelEvent.Level);
			}
			break;
			case EventType::UpdateInvSlot:
			{
				const Babel::UpdateInvSlot& slotInfo = static_cast<const Babel::UpdateInvSlot&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(2);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::UpdateInvSlot), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == slotInfo.Size);
				std::string objName(strInfo[0].StartPos, strInfo[0].Size);
				std::string objDesc(strInfo[1].StartPos, strInfo[1].Size);
				UpdateInvSlot(objName, objDesc, slotInfo);
			}
			break;
			case EventType::UpdateSpellSlot:
			{
				const Babel::UpdateSpellSlot& slotInfo = static_cast<const Babel::UpdateSpellSlot&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::UpdateSpellSlot), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == slotInfo.Size);
				std::string objName(strInfo[0].StartPos, strInfo[0].Size);
				UpdateSpellSlot(objName, slotInfo);
			}
			break;
			case EventType::UpdateHp:
			{
				const Babel::UpdateHp& invLevelEvent = static_cast<const Babel::UpdateHp&>(eventData);
				UpdateHp(invLevelEvent.NewHpValue, invLevelEvent.NewShieldValue);
				break;
			}
			case EventType::UpdateMana:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateMana(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateStamina:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateStamina(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateFood:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateFood(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateDrink:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateDrink(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateGold:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateGold(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateExp:
			{
				const Babel::DoubleIntEvent& doubleInEvent = static_cast<const Babel::DoubleIntEvent&>(eventData);
				UpdateExp(doubleInEvent.Value1, doubleInEvent.Value2);
				break;
			}
			case EventType::OpenChat:
			{
				const Babel::SingleIntEvent& invLevelEvent = static_cast<const Babel::SingleIntEvent&>(eventData);
				OpenChat(invLevelEvent.Value);
				break;
			}
			case EventType::UpdateStrAndAgi:
			{
				const Babel::UpdateAgiAndStr& evtData = static_cast<const Babel::UpdateAgiAndStr&>(eventData);
				UpdateStrAndAgi(evtData);
				break;
			}
			case EventType::UpdateMapName:
			{
				const Babel::DoubleIntEvent& evtData = static_cast<const Babel::DoubleIntEvent&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::DoubleIntEvent), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == evtData.Size);
				std::string mapName(strInfo[0].StartPos, strInfo[0].Size);
				UpdateMapName(evtData.Value1, mapName, evtData.Value2);
				break;
			}
			case EventType::UpdateMapNpc:
			{
				const Babel::SingleIntEvent& evtData = static_cast<const Babel::SingleIntEvent&>(eventData);
				char* arrayStart = (char*)(&eventData) + sizeof(Babel::SingleIntEvent);
				Babel::QuestNpc* npcDataPtr = reinterpret_cast<Babel::QuestNpc*>(arrayStart);
				auto size = (evtData.Value * sizeof(Babel::QuestNpc)) + sizeof(Babel::SingleIntEvent);
				assert(size == evtData.Size);
				UpdateMapNpc(evtData.Value, npcDataPtr);
				break;
			}
			case EventType::UpdateUserPos:
			{
				const Babel::UpdateUserPosEvt& evtData = static_cast<const Babel::UpdateUserPosEvt&>(eventData);
				UpdateUserPos(evtData);
				break;
			}
			case EventType::UpdateGroupMemberPos:
			{
				const Babel::UpdateGroupMemberPosEvt& evtData = static_cast<const Babel::UpdateGroupMemberPosEvt&>(eventData);
				UpdateGroupMemberPos(evtData);
				break;
			}
			case EventType::UpdateKeySlot:
			{
				const Babel::UpdateInvSlot& slotInfo = static_cast<const Babel::UpdateInvSlot&>(eventData);
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(2);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::UpdateInvSlot), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == slotInfo.Size);
				std::string objName(strInfo[0].StartPos, strInfo[0].Size);
				std::string objDesc(strInfo[1].StartPos, strInfo[1].Size);
				UpdateKeySlot(objName, objDesc, slotInfo);
				break;
			}
			case EventType::UpdateIntervals:
			{
				const Babel::UpdateIntervalsEvent& slotInfo = static_cast<const Babel::UpdateIntervalsEvent&>(eventData);
				
				UpdateInterval(slotInfo.Intervals);
				break;
			}
			case EventType::StartInterval:
			{
				const Babel::StartIntervalEvent& slotInfo = static_cast<const Babel::StartIntervalEvent&>(eventData);

				StartInterval(slotInfo.IntervalType, slotInfo.Timestamp);
				break;
			}
			case EventType::UpdateSafeState:
			{
				const Babel::DoubleIntEvent& evtInfo = static_cast<const Babel::DoubleIntEvent&>(eventData);
				UpdatesafeState(evtInfo.Value1, evtInfo.Value2);
				break;
			}
			case EventType::UpdateOnlines:
			{
				const Babel::SingleIntEvent& evtInfo = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateOnlines(evtInfo.Value);
				break;
			}
			case EventType::UpdateGameTime:
			{
				const Babel::DoubleIntEvent& evtInfo = static_cast<const Babel::DoubleIntEvent&>(eventData);
				UpdateGameTime(evtInfo.Value1, evtInfo.Value2);
				break;
			}
			case EventType::UpdateIsGameMaster:
			{
				const Babel::SingleIntEvent& evtInfo = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateIsGameMaster(evtInfo.Value);
				break;
			}
			case EventType::UpdateMagicAttack:
			{
				const Babel::SingleIntEvent& evtInfo = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateMagicAttack(evtInfo.Value);
				break;
			}
			case EventType::UpdateMagicResistance:
			{
				const Babel::SingleIntEvent& evtInfo = static_cast<const Babel::SingleIntEvent&>(eventData);
				UpdateMagicResistance(evtInfo.Value);
				break;
			}
			case EventType::SetWhisperTarget:
			{
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::Event), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == eventData.Size);
				std::string targetName(strInfo[0].StartPos, strInfo[0].Size);
				UpdateWhisperTarget(targetName);
				break;
			}
			case EventType::PasteText:
			{
				std::vector<StringInBuffer> strInfo;
				strInfo.resize(1);
				const char* output = GetStringPtrInEvent((char*)(&eventData), sizeof(Babel::Event), strInfo);
				auto size = output - (char*)(&eventData);
				assert(size == eventData.Size);
				std::string pasteData(strInfo[0].StartPos, strInfo[0].Size);
				HandleTextPaste(pasteData);
				break;
			}
			case EventType::ReloadSettings:
			{
				UpdateGameSettings();
				break;
			}
			case EventType::SetRemoteTrackingState:
			{
				const Babel::SingleIntEvent& evtInfo = static_cast<const Babel::SingleIntEvent&>(eventData);
				SetRemoteTrackingState(evtInfo.Value);
				break;
			}
			case EventType::RemoteInvSpellState:
			{
				const Babel::TripleIntEvent& evtInfo = static_cast<const Babel::TripleIntEvent&>(eventData);
				SetRemoteInvstate(evtInfo.Value1, evtInfo.Value2, evtInfo.Value3);
				break;
			}
			case EventType::RemoteUserClick:
			{
				RemoteUserClick();
				break;
			}
			case EventType::UpdateRemoteMousePos:
			{
				const Babel::DoubleIntEvent& evtInfo = static_cast<const Babel::DoubleIntEvent&>(eventData);
				UpdateRemoteMousePos(evtInfo.Value1, evtInfo.Value2);
				break;
			}
			case EventType::StartSpellCd:
			{
				const Babel::DoubleIntEvent& evtInfo = static_cast<const Babel::DoubleIntEvent&>(eventData);
				StartSpellCd(evtInfo.Value1, evtInfo.Value2);
				break;
			}
			case EventType::UpdateCombatAndglobalChatSettings:
			{
				const Babel::DoubleIntEvent& evtInfo = static_cast<const Babel::DoubleIntEvent&>(eventData);
				UpdateCombatAndGlobalChatSettings(evtInfo.Value1, evtInfo.Value2);
				break;
			}			
			default:
				break;
		}
	}

	ultralight::JSValue JSBridge::LogIn(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 3)
		{
			return "invalid params";
		}
		ultralight::String user = args[0];
		ultralight::String password = args[1];
		bool storeCredentials = args[2];

		LoginCredentialsEvent loginEvent;
		auto usr = user.utf8();
		auto pwd = password.utf8();
		loginEvent.SetUserAndPassword(usr.data(), static_cast<int>((int)usr.length()), pwd.data(), (int)pwd.length());
		loginEvent.storeCredentials = storeCredentials;
		loginEvent.Size = sizeof(LoginCredentialsEvent);
		loginEvent.EventType = EventType::Login;
		mEventBuffer.AddEvent((uint8_t*)&loginEvent, loginEvent.Size);
		return JSValue(true);
	}
	ultralight::JSValue JSBridge::CreateAccount(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 4)
		{
			return "invalid params";
		}
		ultralight::String user = args[0];
		ultralight::String password = args[1];
		ultralight::String uname = args[2];
		ultralight::String usurname = args[3];

		NewAccountEvent newAccountEvent;
		auto usr = user.utf8();
		auto pwd = password.utf8();
		auto name = uname.utf8();
		auto surname = usurname.utf8();
		newAccountEvent.SetUserAndPassword(usr.data(), static_cast<int>((int)usr.length()), pwd.data(), (int)pwd.length(),
										  name.data(), (int)name.length(), surname.data(), (int)surname.length());
		newAccountEvent.Size = sizeof(NewAccountEvent);
		newAccountEvent.EventType = EventType::CreateAccount;
		mEventBuffer.AddEvent((uint8_t*)&newAccountEvent, newAccountEvent.Size);
		return JSValue(true);
	}
	void JSBridge::CloseClient(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		Event closeClient;
		closeClient.EventType = EventType::CloseClient;
		closeClient.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&closeClient, closeClient.Size);
	}
	ultralight::JSValue JSBridge::GetStoredCredentials(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		auto path = GetFilePath("OUTPUT/Cuenta.ini");
		INIReader Reader(path.u8string());
		auto account = Reader.Get("CUENTA", "Nombre", "");
		auto password = Reader.Get("CUENTA", "Password", "");
		password = Decode(password, "9256");

		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> user =
			adopt(JSStringCreateWithUTF8CString(account.c_str()));
		JSRetainPtr<JSStringRef> pwd =
			adopt(JSStringCreateWithUTF8CString(password.c_str()));
		JSRetainPtr<JSStringRef> userParam =
			adopt(JSStringCreateWithUTF8CString("user"));
		JSRetainPtr<JSStringRef> pwdParam =
			adopt(JSStringCreateWithUTF8CString("password"));
		auto juser = JSValueMakeString(ctx, user.get());
		auto jpwd = JSValueMakeString(ctx, pwd.get());
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectSetProperty(ctx, ret, userParam.get(), juser, 0, nullptr);
		JSObjectSetProperty(ctx, ret, pwdParam.get(), jpwd, 0, nullptr);
		return ret;
	}
	ultralight::JSValue JSBridge::ResendValidationCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String email = args[0];
		auto usr = utf8_to_ascii(email.utf8().data());
		
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::ResendValidationCode;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = usr.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue JSBridge::ValidateCode(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return "invalid params";
		}
		ultralight::String jemail = args[0];
		ultralight::String jcode = args[1];

		auto usr = utf8_to_ascii(jemail.utf8().data());
		auto code = utf8_to_ascii(jcode.utf8().data());

		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::ValidateCode;
		std::vector<StringInBuffer> strInfo(2);
		strInfo[0].StartPos = usr.c_str();
		strInfo[1].StartPos = code.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue JSBridge::SetHost(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String jenv= args[0];
		auto envornment = utf8_to_ascii(jenv.utf8().data());
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::SetHost;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = envornment.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue JSBridge::RequestPasswordReset(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}
		ultralight::String email = args[0];

		auto usr = utf8_to_ascii(email.utf8().data());
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::RequestPasswordReset;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = usr.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue JSBridge::NewPasswordRequest(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 3)
		{
			return "invalid params";
		}
		ultralight::String jemail = args[0];
		ultralight::String jcode = args[1];
		ultralight::String jpassword = args[2];

		auto usr = utf8_to_ascii(jemail.utf8().data());
		auto code = utf8_to_ascii(jcode.utf8().data());
		auto pwd = utf8_to_ascii(jpassword.utf8().data());

		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::NewPasswordRequest;
		std::vector<StringInBuffer> strInfo(3);
		strInfo[0].StartPos = usr.c_str();
		strInfo[1].StartPos = code.c_str();
		strInfo[2].StartPos = pwd.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
		return JSValue(true);
	}

	ultralight::JSValue JSBridge::GetCharacterDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 5)
		{
			return "invalid params";
		}

		int body = args[0];
		int head = args[1];
		int helm = args[2];
		int shield = args[3];
		int weapon = args[4];
		AO::CharacterRenderInfo charData;
		mResources->GetBodyInfo(charData, body, head, helm, shield, weapon);

		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef character = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef bodyObj = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef bodyGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef headGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef helmGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef waponGrh = JSObjectMake(ctx, nullptr, nullptr);
		JSObjectRef shieldGrh = JSObjectMake(ctx, nullptr, nullptr);
		SetGrhJsObject(ctx, bodyGrh, charData.Body.image);
		SetChildObject(ctx, bodyObj, "body", bodyGrh);
		SetObjectNumber(ctx, bodyObj, "HeadOffsetY", charData.Body.HeadOffset.Y);
		SetObjectNumber(ctx, bodyObj, "HeadOffsetX", charData.Body.HeadOffset.X);
		SetChildObject(ctx, character, "body", bodyObj);
		SetGrhJsObject(ctx, headGrh, charData.Head);
		SetGrhJsObject(ctx, helmGrh, charData.Helm);
		SetGrhJsObject(ctx, waponGrh, charData.Weapon);
		SetGrhJsObject(ctx, shieldGrh, charData.Shield);
		SetChildObject(ctx, character, "head", headGrh);
		SetChildObject(ctx, character, "helm", helmGrh);
		SetChildObject(ctx, character, "weapon", waponGrh);
		SetChildObject(ctx, character, "shield", shieldGrh);
		return character;
	}

	ultralight::JSValue JSBridge::GetHeadDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		AO::GrhDetails headInfo;

		mResources->GetHeadInfo(headInfo, args[0]);
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef headGrh = JSObjectMake(ctx, nullptr, nullptr);
		SetGrhJsObject(ctx, headGrh, headInfo);
		return headGrh;
	}

	ultralight::JSValue JSBridge::GetGrhDrawInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		AO::GrhDetails grhInfo;

		mResources->GetGrhInfo(grhInfo, args[0]);
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef grh = JSObjectMake(ctx, nullptr, nullptr);
		SetGrhJsObject(ctx, grh, grhInfo);
		return grh;
	}

	ultralight::JSValue JSBridge::SelectCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		SelectCharacterEvent selectCharEvent;
		selectCharEvent.CharIndex = args[0];
		selectCharEvent.EventType = EventType::SelectCharacter;
		selectCharEvent.Size = sizeof(SelectCharacterEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectCharEvent, sizeof(selectCharEvent));
		return ultralight::JSValue();
	}

	ultralight::JSValue JSBridge::LoginCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		SelectCharacterEvent selectCharEvent;
		selectCharEvent.CharIndex = args[0];
		selectCharEvent.EventType = EventType::LoginCharacter;
		selectCharEvent.Size = sizeof(SelectCharacterEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectCharEvent, sizeof(selectCharEvent));
		return ultralight::JSValue();
	}

	ultralight::JSValue JSBridge::CreateCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 6)
		{
			return "invalid params";
		}
		ultralight::String jName = args[0];
		NewCharacterEvent charInfo;
		charInfo.Gender = args[1];
		charInfo.Race = args[2];
		charInfo.Head = args[3];
		charInfo.Class = args[4];
		charInfo.HomeCity = args[5];
		auto name = utf8_to_ascii(jName.utf8().data());
		charInfo.EventType = Babel::EventType::CreateCharacter;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = name.c_str();
		charInfo.Size = sizeof(charInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&charInfo, sizeof(charInfo), strInfo);
		return ultralight::JSValue();
	}

	ultralight::JSValue JSBridge::GetStoredLocale(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 0)
		{
			return "invalid params";
		}
		auto path = GetFilePath("OUTPUT/Configuracion.ini");
		INIReader Reader(path.u8string());
		auto activeLanguange = Reader.GetInteger("OPCIONES", "Localization", 1);

		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> lang;
		if (activeLanguange == 2)
		{
			return "en";
		}
		else
		{
			return "es";
		}
	}

	ultralight::JSValue JSBridge::EnableDebug(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		return mApplication.GetSettings().EnableDebug;
	}

	void JSBridge::ExitCharacterSelection(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 0)
		{
			return;
		}
		Event evt;
		evt.EventType = EventType::ReturnToLogin;
		evt.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&evt, evt.Size);
	}

	ultralight::JSValue JSBridge::RequestDeleteCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		SelectCharacterEvent selectCharEvent;
		selectCharEvent.CharIndex = args[0];
		selectCharEvent.EventType = EventType::RequestDeleteCharacter;
		selectCharEvent.Size = sizeof(SelectCharacterEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectCharEvent, sizeof(selectCharEvent));
		return ultralight::JSValue();
	}

	ultralight::JSValue JSBridge::ConfirmDeleteCharacter(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return "invalid params";
		}

		ultralight::String jcode = args[1];

		std::string code = utf8_to_ascii(jcode.utf8().data());
		SelectCharacterEvent confirmDelete;
		confirmDelete.CharIndex = args[0];
		confirmDelete.EventType = EventType::ConfirmDeleteCharacter;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = code.c_str();
		confirmDelete.Size = sizeof(confirmDelete) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&confirmDelete, sizeof(confirmDelete), strInfo);
		return ultralight::JSValue();
	}

	ultralight::JSValue JSBridge::RequestCharacterTransfer(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return "invalid params";
		}

		ultralight::String jemail = args[1];

		std::string email = utf8_to_ascii(jemail.utf8().data());
		SelectCharacterEvent transferChar;
		transferChar.CharIndex = args[0];
		transferChar.EventType = EventType::RequestTransferCharacter;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = email.c_str();
		transferChar.Size = sizeof(transferChar) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&transferChar, sizeof(transferChar), strInfo);
		return ultralight::JSValue();
	}

	void JSBridge::Copytext(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		ultralight::String jenv = args[0];
		auto message = utf8_to_ascii(jenv.utf8().data());
		CopyToClipboard(message);
	}

	void JSBridge::SendChatMsg(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		ultralight::String jenv = args[0];
		auto message = utf8_to_ascii(jenv.utf8().data());
		if (message.empty())
		{
			return;
		}
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::SendConsoleMsgToVB;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = message.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
	}

	void JSBridge::OpenVBDialog(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		ultralight::String jenv = args[0];
		auto envornment = utf8_to_ascii(jenv.utf8().data());
		Babel::Event eventInfo;
		eventInfo.EventType = Babel::EventType::ShowVBDialog;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = envornment.c_str();
		eventInfo.Size = sizeof(eventInfo) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&eventInfo, sizeof(eventInfo), strInfo);
	}

	void JSBridge::UpdateSelectedInvSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent selectInvSlotEvent;
		selectInvSlotEvent.Value = args[0];
		selectInvSlotEvent.EventType = EventType::SelectInvSlot;
		selectInvSlotEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectInvSlotEvent, sizeof(selectInvSlotEvent));
		return;
	}

	void JSBridge::UseInvSlotIndex(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent userSlotEvent;
		userSlotEvent.Value = args[0];
		userSlotEvent.EventType = EventType::UseInvSlot;
		userSlotEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&userSlotEvent, sizeof(userSlotEvent));
		return;
	}

	void JSBridge::UpdateSelectedSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent selectSpellSlotEvent;
		selectSpellSlotEvent.Value = args[0];
		selectSpellSlotEvent.EventType = EventType::SelectSpellSlot;
		selectSpellSlotEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&selectSpellSlotEvent, sizeof(selectSpellSlotEvent));
		return;
	}

	void JSBridge::UseSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent useSpellEvent;
		useSpellEvent.Value = args[0];
		useSpellEvent.EventType = EventType::UseSpellSlot;
		useSpellEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&useSpellEvent, sizeof(useSpellEvent));
		return;
	}
	
	void JSBridge::UpdateInputFocus(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleBoolEvent updateInputFocus;
		updateInputFocus.Value = args[0];
		updateInputFocus.EventType = EventType::UpdateInputFocus;
		updateInputFocus.Size = sizeof(SingleBoolEvent);
		mEventBuffer.AddEvent((uint8_t*)&updateInputFocus, sizeof(updateInputFocus));
		return;
	}

	void JSBridge::ClickLink(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		ultralight::String jenv = args[0];
		auto url = utf8_to_ascii(jenv.utf8().data());
		Event clickLink;
		clickLink.EventType = EventType::ClickLink;
		std::vector<StringInBuffer> strInfo(1);
		strInfo[0].StartPos = url.c_str();
		clickLink.Size = sizeof(clickLink) + PrepareDynamicStrings(strInfo);
		mEventBuffer.AddEvent((uint8_t*)&clickLink, sizeof(clickLink), strInfo);
		return;
	}

	void JSBridge::ClickGold(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 0)
		{
			return;
		}

		Event clickGold;
		clickGold.EventType = EventType::ClickGold;
		clickGold.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&clickGold, sizeof(clickGold));
		return;
	}

	void JSBridge::MoveInvItem(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return;
		}

		DoubleIntEvent moveItem;
		moveItem.Value1 = args[0];
		moveItem.Value2 = args[1];
		moveItem.EventType = EventType::MoveInvSlot;
		moveItem.Size = sizeof(Event);
		mEventBuffer.AddEvent((uint8_t*)&moveItem, sizeof(moveItem));
		return;
	}

	void JSBridge::RequestAction(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent requestActionEvent;
		requestActionEvent.Value = args[0];
		requestActionEvent.EventType = EventType::RequestAction;
		requestActionEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&requestActionEvent, sizeof(requestActionEvent));
		return;
	}

	void JSBridge::UseKey(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleIntEvent requestActionEvent;
		requestActionEvent.Value = args[0];
		requestActionEvent.EventType = EventType::UseKey;
		requestActionEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&requestActionEvent, sizeof(requestActionEvent));
		return;
	}

	void JSBridge::MoveSpellSlot(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return;
		}
		DoubleIntEvent requestMoveSpellSlot;
		requestMoveSpellSlot.Value1 = args[0];
		requestMoveSpellSlot.Value2 = args[1];
		requestMoveSpellSlot.EventType = EventType::MoveSpellSlot;
		requestMoveSpellSlot.Size = sizeof(requestMoveSpellSlot);
		mEventBuffer.AddEvent((uint8_t*)&requestMoveSpellSlot, sizeof(requestMoveSpellSlot));
	}

	void JSBridge::DeleteItem(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		SingleIntEvent evtData;
		evtData.Value = args[0];
		evtData.EventType = EventType::DeleteItem;
		evtData.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&evtData, sizeof(evtData));
	}

	void JSBridge::UpdateOpenDialog(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		SingleBoolEvent updateInputFocus;
		updateInputFocus.Value = args[0];
		updateInputFocus.EventType = EventType::UpdateOpenDialog;
		updateInputFocus.Size = sizeof(SingleBoolEvent);
		mEventBuffer.AddEvent((uint8_t*)&updateInputFocus, sizeof(updateInputFocus));
		return;
	}

	ultralight::JSValue JSBridge::GetSpellInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		AO::SpellData spellInfo;

		mResources->GetSpellDetails(spellInfo, args[0]);
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef spell = JSObjectMake(ctx, nullptr, nullptr);
		SetSpellObject(ctx, spell, spellInfo);
		return spell;
	}

	ultralight::JSValue JSBridge::GetItemInfo(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return "invalid params";
		}

		AO::ObjectData objData;
		mResources->GetObjectDetails(objData, args[0]);
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef obj = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectInfo(ctx, obj, objData);
		return obj;
	}

	void JSBridge::LogError(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}
		ultralight::String jenv = args[0];
		auto errorMsg = utf8_to_ascii(jenv.utf8().data());
		Babel::LOGGER->log(errorMsg.c_str());
	}

	void JSBridge::InformSpellListScroll(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 1)
		{
			return;
		}

		AO::ObjectData objData;

		SingleIntEvent scrollSpellEvent;
		scrollSpellEvent.Value = args[0];
		scrollSpellEvent.EventType = EventType::ScrollSpell;
		scrollSpellEvent.Size = sizeof(SingleIntEvent);
		mEventBuffer.AddEvent((uint8_t*)&scrollSpellEvent, sizeof(scrollSpellEvent));
	}

	void JSBridge::ClickMiniMapPos(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return;
		}

		AO::ObjectData objData;

		DoubleIntEvent mapPosEvent;
		mapPosEvent.Value1 = args[0];
		mapPosEvent.Value2 = args[1];
		mapPosEvent.EventType = EventType::TeleportToMinimapPos;
		mapPosEvent.Size = sizeof(mapPosEvent);
		mEventBuffer.AddEvent((uint8_t*)&mapPosEvent, sizeof(mapPosEvent));
	}

	void JSBridge::UpdateCombatAndGlobatChatState(const ultralight::JSObject& thisObject, const ultralight::JSArgs& args)
	{
		if (args.size() != 2)
		{
			return;
		}

		AO::ObjectData objData;

		DoubleIntEvent evtData;
		evtData.Value1 = args[0];
		evtData.Value2 = args[1];
		evtData.EventType = EventType::UpdateCombatAndglobalChatSettings;
		evtData.Size = sizeof(evtData);
		mEventBuffer.AddEvent((uint8_t*)&evtData, sizeof(evtData));
	}
	
	void JSBridge::HandlekeyData(const KeyEvent& keyData)
	{
		ultralight::KeyEvent evt;
		evt.type = static_cast<ultralight::KeyEvent::Type>(keyData.Type);
		evt.modifiers = ConvertKeyModifiers(keyData.ShiftState);
		switch (keyData.Type)
		{
		
		case ultralight::KeyEvent::kType_KeyDown:
			evt.type = ultralight::KeyEvent::kType_RawKeyDown;
			break;
		case ultralight::KeyEvent::kType_RawKeyDown:
		case ultralight::KeyEvent::kType_KeyUp:
			evt.virtual_key_code = keyData.KeyCode;
			evt.native_key_code = 0;
			GetKeyIdentifierFromVirtualKeyCode(evt.virtual_key_code, evt.key_identifier);
			mRenderer.SendKeyEvent(evt, keyData.Inspector);
			break;
		case ultralight::KeyEvent::kType_Char:
		{
			char key = static_cast<char>(keyData.KeyCode);
			evt.text = ascii_to_utf8(std::string(1, key)).c_str();
			evt.unmodified_text = evt.text;
			mRenderer.SendKeyEvent(evt, keyData.Inspector);
			break;
		}
		default:
			break;
		}
	}
	void JSBridge::SendErrorMessage(const ErrorMessageEvent& messageData)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.ErrorMessage"));

		// Evaluate the string "ShowMessage"
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func)) 
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {
				std::string utf8_string = ascii_to_utf8(messageData.StrData);
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(utf8_string.c_str()));

				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()),
											JSValueMakeNumber(ctx, static_cast<double>(messageData.MessageType)),
											JSValueMakeNumber(ctx, static_cast<double>(messageData.Action)) };

				// Count the number of arguments in the array.
				size_t num_args = sizeof(args) / sizeof(JSValueRef*);

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::SetActiveScreen(const std::string& name)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetActiveDialog"));

		// Evaluate the string "ShowMessage"
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(name.c_str()));

				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()) };

				// Count the number of arguments in the array.
				size_t num_args = 1;

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::SetLoadingMessage(const std::string& message, bool localize)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetLoadingMessage"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(ascii_to_utf8(message).c_str()));
				// Create our list of arguments (we only have one)
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()),
											JSValueMakeBoolean(ctx, localize)};

				// Count the number of arguments in the array.
				size_t num_args = 2;

				// Create a place to store an exception, if any
				JSValueRef exception = 0;

				// Call the ShowMessage() function with our list of arguments.
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::HandleLoginCharList(const CharacterListEvent& messageData)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetCharacter"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.
				
				for (int i = 0; i < messageData.CharacterCount; i++)
				{
					JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
					SetObjectString(ctx, ret, "name", ascii_to_utf8(messageData.CharacterList[i].Name).c_str());
					SetObjectNumber(ctx, ret, "head", messageData.CharacterList[i].Head);
					SetObjectNumber(ctx, ret, "body", messageData.CharacterList[i].Body);
					SetObjectNumber(ctx, ret, "helm", messageData.CharacterList[i].Helm);
					SetObjectNumber(ctx, ret, "shield", messageData.CharacterList[i].Shield);
					SetObjectNumber(ctx, ret, "weapon", messageData.CharacterList[i].Weapon);
					SetObjectNumber(ctx, ret, "level", messageData.CharacterList[i].Level);
					SetObjectNumber(ctx, ret, "status", messageData.CharacterList[i].Status);
					SetObjectNumber(ctx, ret, "index", messageData.CharacterList[i].Index);
					SetObjectNumber(ctx, ret, "class", messageData.CharacterList[i].Class);
					const JSValueRef args[] = { ret };
					size_t num_args = 1;
					JSValueRef exception = 0;
					JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
						num_args, args,
						&exception);
				}
			}
		}
	}
	void JSBridge::DeleteCharacterFromList(int characterIndex)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.DeleteCharacterFromList"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) 
			{
				const JSValueRef args[] = { JSValueMakeNumber(ctx, characterIndex) };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}

	void JSBridge::RequestDeleteCode()
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.RequestDeleteCode"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {
				size_t num_args = 0;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0, num_args, nullptr, &exception);
			}
		}
	}
	void JSBridge::UpdateStats(const UserStats& userStats)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.UpdateUserStats"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.

				JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
				SetObjectNumber(ctx, ret, "maxHp", userStats.MaxHp);
				SetObjectNumber(ctx, ret, "minHp", userStats.MinHp);
				SetObjectNumber(ctx, ret, "hpShield", userStats.HpShield);
				SetObjectNumber(ctx, ret, "maxMan", userStats.MaxMAN);
				SetObjectNumber(ctx, ret, "minMan", userStats.MinMAN);
				SetObjectNumber(ctx, ret, "maxSta", userStats.MaxSTA);
				SetObjectNumber(ctx, ret, "minSta", userStats.MinSTA);
				SetObjectNumber(ctx, ret, "maxAgu", userStats.MaxAGU);
				SetObjectNumber(ctx, ret, "minAgu", userStats.MinAGU);
				SetObjectNumber(ctx, ret, "maxHam", userStats.MaxHAM);
				SetObjectNumber(ctx, ret, "minHam", userStats.MinHAM);
				SetObjectNumber(ctx, ret, "gold", userStats.GLD);
				SetObjectNumber(ctx, ret, "level", userStats.Lvl);
				SetObjectNumber(ctx, ret, "class", userStats.Class);
				SetObjectNumber(ctx, ret, "gender", userStats.Gender);
				SetObjectNumber(ctx, ret, "race", userStats.Race);
				SetObjectNumber(ctx, ret, "home", userStats.Home);
				SetObjectNumber(ctx, ret, "nextLevel", userStats.NextLevel);
				SetObjectNumber(ctx, ret, "exp", userStats.Exp);
				SetObjectNumber(ctx, ret, "status", userStats.Status);
				const JSValueRef args[] = { ret };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateUserName(const std::string& name)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetUserName"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {
				JSRetainPtr<JSStringRef> msg =
					adopt(JSStringCreateWithUTF8CString(name.c_str()));
				const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()) };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::SendChatMessage(const std::string& sender, const Color& senderColor, const std::string& text, const Color& textColor, uint8_t textStyle)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.PostChatMsg"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.

				JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);

				JSObjectRef senderColorObj = JSObjectMake(ctx, nullptr, nullptr);
				SetObjectString(ctx, ret, "sender", ascii_to_utf8(sender).c_str());
				SetColorObject(ctx, senderColorObj, senderColor);
				SetChildObject(ctx, ret, "senderColor", senderColorObj);
				
				SetObjectString(ctx, ret, "text", ascii_to_utf8(text).c_str());
				JSObjectRef textColorObj = JSObjectMake(ctx, nullptr, nullptr);
				SetColorObject(ctx, textColorObj, textColor);
				SetChildObject(ctx, ret, "textColor", textColorObj);
				SetObjectNumber(ctx, ret, "textStyle", textStyle);
				SetObjectBoolean(ctx, ret, "italic", (textStyle & (uint8_t)Babel::eTextFormatMask::eItalic) > 0);
				SetObjectBoolean(ctx, ret, "bold", (textStyle & (uint8_t)Babel::eTextFormatMask::eBold) > 0);

				const JSValueRef args[] = { ret };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateFps(int Fps)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.UpdateFps"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj))
			{
				const JSValueRef args[] = { JSValueMakeNumber(ctx, Fps) };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateInvLevel(int level)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.SetInventoryLevel"));

		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj))
			{
				const JSValueRef args[] = { JSValueMakeNumber(ctx, level) };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateInvSlot(const std::string& objName, const std::string& objDesc, const Babel::UpdateInvSlot& slotInfo)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.UpdateInvSlot"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.

				JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);

				JSObjectRef senderColorObj = JSObjectMake(ctx, nullptr, nullptr);
				SetObjectString(ctx, ret, "name", ascii_to_utf8(objName).c_str());
				SetObjectString(ctx, ret, "description", ascii_to_utf8(objDesc).c_str());
				SetObjectNumber(ctx, ret, "count", slotInfo.Amount);
				SetObjectNumber(ctx, ret, "cantUse", slotInfo.CantUse);
				SetObjectBoolean(ctx, ret, "equipped", slotInfo.Equipped > 0);
				SetObjectNumber(ctx, ret, "grh", slotInfo.GrhIndex);
				SetObjectNumber(ctx, ret, "maxDef", slotInfo.MaxDef);
				SetObjectNumber(ctx, ret, "minDef", slotInfo.MinDef);
				SetObjectNumber(ctx, ret, "maxHit", slotInfo.MaxHit);
				SetObjectNumber(ctx, ret, "minHit", slotInfo.MinHit);
				SetObjectNumber(ctx, ret, "objIndex", slotInfo.ObjIndex);
				SetObjectNumber(ctx, ret, "index", slotInfo.Slot);
				SetObjectNumber(ctx, ret, "type", slotInfo.ObjType);
				SetObjectNumber(ctx, ret, "value", slotInfo.Value);
				SetObjectNumber(ctx, ret, "cooldown", slotInfo.Cooldown);
				SetObjectNumber(ctx, ret, "cdType", slotInfo.CDType);
				SetObjectNumber(ctx, ret, "cdMask", slotInfo.CDMask);
				SetObjectNumber(ctx, ret, "amunition", slotInfo.Amunition);

				const JSValueRef args[] = { ret };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateSpellSlot(const std::string& objName, const Babel::UpdateSpellSlot& slotInfo)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString("APicallbacks.UpdateSpellSlot"));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			// Cast 'func' to an Object, will return null if typecast failed.
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

			// Check if 'funcObj' is a Function and not null
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {

				// Create a JS string from null-terminated UTF8 C-string, store it
				// in a smart pointer to release it when it goes out of scope.

				JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);

				JSObjectRef senderColorObj = JSObjectMake(ctx, nullptr, nullptr);
				SetObjectString(ctx, ret, "name", ascii_to_utf8(objName).c_str());
				SetObjectNumber(ctx, ret, "index", slotInfo.Slot);
				SetObjectNumber(ctx, ret, "spellIndex", slotInfo.SpellIndex);
				SetObjectNumber(ctx, ret, "grh", slotInfo.IconIndex);
				SetObjectNumber(ctx, ret, "cooldown", slotInfo.Cooldown);

				const JSValueRef args[] = { ret };
				size_t num_args = 1;
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					num_args, args,
					&exception);
			}
		}
	}
	void JSBridge::UpdateHp(int32_t newHp, int32_t newShield)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectNumber(ctx, ret, "hp", newHp);
		SetObjectNumber(ctx, ret, "shield", newShield);
		const JSValueRef args[] = { ret };
		CallJsFunction(ctx, "APicallbacks.UpdateHp", args, 1);
	}

	void JSBridge::UpdateMana(int32_t newMana)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newMana) };
		CallJsFunction(ctx, "APicallbacks.UpdateMana", args, 1);
	}
	void JSBridge::UpdateStamina(int32_t newStamina)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newStamina) };
		CallJsFunction(ctx, "APicallbacks.UpdateStamina", args, 1);
	}
	void JSBridge::UpdateDrink(int32_t newDrink)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newDrink) };
		CallJsFunction(ctx, "APicallbacks.UpdateDrink", args, 1);
	}
	void JSBridge::UpdateFood(int32_t newFood)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newFood) };
		CallJsFunction(ctx, "APicallbacks.UpdateFood", args, 1);
	}

	void JSBridge::UpdateGold(int32_t newGold)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newGold) };
		CallJsFunction(ctx, "APicallbacks.UpdateGold", args, 1);
	}

	void JSBridge::UpdateExp(int32_t current, int32_t maxExp)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, current),
									JSValueMakeNumber(ctx, maxExp) };
		CallJsFunction(ctx, "APicallbacks.UpdateExp", args, 2);
	}

	void JSBridge::OpenChat(int32_t mode)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, mode) };
		CallJsFunction(ctx, "APicallbacks.OpenChat", args, 1);
	}

	void JSBridge::UpdateStrAndAgi(const UpdateAgiAndStr& updatedInfo)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, updatedInfo.Str),
									JSValueMakeNumber(ctx, updatedInfo.Agi),
									JSValueMakeNumber(ctx, updatedInfo.StrState),
									JSValueMakeNumber(ctx, updatedInfo.AgiState) };
		CallJsFunction(ctx, "APicallbacks.UpdateStrAndAgi", args, 4);
	}

	void JSBridge::UpdateMapName(int32_t mapNumber, const std::string& mapName, int32_t isSafe)
	{
		
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> msg =
			adopt(JSStringCreateWithUTF8CString(mapName.c_str()));
		const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()),
									JSValueMakeNumber(ctx, mapNumber),
									JSValueMakeBoolean(ctx, isSafe > 0)};
		CallJsFunction(ctx, "APicallbacks.UpdateMapNumber", args, 3);
	}

	void JSBridge::UpdateMapNpc(int32_t npcCount, const Babel::QuestNpc* npcList)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		std::vector<JSObjectRef> NpcList;
		NpcList.resize(npcCount);
		for (int i = 0; i < npcCount; i++)
		{
			NpcList[i] = JSObjectMake(ctx, nullptr, nullptr);
			JSObjectRef pos = JSObjectMake(ctx, nullptr, nullptr);
			SetObjectNumber(ctx, pos, "tileX", npcList[i].Position.TileX);
			SetObjectNumber(ctx, pos, "tileY", npcList[i].Position.TileY);
			SetChildObject(ctx, NpcList[i], "position", pos);
			SetObjectNumber(ctx, NpcList[i], "npcNumber", npcList[i].NpcNumber);
			SetObjectNumber(ctx, NpcList[i], "state", npcList[i].State);
		}
		JSObjectRef jsArray = JSObjectMakeArray(ctx, npcCount, NpcList.data(),0);
		const JSValueRef args[] = { jsArray };
		CallJsFunction(ctx, "APicallbacks.UpdateMapNpc", args, 1);
	}

	void JSBridge::UpdateUserPos(const Babel::UpdateUserPosEvt& updatePos)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, updatePos.TileX),
									JSValueMakeNumber(ctx, updatePos.TileY),
									JSValueMakeNumber(ctx, updatePos.MapPos.TileX),
									JSValueMakeNumber(ctx, updatePos.MapPos.TileY) };
		CallJsFunction(ctx, "APicallbacks.UpdatePlayerCoord", args, 4);
	}

	void JSBridge::UpdateGroupMemberPos(const Babel::UpdateGroupMemberPosEvt& memberPos)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, memberPos.MapPos.TileX),
									JSValueMakeNumber(ctx, memberPos.MapPos.TileY),
									JSValueMakeNumber(ctx, memberPos.GroupIndex)};
		CallJsFunction(ctx, "APicallbacks.UpdateGroupMarker", args, 3);
	}

	void JSBridge::UpdateKeySlot(const std::string& objName, const std::string& objDesc, const Babel::UpdateInvSlot& slotInfo)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectString(ctx, ret, "name", ascii_to_utf8(objName).c_str());
		SetObjectString(ctx, ret, "description", ascii_to_utf8(objDesc).c_str());
		SetObjectNumber(ctx, ret, "count", slotInfo.Amount);
		SetObjectNumber(ctx, ret, "cantUse", slotInfo.CantUse);
		SetObjectBoolean(ctx, ret, "equipped", slotInfo.Equipped > 0);
		SetObjectNumber(ctx, ret, "grh", slotInfo.GrhIndex);
		SetObjectNumber(ctx, ret, "maxDef", slotInfo.MaxDef);
		SetObjectNumber(ctx, ret, "minDef", slotInfo.MinDef);
		SetObjectNumber(ctx, ret, "maxHit", slotInfo.MaxHit);
		SetObjectNumber(ctx, ret, "minHit", slotInfo.MinHit);
		SetObjectNumber(ctx, ret, "objIndex", slotInfo.ObjIndex);
		SetObjectNumber(ctx, ret, "index", slotInfo.Slot);
		SetObjectNumber(ctx, ret, "type", slotInfo.ObjType);
		SetObjectNumber(ctx, ret, "value", slotInfo.Value);
		SetObjectNumber(ctx, ret, "cooldown", slotInfo.Cooldown);
		SetObjectNumber(ctx, ret, "cdType", slotInfo.CDType);
		SetObjectNumber(ctx, ret, "cdMask", slotInfo.CDMask);

		const JSValueRef args[] = { ret };
		CallJsFunction(ctx, "APicallbacks.UpdateKeySlot", args, 1);
	}

	void JSBridge::UpdateInterval(const Intervals& intervals)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectNumber(ctx, ret, "bow", intervals.Bow);
		SetObjectNumber(ctx, ret, "hit", intervals.Hit);
		SetObjectNumber(ctx, ret, "magic", intervals.Magic);
		SetObjectNumber(ctx, ret, "buildWork", intervals.BuildWork);
		SetObjectNumber(ctx, ret, "dropItem", intervals.DropItem);
		SetObjectNumber(ctx, ret, "extractWork", intervals.ExtractWork);
		SetObjectNumber(ctx, ret, "hitMagic", intervals.HitMagic);
		SetObjectNumber(ctx, ret, "hitUseItem", intervals.HitUseItem);
		SetObjectNumber(ctx, ret, "magicHit", intervals.MagicHit);
		SetObjectNumber(ctx, ret, "useItemClick", intervals.UseItemClick);
		SetObjectNumber(ctx, ret, "useItemKey", intervals.UseItemKey);
		SetObjectNumber(ctx, ret, "walk", intervals.Walk);

		const JSValueRef args[] = { ret };
		CallJsFunction(ctx, "APicallbacks.UpdateIntervals", args, 1);
	}

	void JSBridge::StartInterval(int32_t intervalType, int64_t timestamp)
	{
		using namespace std::chrono;
		int64_t currentTimestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, intervalType),
									JSValueMakeNumber(ctx, currentTimestamp - timestamp)};
		CallJsFunction(ctx, "APicallbacks.StartInterval", args, 2);
	}

	void JSBridge::UpdatesafeState(int32_t type, int32_t state)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, type),
									JSValueMakeBoolean(ctx, state > 0)};
		CallJsFunction(ctx, "APicallbacks.UpdateLockState", args, 2);
	}

	void JSBridge::UpdateOnlines(int32_t onlines)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, onlines) };
		CallJsFunction(ctx, "APicallbacks.UpdateOnlines", args, 1);
	}

	void JSBridge::UpdateGameTime(int32_t hour, int32_t minutes)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, hour), 
									JSValueMakeNumber(ctx, minutes)};
		CallJsFunction(ctx, "APicallbacks.UpdateGameTime", args, 2);
	}

	void JSBridge::UpdateIsGameMaster(int32_t state)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeBoolean(ctx, state > 0)};
		CallJsFunction(ctx, "APicallbacks.UpdateIsGameGaster", args, 1);
	}

	void JSBridge::UpdateMagicResistance(int32_t value)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, value) };
		CallJsFunction(ctx, "APicallbacks.UpdateMagicResistance", args, 1);
	}

	void JSBridge::UpdateMagicAttack(int32_t value)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		const JSValueRef args[] = { JSValueMakeNumber(ctx, value) };
		CallJsFunction(ctx, "APicallbacks.UpdateMagicAttack", args, 1);
	}

	void JSBridge::UpdateWhisperTarget(const std::string& target)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> msg =
			adopt(JSStringCreateWithUTF8CString(target.c_str()));
		const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()) };
		CallJsFunction(ctx, "APicallbacks.SetWhisperTarget", args, 1);
	}

	void JSBridge::HandleTextPaste(const std::string& data)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		JSRetainPtr<JSStringRef> msg =
			adopt(JSStringCreateWithUTF8CString(data.c_str()));
		const JSValueRef args[] = { JSValueMakeString(ctx, msg.get()) };
		CallJsFunction(ctx, "APicallbacks.PasteText", args, 1);
	}

	void JSBridge::UpdateGameSettings()
	{
		auto path = GetFilePath("OUTPUT/Configuracion.ini");
		INIReader Reader(path.u8string());
		auto chatBombat = Reader.GetInteger("OPCIONES", "CHATCOMBATE", 1);
		auto spellMode = Reader.GetInteger("OPCIONES", "ModoHechizos", 1); 
		auto inventoryFullNumbers = Reader.GetInteger("OPCIONES", "NumerosCompletosInventario", 1);
		auto scrollDrag = Reader.GetInteger("OPCIONES", "ScrollArrastrar", 1); 
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		
		JSObjectRef ret = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectNumber(ctx, ret, "combatChat", chatBombat);
		SetObjectNumber(ctx, ret, "spellMode", spellMode);
		SetObjectNumber(ctx, ret, "inventoryFullNumbers", inventoryFullNumbers);
		SetObjectNumber(ctx, ret, "scrollDrag", scrollDrag);

		const JSValueRef args[] = { ret };
		CallJsFunction(ctx, "APicallbacks.ReloadSettings", args, 1);
	}

	void JSBridge::SetRemoteTrackingState(int newState)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();
		
		const JSValueRef args[] = { JSValueMakeNumber(ctx, newState) };
		CallJsFunction(ctx, "APicallbacks.SetRemoteTrackingState", args, 1);
	}

	void JSBridge::SetRemoteInvstate(int selectedTab, int selectedSpell, int firstListSpellInView)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();

		const JSValueRef args[] = { JSValueMakeNumber(ctx, selectedTab),
									JSValueMakeNumber(ctx, selectedSpell),
									JSValueMakeNumber(ctx, firstListSpellInView) };
		CallJsFunction(ctx, "APicallbacks.SetRemoteInvstate", args, 3);
	}

	void JSBridge::RemoteUserClick()
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();

		const JSValueRef args {};
		CallJsFunction(ctx, "APicallbacks.RemoteUserClick", &args, 0);
	}

	void JSBridge::UpdateRemoteMousePos(int posX, int posY)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();

		const JSValueRef args[] = { JSValueMakeNumber(ctx, posX),
									JSValueMakeNumber(ctx, posY) };
		CallJsFunction(ctx, "APicallbacks.UpdateRemoteMousePos", args, 2);
	}

	void JSBridge::StartSpellCd(int spellId, int cdTime)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();

		const JSValueRef args[] = { JSValueMakeNumber(ctx, spellId),
									JSValueMakeNumber(ctx, cdTime) };
		CallJsFunction(ctx, "APicallbacks.StartSpellCd", args, 2);
	}

	void JSBridge::UpdateCombatAndGlobalChatSettings(int combatValue, int globalValue)
	{
		RefPtr<JSContext> context = mRenderer.GetMainView()->LockJSContext();
		JSContextRef ctx = context->ctx();

		const JSValueRef args[] = { JSValueMakeNumber(ctx, combatValue),
									JSValueMakeNumber(ctx, globalValue) };
		CallJsFunction(ctx, "APicallbacks.UpdateCombatAndGlobalChatModes", args, 2);
	}

	void JSBridge::CallJsFunction(JSContextRef& ctx, const char* functionName, const JSValueRef* args, int argCount)
	{
		JSRetainPtr<JSStringRef> str = adopt(
			JSStringCreateWithUTF8CString(functionName));
		JSValueRef func = JSEvaluateScript(ctx, str.get(), 0, 0, 0, 0);
		if (JSValueIsObject(ctx, func))
		{
			JSObjectRef funcObj = JSValueToObject(ctx, func, 0);
			if (funcObj && JSObjectIsFunction(ctx, funcObj)) {
				JSValueRef exception = 0;
				JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0,
					argCount, args,
					&exception);
			}
		}
	}
}