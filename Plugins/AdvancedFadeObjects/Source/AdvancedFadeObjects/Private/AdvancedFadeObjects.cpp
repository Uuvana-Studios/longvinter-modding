// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved. 

#include "AdvancedFadeObjects.h"

#define LOCTEXT_NAMESPACE "FAdvancedFadeObjectsModule"

void FAdvancedFadeObjectsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FAdvancedFadeObjectsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAdvancedFadeObjectsModule, AdvancedFadeObjects)