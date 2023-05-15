// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsInput.h"
#include "InputMappingManager.h"
#include "Misc/CoreDelegates.h"
#include "Misc/AutoSettingsInputLogs.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "FAutoSettingsModule"

void FAutoSettingsInputModule::StartupModule()
{
	UE_LOG(LogAutoSettingsInput, Log, TEXT("AutoSettingsInput module starting up"));

	FCoreDelegates::OnPostEngineInit.AddLambda([]()
	{
		UInputMappingManager::Get();
	});

}

void FAutoSettingsInputModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAutoSettingsInputModule, AutoSettingsInput)
