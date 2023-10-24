#pragma once

#include "EventHandler.hpp"
#include "CommonDefines.hpp"

namespace Babel {

	struct UpdateUserStats : public Event
	{
		UserStats UserStats;
	};

	enum class  eTextFormatMask : uint8_t
	{
		eBold = 1,
		eItalic = 1 << 1
	};

	struct ChatMessageEvent : public Event
	{
		Color SenderColor;
		Color TextColor;
		uint8_t TextStyle;
	};

	struct UpdateFps : public Event 
	{
		int Fps;
	};

	struct SetInvLevel : public Event
	{
		int Level;
	};

	struct UpdateInvSlot : public Event
	{
		uint8_t Slot;
		int16_t ObjIndex;
		int32_t GrhIndex;
		uint8_t ObjType;
		uint8_t Equipped;
		uint8_t CantUse;
		int16_t Amount;
		int16_t MinHit;
		int16_t MaxHit;
		int16_t MinDef;
		int16_t MaxDef;
		float Value;
		int32_t Cooldown;
		int16_t CDType;
		int32_t CDMask;
		int16_t Amunition;
		uint8_t IsBindable;
	};

	struct UpdateSpellSlot : public Event
	{
		uint8_t Slot;
		int16_t SpellIndex;
		int32_t IconIndex;
		int32_t Cooldown;
		uint8_t IsBindable;
	};

	struct UpdateHp : public Event
	{
		int32_t NewHpValue;
		int32_t NewShieldValue;
	};

	struct UpdateAgiAndStr : public Event
	{
		uint8_t Str;
		uint8_t Agi;
		uint8_t StrState;
		uint8_t AgiState;
	};

	struct UpdateUserPosEvt : public Event
	{
		int16_t TileX;
		int16_t TileY;
		Position MapPos;
	};
	struct UpdateGroupMemberPosEvt : public Event
	{;
		int16_t GroupIndex;
		Position MapPos;
	};

	struct UpdateIntervalsEvent : public Event
	{
		Intervals Intervals;
	};

	struct StartIntervalEvent : public Event
	{
		int32_t IntervalType;
		int64_t Timestamp;
	};

	struct UpdateHotkeySlot : public Event
	{
		HotkeySlot SlotInfo;
		int SlotIndex;
	};

	struct AoShop : public Event
	{
		int32_t AvailableCredits;
	};

	struct LobbyDataUpdate : public Event
	{
		int16_t Index;
		int16_t Id;
		int16_t MinLevel;
		int16_t MaxLevel;
		int16_t MinPlayers;
		int16_t MaxPlayers;
		int16_t RegisteredPlayers;
		int16_t TeamSize;
		int16_t TeamType;
		uint8_t IsPrivate;
		int32_t InscriptionPrice;
	};

	struct CreateNewEvent : public Event
	{
		ScenarioSettings Settings;
	};
}