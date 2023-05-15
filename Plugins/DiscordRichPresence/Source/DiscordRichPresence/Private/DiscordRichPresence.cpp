// Discord Rich Presence Plugin - (c) Jonathan Verbeek 2019

#include "DiscordRichPresence.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FDiscordRichPresenceModule"

void FDiscordRichPresenceModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Load the Discord DLL
	FString DLLPath = IPluginManager::Get().FindPlugin("DiscordRichPresence")->GetBaseDir() / TEXT("Source") / TEXT("ThirdParty") / TEXT("x86_64") / TEXT("discord_game_sdk.dll");

	if (FPaths::FileExists(DLLPath))
	{
		DiscordDLLHandle = FPlatformProcess::GetDllHandle(*DLLPath);
		UE_LOG(LogTemp, Log, TEXT("Found Discord DLL in Plugin directory"));
		return;
	}

	DLLPath = FPaths::ProjectDir() / TEXT("Binaries") / TEXT("Win64") / TEXT("discord_game_sdk.dll");

	if (FPaths::FileExists(DLLPath))
	{
		DiscordDLLHandle = FPlatformProcess::GetDllHandle(*DLLPath);
		UE_LOG(LogTemp, Log, TEXT("Found Discord DLL in Binaries directory"));
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Couldn't find Discord DLL! Initialization will fail!"));
}

void FDiscordRichPresenceModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Unload Discords DLL
	if (DiscordDLLHandle)
	{
		FPlatformProcess::FreeDllHandle(DiscordDLLHandle);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDiscordRichPresenceModule, DiscordRichPresence)
