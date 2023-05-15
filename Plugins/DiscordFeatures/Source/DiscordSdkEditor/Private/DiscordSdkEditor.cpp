// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordSdkEditor.h"
#include "DiscordSdk.h"
#include "Misc/MessageDialog.h"
#include "Interfaces/IMainFrameModule.h"
#include "DiscordSdkEditorStyle.h"
#include "DiscordSdkCommands.h"
#include "DiscordSdkWindow.h"
#include "DiscordButtonDropdown.h"

#if UE_HAS_TOOLMENUS
	#include "ToolMenuEntry.h"
	#include "ToolMenus.h"
#else // !UE_HAS_TOOLMENUS
	#include "LevelEditor.h"
#endif // !UE_HAS_TOOLMENUS

#define LOCTEXT_NAMESPACE "FDiscordSdkEditorModule"

void FDiscordSdkEditorModule::StartupModule()
{
	FDiscordSdkEditorStyle::Initialize();
	FDiscordSdkEditorStyle::ReloadTextures();
	
	FDiscordSdkCommands::Register();
	
#if UE_HAS_TOOLMENUS
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDiscordSdkEditorModule::RegisterMenus));
#else
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtender> NewToolbarExtender = MakeShared<FExtender>();
	NewToolbarExtender->AddToolBarExtension("Content",
		EExtensionHook::Before,
		nullptr,
		FToolBarExtensionDelegate::CreateRaw(this, &FDiscordSdkEditorModule::AddToolbarButton));
	LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(NewToolbarExtender);
#endif // UE_HAS_TOOLMENUS
}

void FDiscordSdkEditorModule::ShutdownModule()
{
#if UE_HAS_TOOLMENUS
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
#endif // UE_HAS_TOOLMENUS

	FDiscordSdkEditorStyle::Shutdown();
	FDiscordSdkCommands::Unregister();
}

void FDiscordSdkEditorModule::RegisterMenus()
{
#if UE_HAS_TOOLMENUS
	FToolMenuOwnerScoped OwnerScoped(this);

#if ENGINE_MAJOR_VERSION > 4
	UToolMenu* const ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
#else
	UToolMenu* const ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
#endif
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Modules");
	FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitComboButton(
		TEXT("DiscordFeatures"),
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FDiscordSdkEditorModule::GenerateButtonCombo),
		LOCTEXT("DiscordSdkEditor_Button", "Discord"),
		LOCTEXT("DiscordSdkEditor_Button_Tooltip", "Edit Discord Features options."),
		FSlateIcon(FDiscordSdkEditorStyle::GetStyleSetName(), TEXT("DiscordSdkEditor.PluginAction"))
	));
#endif // UE_HAS_TOOLMENUS
}

TSharedRef<SWidget> FDiscordSdkEditorModule::GenerateButtonCombo()
{
	return SNew(SDiscordButtonDropdown);
}

#if !UE_HAS_TOOLMENUS
void FDiscordSdkEditorModule::AddToolbarButton(FToolBarBuilder& Builder)
{
	Builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FDiscordSdkEditorModule::GenerateButtonCombo),
		LOCTEXT("DiscordSdkEditor_Button", "Discord"),
		LOCTEXT("DiscordSdkEditor_Button_Tooltip", "Edit Discord Features options."),
		FSlateIcon(FDiscordSdkEditorStyle::GetStyleSetName(), TEXT("DiscordSdkEditor.PluginAction"))
	);
}
#endif // !UE_HAS_TOOLMENUS

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDiscordSdkEditorModule, DiscordSdkEditor)
