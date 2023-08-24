#pragma once

namespace Babel
{
	struct Settings {
		int Width;
		int Height;
		int Compmpressed;
		int EnableDebug;
	};

	struct UserStats
	{
		int16_t MaxHp;
		int16_t MinHp;
		int32_t HpShield;
		int16_t MaxMAN;
		int16_t MinMAN;
		int16_t MaxSTA;
		int16_t MinSTA;
		uint8_t MaxAGU;
		uint8_t MinAGU;
		uint8_t MaxHAM;
		uint8_t MinHAM;
		int32_t GLD;
		int32_t OroPorNivel;
		int16_t Lvl;
		uint8_t Class;
		uint8_t Gender;
		uint8_t Race;
		uint8_t Home;
		int32_t NextLevel;
		int32_t Exp;
		uint8_t Status; // 0 = Alive & 1 = Death
	};

	struct Color
	{
		uint8_t R;
		uint8_t G;
		uint8_t B;
	};

	struct ChatMessage
	{
		const char* Sender;
		Color SenderColor;
		const char* Text;
		Color TextColor;
		uint8_t BoldText;
		uint8_t ItalicText;
	};

	struct InvItem
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
		int32_t Value;
		int32_t Cooldown;
		int16_t CDType;
		int32_t CDMask;
		int16_t Amunition;
		uint8_t IsBindable;
		const char* Name;
		const char* Desc;
	};

	struct SpellEntry
	{
		uint8_t Slot;
		int16_t SpellIndex;
		int32_t Icon;
		int32_t Cooldown;
		uint8_t IsBindable;
		const char* Name;
	};

	struct Position
	{
		float TileX;
		float TileY;
	};

	struct QuestNpc
	{
		Position Position;
		int16_t NpcNumber;
		int16_t State;
	};

	struct Intervals
	{
		int32_t Hit;
		int32_t Bow;
		int32_t Magic;
		int32_t ExtractWork;
		int32_t BuildWork;
		int32_t Walk;
		int32_t DropItem;
		int32_t UseItemKey;
		int32_t UseItemClick;
		int32_t HitMagic;
		int32_t MagicHit;
		int32_t HitUseItem;
	};

	struct HotkeySlot
	{
		int16_t Type;
		int16_t Index;
		int16_t LastKnownslot;
	};
}