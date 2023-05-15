// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordGateway.h"

#define LOCTEXT_NAMESPACE "FDiscordGatewayModule"

void FDiscordGatewayModule::StartupModule()
{
	const FName WebSocketModuleName = TEXT("WebSockets");
	if (!FModuleManager::Get().IsModuleLoaded(WebSocketModuleName))
	{
		FModuleManager::Get().LoadModule(WebSocketModuleName);
	}
}

void FDiscordGatewayModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDiscordGatewayModule, DiscordGateway)

