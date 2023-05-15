// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordSDK.h"
#include "Misc/Paths.h"
#include "DiscordTypes.h"
#include "DiscordFunctionsSignature.h"
#include "DiscordSettings.h"
#include "Misc/ConfigCacheIni.h"


/* static */ FDiscordSdk* FDiscordSdk::Get()
{
	static FDiscordSdk SDK;

	return &SDK;
}

FDiscordSdk::FDiscordSdk()
	: SdkDllHandle(nullptr)
	, SdkVersion(2)
{	
	SettingsUpdated();
}

FDiscordSdk::~FDiscordSdk()
{
	if (SdkDllHandle)
	{
		FPlatformProcess::FreeDllHandle(SdkDllHandle);
	}
}

void FDiscordSdk::SettingsUpdated()
{
	SdkVersion = UDiscordSettings::GetSdkVersion();

	UE_LOG(LogDiscordSdk, Log, TEXT("SDK version: %d."), SdkVersion);

#if WITH_EDITOR
	if (GConfig)
	{
		GConfig->SetString(TEXT("/Script/DiscordSdk.DiscordSettings"), TEXT("ClientId"), 
			*FString::Printf(TEXT("%lld"), UDiscordSettings::GetClientId()), GEngineIni);
	}
#endif

	LoadSdk();
}

void FDiscordSdk::Free()
{
	if (SdkDllHandle)
	{
		FPlatformProcess::FreeDllHandle(SdkDllHandle);
		SdkDllHandle = nullptr;
	}
}

bool FDiscordSdk::LoadSdk()
{
	const FString NewSdkPath = UDiscordSettings::GetSdkPath();

	//if (NewSdkPath == SdkPath)
	//{
	//	UE_LOG(LogDiscordSdk, Log, TEXT("SDK path remains unchanged, skipping loading."));
	//	return false;
	//}

	UE_LOG(LogDiscordSdk, Log, TEXT("Loading Discord SDK..."));

	if (!FPaths::FileExists(NewSdkPath))
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to find Dll with path \"%s\"."), *FPaths::ConvertRelativePathToFull(NewSdkPath));
		return false;
	}

	FDllHandle OldDll = SdkDllHandle;

	SdkDllHandle = FPlatformProcess::GetDllHandle(*NewSdkPath);

	if (!SdkDllHandle)
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to get Dll Handle. Dll path: \"%s\"."), *FPaths::ConvertRelativePathToFull(NewSdkPath));
		return false;
	}

	if (OldDll)
	{
		FPlatformProcess::FreeDllHandle(OldDll);
	}
	
	SdkPath = NewSdkPath;

	UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK DLL loaded. SDK full path: \"%s\"."), *FPaths::ConvertRelativePathToFull(SdkPath));

	return LoadFunctions();
}

bool FDiscordSdk::LoadFunctions()
{
	if (!SdkDllHandle)
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Can't load Discord SDK. Dll Handle is nullptr."));
		return false;
	}

	static const TSet<FString> FunctionsToLoad =
	{
		TEXT("DiscordCreate")
	};

	SdkFunctions.Empty(FunctionsToLoad.Num());

	for (const FString& FunctionToLoad : FunctionsToLoad)
	{
		void* const Function = FPlatformProcess::GetDllExport(SdkDllHandle, *FunctionToLoad);
		
		if (!Function)
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Failed to load function %s from SDK Dll."), *FunctionToLoad);
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Function %s loaded from Discord SDK Dll."), *FunctionToLoad);
		
		SdkFunctions.Add(FunctionToLoad, Function);
	}
	return true;
}

FRawDiscord::IDiscordCore* FDiscordSdk::CreateCore(FRawDiscord::DiscordCreateParams& DiscordCreateParams)
{
	FDiscordCreate DiscordCreateFunction = GetSdkFunction<FDiscordCreate>(TEXT("DiscordCreate"));

	if (!DiscordCreateFunction)
	{
		return nullptr;
	}

	FRawDiscord::IDiscordCore* DiscordCore = nullptr;

	const FRawDiscord::EDiscordResult Result = DiscordCreateFunction(SdkVersion, &DiscordCreateParams, &DiscordCore);
	if (Result != FRawDiscord::EDiscordResult::Ok)
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("DiscordCreate() failed. Code: %d."), (int32)Result);
		return nullptr;
	}

	UE_LOG(LogDiscordSdk, Log, TEXT("New Discord Core created."));

	return DiscordCore;
}

bool FDiscordSdk::IsValid() const
{
	return SdkDllHandle != nullptr;
}

#if WITH_EDITOR
bool FDiscordSdk::AreAllDllsValid() const
{
	return
		UDiscordSettings::IsWindowsPathValid() &&
		UDiscordSettings::IsMacOSPathValid() &&
		UDiscordSettings::IsLinuxPathValid();
}
#endif // WITH_EDITOR
