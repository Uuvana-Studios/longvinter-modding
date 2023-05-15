// Copyright Sam Bonifacio. All Rights Reserved.

#include "AutoSettingsCore.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#endif

#define LOCTEXT_NAMESPACE "FAutoSettingsCoreModule"

void FAutoSettingsCoreModule::StartupModule()
{

}

void FAutoSettingsCoreModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAutoSettingsCoreModule, AutoSettingsCore)
