#pragma once
#include <memory>

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

	class ResourceLoader;
	class Resources
	{
	public:
		Resources(bool compresed);
		~Resources();

		void GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex);
		void GetHeadInfo(GrhDetails& headInfo, int headIndex);
	private:
		std::unique_ptr<ResourceLoader> mResources;
	};
}