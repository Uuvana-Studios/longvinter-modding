// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordSDKModule.h"
#include "CoreMinimal.h"
#include "DiscordTypes.h"

typedef void* FDllHandle;

/**
 *	Wrapper around the Discord SDK.
 */
class DISCORDSDK_API FDiscordSdk
{
public:
	static FDiscordSdk* Get();

	~FDiscordSdk();

	void SettingsUpdated();

	/**
	 * Creates a raw Discord Core. You have to free it yourself.
	*/
	struct FRawDiscord::IDiscordCore* CreateCore(FRawDiscord::DiscordCreateParams& DiscordCreateParams);

	FORCEINLINE bool IsValid() const; 

	void Free();

#if WITH_EDITOR
	bool AreAllDllsValid() const;
#endif // WITH_EDITOR

private:
	FDiscordSdk();

	FDiscordSdk(FDiscordSdk const&)				= delete;
	FDiscordSdk& operator=(FDiscordSdk const&)	= delete;
	FDiscordSdk(FDiscordSdk&&)					= delete;
	FDiscordSdk& operator=(FDiscordSdk&&)		= delete;


	bool LoadSdk();
	bool LoadFunctions();

	template<class FFuncSignature>
	FFuncSignature GetSdkFunction(const FString& FunctionName)
	{
		void * const * const ElementPtr = SdkFunctions.Find(FunctionName);
		if (!ElementPtr)
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Function %s is not in the SDK loaded function list."), *FunctionName);
			return nullptr;
		}
		return (FFuncSignature)(*ElementPtr);
	}

private:
	FString SdkPath;

	FDllHandle SdkDllHandle;

	TMap<FString, void*> SdkFunctions;

	int32 SdkVersion;
};

