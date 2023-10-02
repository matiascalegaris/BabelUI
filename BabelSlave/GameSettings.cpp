#include "GameSettings.hpp"
#include "Utils/IniReader.h"
#include "Utils/FileUtils.h"
#include "Utils/JSUtils.hpp"
#include <Windows.h>

namespace Babel
{
	namespace {
		const char* TrueStr = "1";
		const char *FalseStr = "0";
	}
	void GameSettings::Load()
	{
		auto path = GetFilePath("OUTPUT/Configuracion.ini");
		INIReader Reader(path.u8string());

		mGameplaySettings.CopyDialogsEnabled = Reader.GetInteger("OPCIONES", "CopiarDialogoAConsola", 0) > 0;
		mGameplaySettings.WriteAndMove = Reader.GetInteger("OPCIONES", "PermitirMoverse", 0) > 0;
		mGameplaySettings.BlockSpellListScroll = Reader.GetInteger("OPCIONES", "ScrollArrastrar", 0) > 0;
		mGameplaySettings.ThrowSpellLockBehavior = Reader.GetInteger("OPCIONES", "ModoHechizos", 0);
		mGameplaySettings.MouseSens = Reader.GetInteger("OPCIONES", "SensibilidadMouse", 0);
		mGameplaySettings.UserGraphicCursor = Reader.GetInteger("VIDEO", "CursoresGraficos", 0) > 0;
		mGameplaySettings.Language = Reader.GetInteger("OPCIONES", "Localization", 1);
		mGameplaySettings.NpcTextType = Reader.GetInteger("OPCIONES", "NpcsEnRender", 1);
		mGameplaySettings.TutorialEnabled = Reader.GetInteger("INITTUTORIAL", "MostrarTutorial", 0) > 0;
		mGameplaySettings.EnableBabelUI = Reader.GetInteger("OPCIONES", "UseExperimentalUI", 0) > 0;

		mVideoSettings.ShopFps = Reader.GetInteger("OPCIONES", "FPSFLAG", 0) > 0;
		mVideoSettings.MoveGameWindow = Reader.GetInteger("OPCIONES", "MoverVentana", 0) > 0; 
		mVideoSettings.CharacterBreathing = Reader.GetInteger("VIDEO", "MostrarRespiracion", 0) > 0;
		mVideoSettings.FullScreen = Reader.GetInteger("VIDEO", "PantallaCompleta", 0) > 0;
		mVideoSettings.DisplayFloorItemInfo = Reader.GetInteger("VIDEO", "InfoItemsEnRender", 0) > 0;
		mVideoSettings.DisplayFullNumbersInventory = Reader.GetInteger("OPCIONES", "NumerosCompletosInventario", 0) > 0;
		mVideoSettings.LightSettings = Reader.GetInteger("VIDEO", "LuzGlobal", 0);

		mAudioSettings.EnableMusic = Reader.GetInteger("AUDIO", "Musica", 0) > 0;
		mAudioSettings.EnableFx = Reader.GetInteger("AUDIO", "Fx", 0) > 0;
		mAudioSettings.EnableAmbient = Reader.GetInteger("AUDIO", "AmbientalActivated", 0) > 0;
		mAudioSettings.SailFx = Reader.GetInteger("AUDIO", "FxNavega", 0) > 0;
		mAudioSettings.InvertChannels = Reader.GetInteger("AUDIO", "InvertirSonido", 0) > 0;
		mAudioSettings.MusicVolume = Reader.GetInteger("AUDIO", "VolMusic", 0) > 0;
		mAudioSettings.FxVolume = Reader.GetInteger("AUDIO", "VolFx", 0) > 0;
		mAudioSettings.AmbientVolume = Reader.GetInteger("AUDIO", "VolAmbient", 0) > 0;
	}

	void GameSettings::Write()
	{
		auto path = GetFilePath("OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "CopiarDialogoAConsola", mGameplaySettings.CopyDialogsEnabled ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "PermitirMoverse", mGameplaySettings.WriteAndMove ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "ScrollArrastrar", mGameplaySettings.BlockSpellListScroll ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "ModoHechizos", std::to_string(mGameplaySettings.ThrowSpellLockBehavior).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "SensibilidadMouse", std::to_string(mGameplaySettings.MouseSens).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "InvertirMouse", mGameplaySettings.InvertMouse ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("VIDEO", "CursoresGraficos", mGameplaySettings.UserGraphicCursor ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "Localization", std::to_string(mGameplaySettings.Language).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "NpcsEnRender", std::to_string(mGameplaySettings.NpcTextType).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("INITTUTORIAL", "MostrarTutorial", mGameplaySettings.TutorialEnabled ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "UseExperimentalUI", mGameplaySettings.EnableBabelUI ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");

		WritePrivateProfileStringA("OPCIONES", "FPSFLAG", mVideoSettings.ShopFps ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "MoverVentana", mVideoSettings.MoveGameWindow ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("VIDEO", "MostrarRespiracion", mVideoSettings.CharacterBreathing ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("VIDEO", "PantallaCompleta", mVideoSettings.FullScreen ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("VIDEO", "InfoItemsEnRender", mVideoSettings.DisplayFloorItemInfo ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("OPCIONES", "NumerosCompletosInventario", mVideoSettings.DisplayFullNumbersInventory ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");

		WritePrivateProfileStringA("AUDIO", "Musica", mAudioSettings.EnableMusic ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "Fx", mAudioSettings.EnableFx ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "AmbientalActivated", mAudioSettings.EnableAmbient ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "FxNavega", mAudioSettings.SailFx ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "InvertirSonido", mAudioSettings.InvertChannels ? TrueStr : FalseStr, "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "VolMusic", std::to_string(mAudioSettings.MusicVolume).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "VolFx", std::to_string(mAudioSettings.FxVolume).c_str(), "OUTPUT/Configuracion.ini");
		WritePrivateProfileStringA("AUDIO", "VolAmbient", std::to_string(mAudioSettings.AmbientVolume).c_str(), "OUTPUT/Configuracion.ini");
	}

	void GameSettings::GetJsSettings(JSContextRef& ctx, JSObjectRef& outObject)
	{
		JSObjectRef gameplay = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectBoolean(ctx, gameplay, "copyDialogsEnabled", mGameplaySettings.CopyDialogsEnabled);
		SetObjectBoolean(ctx, gameplay, "writeAndMove", mGameplaySettings.WriteAndMove);
		SetObjectBoolean(ctx, gameplay, "blockSpellListScroll", mGameplaySettings.BlockSpellListScroll);
		SetObjectNumber(ctx, gameplay, "throwSpellLockBehavior", mGameplaySettings.ThrowSpellLockBehavior);
		SetObjectNumber(ctx, gameplay, "mouseSens", mGameplaySettings.MouseSens);
		SetObjectBoolean(ctx, gameplay, "invertMouse", mGameplaySettings.InvertMouse);
		SetObjectBoolean(ctx, gameplay, "userGraphicCursor", mGameplaySettings.UserGraphicCursor);
		SetObjectNumber(ctx, gameplay, "language", mGameplaySettings.Language);
		SetObjectBoolean(ctx, gameplay, "renderNpcText", mGameplaySettings.NpcTextType);
		SetObjectBoolean(ctx, gameplay, "tutorialEnabled", mGameplaySettings.TutorialEnabled);

		JSObjectRef video = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectBoolean(ctx, video, "showFps", mVideoSettings.ShopFps);
		SetObjectBoolean(ctx, video, "moveGameWindow", mVideoSettings.MoveGameWindow);
		SetObjectBoolean(ctx, video, "characterBreathing", mVideoSettings.CharacterBreathing);
		SetObjectBoolean(ctx, video, "fullScreen", mVideoSettings.FullScreen);
		SetObjectBoolean(ctx, video, "displayFloorItemInfo", mVideoSettings.DisplayFloorItemInfo);
		SetObjectBoolean(ctx, video, "displayFullNumbersInventory", mVideoSettings.DisplayFullNumbersInventory);
		SetObjectBoolean(ctx, video, "enableBabelUI", mGameplaySettings.EnableBabelUI);
		SetObjectNumber(ctx, video, "lightSettings", mVideoSettings.LightSettings);

		JSObjectRef audio = JSObjectMake(ctx, nullptr, nullptr);
		SetObjectBoolean(ctx, audio, "enableMusic", mAudioSettings.EnableMusic);
		SetObjectBoolean(ctx, audio, "enableFx", mAudioSettings.EnableFx);
		SetObjectBoolean(ctx, audio, "enableAmbient", mAudioSettings.EnableAmbient);
		SetObjectBoolean(ctx, audio, "sailFx", mAudioSettings.SailFx);
		SetObjectBoolean(ctx, audio, "invertChannels", mAudioSettings.InvertChannels);
		SetObjectNumber(ctx, audio, "musicVolume", mAudioSettings.MusicVolume);
		SetObjectNumber(ctx, audio, "fxVolume", mAudioSettings.FxVolume);
		SetObjectNumber(ctx, audio, "ambientVolume", mAudioSettings.AmbientVolume);

		SetChildObject(ctx, outObject, "gameplay", gameplay);
		SetChildObject(ctx, outObject, "video", video);
		SetChildObject(ctx, outObject, "audio", audio);
	}
}
