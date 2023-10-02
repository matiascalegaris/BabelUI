#pragma once

#include <AppCore/JSHelpers.h>

namespace Babel
{
	struct GameplaySettings
	{
		bool CopyDialogsEnabled{false};
		bool WriteAndMove{false};
		bool BlockSpellListScroll{false};
		int ThrowSpellLockBehavior{0};
		int MouseSens{0};
		bool InvertMouse{ false };
		bool UserGraphicCursor{ false };
		int Language{0};
		int NpcTextType{0};
		bool TutorialEnabled{ false };
		bool EnableBabelUI{ false };
	};

	struct VideoSettings
	{
		bool ShopFps{ false };
		bool MoveGameWindow{ false };
		bool CharacterBreathing{ false };
		bool FullScreen{ false };
		bool DisplayFloorItemInfo{ false };
		bool DisplayFullNumbersInventory{ false };
		int LightSettings{ 0 };
	};

	struct AudioSettings
	{
		bool EnableMusic{ false };
		bool EnableFx{ false };
		bool EnableAmbient{ false };
		bool SailFx{ false };
		bool InvertChannels{ false };
		int MusicVolume{0};
		int FxVolume{ 0 };
		int AmbientVolume{ 0 };
	};

	class GameSettings
	{
	public:
		void Load();
		void Write();
		void GetJsSettings(JSContextRef& jsContext, JSObjectRef& outObject);
	private:
		GameplaySettings mGameplaySettings;
		VideoSettings mVideoSettings;
		AudioSettings mAudioSettings;
	};
}