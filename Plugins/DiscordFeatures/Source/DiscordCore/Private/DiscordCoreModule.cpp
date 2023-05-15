// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordCoreModule.h"


DEFINE_LOG_CATEGORY(LogDiscordCore);

#define LOCTEXT_NAMESPACE "FDiscordCoreModule"


void FDiscordCoreModule::StartupModule()
{
}

void FDiscordCoreModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDiscordCoreModule, DiscordCore)