// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscordAchievementEditor, Log, All);

class DISCORDACHIEVEMENTEDITOR_API FDiscordAchievementEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static const FName DiscordAchievementTabName;

	void InvokeAchivementTab();

	void OnDiscordAchievementAction();

private:
	TSharedRef<class SDockTab> SpawnAchievementTab(const FSpawnTabArgs& TabArgs);
};

