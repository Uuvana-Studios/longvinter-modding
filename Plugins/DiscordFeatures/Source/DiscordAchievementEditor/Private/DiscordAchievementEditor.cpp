// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordAchievementEditor.h"
#include "Framework/Docking/TabManager.h"
#include "DiscordAchievementWindow.h"
#include "Widgets/Docking/SDockTab.h"
#include "DiscordSdkEditor.h"
#include "DiscordSdkEditorStyle.h"

DEFINE_LOG_CATEGORY(LogDiscordAchievementEditor);

const FName FDiscordAchievementEditorModule::DiscordAchievementTabName = TEXT("DiscordAchievementTab");

#define LOCTEXT_NAMESPACE "FDiscordAchievementEditorModule"

void FDiscordAchievementEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterTabSpawner(
		DiscordAchievementTabName,
		FOnSpawnTab::CreateRaw(this, &FDiscordAchievementEditorModule::SpawnAchievementTab)
	)
		.SetDisplayName(FText::FromString(TEXT("Discord Achievements")))
		.SetIcon(FSlateIcon(FDiscordSdkEditorStyle::GetStyleSetName(), TEXT("DiscordSdkEditor.DiscordBlack")));

	FModuleManager::GetModuleChecked<FDiscordSdkEditorModule>(TEXT("DiscordSdkEditor"))
		.GetOnDiscordAchievementAction().BindRaw(
			this, &FDiscordAchievementEditorModule::OnDiscordAchievementAction
		);
}

void FDiscordAchievementEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterTabSpawner(DiscordAchievementTabName);
}

TSharedRef<SDockTab> FDiscordAchievementEditorModule::SpawnAchievementTab(const FSpawnTabArgs& TabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::MajorTab)
	[
		SNew(SDiscordAchievementWindow)
	];
}

void FDiscordAchievementEditorModule::InvokeAchivementTab()
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	FGlobalTabmanager::Get()->InvokeTab(DiscordAchievementTabName);
#else
	FGlobalTabmanager::Get()->TryInvokeTab(DiscordAchievementTabName);
#endif
}

void FDiscordAchievementEditorModule::OnDiscordAchievementAction()
{
	InvokeAchivementTab();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDiscordAchievementEditorModule, DiscordAchievementEditor)

