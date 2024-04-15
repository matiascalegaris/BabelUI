#pragma once
#include <memory>
#include <map>
#include <string>

namespace AO
{
	struct Vector2
	{
		int X;
		int Y;
	};
	struct GrhDetails
	{
		int ImageNumber;
		Vector2 StartPos;
		Vector2 EndPos;
	};

	struct Body
	{
		GrhDetails image;
		Vector2 HeadOffset;
	};

	struct CharacterRenderInfo
	{
		Body Body;
		GrhDetails Head;
		GrhDetails Helm;
		GrhDetails Weapon;
		GrhDetails Shield;
	};

	struct SpellData
	{
		std::string Name;           // Indice del grafico que representa el obj
		std::string Description;
		int RequiredMana;
		int8_t RequiredSkill;
		int RequiredStamina;
		int IconIndex;
		int Cooldown;
	};

	struct NpcInfo
	{
		std::string Name;
		int Body;
		int Head;
		int Exp;
		int Hp;
		int MaxDamage;
		int MinDamage;
		int Gold;
		int DropCount;
		std::vector<int> DropList;
		int ClanExp;
		float DropRate;
	};

	struct MapNpcInfo
	{
		int32_t Count;
		int16_t NpcIndex;
		NpcInfo Details;
	};
	struct MapInfo
	{
		bool IsSafe;
		std::map<int, MapNpcInfo> NpcList;
		std::string Name;
	};

	struct World
	{
		std::map<int, MapInfo> MapDetails;
		std::vector<int> MapList;
		int Width;
		int Height;
	};

	struct WorldMap
	{
		std::vector<World> Worlds;
	};

	struct ObjectData
	{
		long grhindex; // Graphic index representing the object
		std::string Name;
		std::string Description;
		int MinDef; // Minimum defense
		int MaxDef; // Maximum defense
		int MinHit; // Minimum hit
		int MaxHit; // Maximum hit
		unsigned char ObjType; // Object type
		std::string CreatesLight; // Create light
		int CreateFloorParticle; // Create floor particle
		std::string CreatesGRH; // Create GRH (Graphic)
		int Roots; // Roots
		int Wood; // Wood
		int ElvenWood; // Elven wood
		int WolfSkin; // Wolf skin
		int BrownBearSkin; // Brown bear skin
		int PolarBearSkin; // Polar bear skin
		int LingH; 
		int LingP; 
		int LingO; 
		unsigned char Destroy; // Destroy
		unsigned char Projectile; // Projectile
		unsigned char Ammunition; // Ammunition
		unsigned char BlacksmithingSkill; // Blacksmithing skill
		unsigned char PotionSkill; // Potion skill
		unsigned char TailoringSkill; 
		long Value; // Value
		bool Grabbable; // Grabbable
		int Key; // Key
		long Cooldown;
		int CdType; // Cooldown type
		int SpellIndex;
	};

	class ResourceLoader;
	class Resources
	{
	public:
		Resources(bool compresed);
		~Resources();

		void GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex);
		void GetHeadInfo(GrhDetails& headInfo, int headIndex);
		void GetGrhInfo(GrhDetails& grhInfo, int grhIndex);
		void GetSpellDetails(SpellData& spellInfo, int spellIndex);
		void GetObjectDetails(ObjectData& destObj, int itemIndex);
		void GetNpcInfo(NpcInfo& destInfo, int npcIndex);
		void GetWorldMap(WorldMap& destWorld);
		std::string GetPasswordFromAOBin();
	private:
		std::unique_ptr<ResourceLoader> mResources;
	};
}