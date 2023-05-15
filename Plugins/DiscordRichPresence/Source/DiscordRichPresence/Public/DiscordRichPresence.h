// Discord Rich Presence Plugin - (c) Jonathan Verbeek 2019

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "DiscordRichPresence/Public/Discord/discord.h"

class FDiscordRichPresenceModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// The handle which stores the DLL's address of Discord's DLL
	void* DiscordDLLHandle;
};
