// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordSettings.h"
#include "DiscordTypes.h"
#include "Misc/Paths.h"
#include "DiscordSDKModule.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
#	include "Misc/ConfigCacheIni.h"
#endif 

UDiscordSettings::UDiscordSettings()
{
	LoadConfig();
}

/* static */ uint64 UDiscordSettings::GetClientId()
{
	return FCString::Strtoui64(*GetDefault<UDiscordSettings>()->ClientId, nullptr, 10);
}

/* static */ FString UDiscordSettings::GetClientIdAsString()
{
	return NewObject<UDiscordSettings>()->ClientId;
}

/* static */ FString UDiscordSettings::GetSdkPath()
{
	return GetPlugin()->GetBaseDir() / DISCORD_SDK_DLL;
}

#if WITH_EDITOR
/* static */ void UDiscordSettings::TestSdkPaths()
{
#define CHECK_SDK_DLL_PATH(Platform) {																				\
	if (Settings->Is ## Platform ## PathValid())																	\
	{																												\
		UE_LOG(LogTemp, Log, TEXT("Discord SDK Dll for ") TEXT(# Platform) TEXT(" is valid."));						\
	}																												\
	else																											\
	{																												\
		UE_LOG(LogTemp, Warning, TEXT("Discord SDK Dll not found for ") TEXT(#Platform) TEXT(" with path \"%s\"."),	\
			*FPaths::ConvertRelativePathToFull(Settings->DiscordSdkDllPath ## Platform.FilePath));					\
	}																												\
}
	const UDiscordSettings* const Settings = NewObject<UDiscordSettings>();

	CHECK_SDK_DLL_PATH(Windows);
	CHECK_SDK_DLL_PATH(MacOS);
	CHECK_SDK_DLL_PATH(Linux);

#undef CHECK_SDK_DLL_PATH
}

/* static */ bool UDiscordSettings::IsWindowsPathValid()
{
	const FString FilePath = NewObject<UDiscordSettings>()->DiscordSdkDllPathWindows.FilePath;
	return FilePath.Len() > 0 && FPaths::FileExists(GetPlugin()->GetBaseDir() / FilePath);
}

/* static */ bool UDiscordSettings::IsMacOSPathValid()
{
	const FString FilePath = NewObject<UDiscordSettings>()->DiscordSdkDllPathMacOS.FilePath;
	return FilePath.Len() > 0 && FPaths::FileExists(GetPlugin()->GetBaseDir() / FilePath);
}

/* static */ bool UDiscordSettings::IsLinuxPathValid()
{
	const FString FilePath = NewObject<UDiscordSettings>()->DiscordSdkDllPathLinux.FilePath;
	return FilePath.Len() > 0 && FPaths::FileExists(GetPlugin()->GetBaseDir() / FilePath);
}
/* static */ void UDiscordSettings::UpdateLocations(const FString& Windows, const FString& MacOS, const FString& Linux)
{
	UDiscordSettings* const Settings = NewObject<UDiscordSettings>();

	// We don't update location anymore, it's frozen in ThirdParty.
	//
	//Settings->DiscordSdkDllPathWindows.FilePath = Windows;
	//Settings->DiscordSdkDllPathMacOS  .FilePath = MacOS;
	//Settings->DiscordSdkDllPathLinux  .FilePath = Linux;

	Settings->SaveConfig();
}

/* static */ void UDiscordSettings::SetApplicationId(const FString& ApplicationId)
{
	UDiscordSettings* const Settings = NewObject<UDiscordSettings>();
	Settings->ClientId = ApplicationId;
	Settings->SaveConfig();
}

/* static */ void UDiscordSettings::SetSdkVersion(const int32 NewVersion)
{
	UDiscordSettings* const Settings = NewObject<UDiscordSettings>();
	Settings->DiscordSdkVersion = NewVersion;
	Settings->SaveConfig();
}
#endif // WITH_EDITOR

/* static */ int32   UDiscordSettings::GetSdkVersion()
{
	return GetDefault<UDiscordSettings>()->DiscordSdkVersion;
}

/* static */ void UDiscordSettings::SetDefaultDiscordCreateParams(FRawDiscord::DiscordCreateParams* const Params)
{
	const UDiscordSettings* const Settings = GetDefault<UDiscordSettings>();

	FMemory::Memset(Params, 0, sizeof(FRawDiscord::DiscordCreateParams));

	Params->application_version  = Settings->DiscordApplicationManagerVersion;
	Params->user_version		 = Settings->DiscordUserManagerVersion;
	Params->user_version		 = Settings->DiscordImageManagerVersion;
	Params->activity_version	 = Settings->DiscordActivityManagerVersion;
	Params->relationship_version = Settings->DiscordRelationShipManagerVersion;
	Params->lobby_version		 = Settings->DiscordLobbyManagerVersion;
	Params->network_version		 = Settings->DiscordNetworkManagerVersion;
	Params->overlay_version		 = Settings->DiscordOverlayManagerVersion;
	Params->storage_version		 = Settings->DiscordStorageManagerVersion;
	Params->store_version		 = Settings->DiscordStoreManagerVersion;
	Params->voice_version		 = Settings->DiscordVoiceManagerVersion;
	Params->achievement_version	 = Settings->DiscordAchievementManagerVersion;
}

TSharedPtr<IPlugin> UDiscordSettings::GetPlugin()
{
	TSharedPtr<IPlugin> DiscordPlugin = IPluginManager::Get().FindPlugin(TEXT("DiscordFeatures"));

	check(DiscordPlugin != nullptr);

	return DiscordPlugin;
}

FString UDiscordSettings::ToString(int64 Value)
{
	return FString::Printf(TEXT("%lld"), Value);
}
