// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettings.h"
#include "SettingsManager.h"
#include "Misc/CoreDelegates.h"
#include "AutoSettingsLogs.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

void FAutoSettingsModule::StartupModule()
{
	UE_LOG(LogAutoSettings, Log, TEXT("AutoSettings module starting up"));

	FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		// Initialize settings manager so that it can load config
		// Do this after engine init so that saved CVar values apply on top of engine-determined values
		// The module itself has to load before PostEngineInit so that content is available to the game
		USettingsManager::Get()->Init();
	});

}

void FAutoSettingsModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAutoSettingsModule, AutoSettings)
