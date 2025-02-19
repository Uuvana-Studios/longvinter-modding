// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class UBARootObject;
class FBlueprintAssistGraphPanelNodeFactory;

class FBlueprintAssistModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	void BindLiveCodingSound();

	void RegisterSettings();

	static FBlueprintAssistModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FBlueprintAssistModule>("BlueprintAssist");
	}

	static bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("BlueprintAssist");
	}

	UBARootObject* GetRootObject() const
	{
		return RootObject;
	};

private:
	bool bWasModuleInitialized = false;

	TSharedPtr<FBlueprintAssistGraphPanelNodeFactory> BANodeFactory;

	UBARootObject* RootObject = nullptr;

	void OnPostEngineInit();
};
