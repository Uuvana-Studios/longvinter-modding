// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordTypes.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "DiscordSettings.generated.h"

/**
 * Settings for the Discord Features plugin.
 */
UCLASS(config=Engine, defaultconfig)
class DISCORDSDK_API UDiscordSettings : public UObject
{
	GENERATED_BODY()

public:
	UDiscordSettings();

	static uint64  GetClientId();
	static FString GetClientIdAsString();
	static FString GetSdkPath();
	static int32   GetSdkVersion();

	static void SetDefaultDiscordCreateParams(FRawDiscord::DiscordCreateParams* const Params);

#if WITH_EDITOR
	static void TestSdkPaths();
	static bool IsWindowsPathValid();
	static bool IsMacOSPathValid();
	static bool IsLinuxPathValid();
	static void UpdateLocations(const FString & Windows, const FString & MacOS, const FString& Linux);
	static void SetApplicationId(const FString& ApplicationId);
	static void SetSdkVersion(const int32 NewVersion);
#endif // WITH_EDITOR

protected:
	
	/**
	 * The version of the SDK you downloaded
	*/
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version", DisplayName = "Discord SDK Version")
	int32 DiscordSdkVersion = 2;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordApplicationManagerVersion	= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordUserManagerVersion			= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordImageManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordActivityManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordRelationShipManagerVersion = 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordLobbyManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordNetworkManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordOverlayManagerVersion		= 2;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordStorageManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordStoreManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordVoiceManagerVersion		= 1;
	UPROPERTY(config, EditAnywhere, Category = "Discord SDK version")
	int32 DiscordAchievementManagerVersion	= 1;

	/**
	 * The path to the Discord SDK DLL for Windows (.dll).
	*/
	//UPROPERTY(config, BlueprintReadOnly, Category = "Discord SDK", DisplayName = "Windows - Discord SDK Dll location")
	FFilePath DiscordSdkDllPathWindows = { TEXT("ThirdParty/DiscordSdk/lib/x86_64/discord_game_sdk.dll") };

	/**
	 * The path to the Discord SDK DLL for MacOS (.dylib).
	*/
	//UPROPERTY(config, BlueprintReadOnly, Category = "Discord SDK", DisplayName = "MacOS - Discord SDK Dll location")
	FFilePath DiscordSdkDllPathMacOS = { TEXT("ThirdParty/DiscordSdk/lib/x86_64/discord_game_sdk.dylib") };

	/**
	 * The path to the Discord SDK DLL for Linux (.so).
	*/
	//UPROPERTY(config, BlueprintReadOnly, Category = "Discord SDK", DisplayName = "Linux - Discord SDK Dll location")
	FFilePath DiscordSdkDllPathLinux = { TEXT("ThirdParty/DiscordSdk/lib/x86_64/discord_game_sdk.so") };
	
	/**
	 * Your application's Client ID.
	*/
	UPROPERTY(config, EditAnywhere, Category = "Discord Application", DisplayName = "Application Client ID")
	FString ClientId;

public:
	UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "->", DisplayName = "To String (int64)", BlueprintAutocast), Category = "Discord")
	static FString ToString(int64 Value);

private:
	static TSharedPtr<class IPlugin> GetPlugin();

};
