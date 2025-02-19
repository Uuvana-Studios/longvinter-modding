// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistToolbar.h"

#include "BlueprintAssistCache.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistStyle.h"
#include "BlueprintAssistTabHandler.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditorModule.h"
#include "ISettingsModule.h"
#include "LevelEditorActions.h"
#include "EdGraph/EdGraph.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/LazySingleton.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SCheckBox.h"

#define LOCTEXT_NAMESPACE "BlueprintAssist"

FBAToolbarCommandsImpl::FBAToolbarCommandsImpl() : TCommands<FBAToolbarCommandsImpl>(
	TEXT("BlueprintAssistToolbarCommands"),
	NSLOCTEXT("Contexts", "BlueprintAssistToolbarCommands", "Blueprint Assist Toolbar Commands"),
	NAME_None,
	BA_GET_STYLE_SET_NAME()) { }

void FBAToolbarCommandsImpl::RegisterCommands()
{
	UI_COMMAND(
		AutoFormatting_Never,
		"Never auto format",
		"Never auto format when you create a new node",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		AutoFormatting_FormatAll,
		"Always format all connected nodes",
		"Always format all connected nodes when you create a new node",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		AutoFormatting_FormatNewlyCreated,
		"Only format the newly created node",
		"Only format the newly created node when you create a new node",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		FormattingStyle_Compact,
		"Formatting Style Compact",
		"Sets the formatting style to Compact",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		FormattingStyle_Expanded,
		"Formatting Style Expanded",
		"Sets the formatting style to Expanded",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		ParameterStyle_LeftHandSide,
		"Parameter Style Left Hand Side",
		"Parameters will be positioned out on the LHS when formatting",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		ParameterStyle_Helixing,
		"Parameter Style Helixing",
		"Parameter nodes will be positioned below when formatting",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		FormatAllStyle_Simple,
		"Format All Style: Simple",
		"Position root nodes into a single column",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		FormatAllStyle_Smart,
		"Format All Style: Smart",
		"Position root nodes into multiple columns based on node position",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		FormatAllStyle_NodeType,
		"Format All Style: Node Type",
		"Position nodes into columns based on root node type",
		EUserInterfaceActionType::RadioButton,
		FInputChord());

	UI_COMMAND(
		BlueprintAssistSettings,
		"Blueprint Assist Settings",
		"Open the blueprint assist settings",
		EUserInterfaceActionType::Button,
		FInputChord());
}

void FBAToolbarCommands::Register()
{
	FBAToolbarCommandsImpl::Register();
}

const FBAToolbarCommandsImpl& FBAToolbarCommands::Get()
{
	return FBAToolbarCommandsImpl::Get();
}

void FBAToolbarCommands::Unregister()
{
	return FBAToolbarCommandsImpl::Unregister();
}

FBAToolbar& FBAToolbar::Get()
{
	return TLazySingleton<FBAToolbar>::Get();
}

void FBAToolbar::TearDown()
{
	TLazySingleton<FBAToolbar>::TearDown();
}

void FBAToolbar::Init()
{
	FBAToolbarCommands::Register();
	BindToolbarCommands();
}

void FBAToolbar::Cleanup()
{
	ToolbarExtenderMap.Empty();
}

void FBAToolbar::OnAssetOpenedInEditor(UObject* Asset, IAssetEditorInstance* AssetEditor)
{
	if (!GetDefault<UBASettings>()->bAddToolbarWidget || !Asset || !AssetEditor)
	{
		return;
	}

	if (!GetDefault<UBASettings>()->SupportedAssetEditors.Contains(AssetEditor->GetEditorName()))
	{
		return;
	}

	FAssetEditorToolkit* AssetEditorToolkit = static_cast<FAssetEditorToolkit*>(AssetEditor);
	if (AssetEditorToolkit)
	{
		TWeakPtr<FAssetEditorToolkit> WeakToolkit = AssetEditorToolkit->AsShared();
		TSharedRef<FUICommandList> ToolkitCommands = AssetEditorToolkit->GetToolkitCommands();

		TSharedPtr<FExtender> Extender = ToolbarExtenderMap.FindRef(WeakToolkit);
		if (Extender.IsValid())
		{
			AssetEditorToolkit->RemoveToolbarExtender(Extender);
		}

		TSharedRef<FExtender> ToolbarExtender = MakeShareable(new FExtender);

		if (AssetEditor)
		{
			TSharedPtr<FTabManager> TabManager = AssetEditor->GetAssociatedTabManager();
			if (TabManager.IsValid())
			{
				ToolbarExtender->AddToolBarExtension(
					"Asset",
					EExtensionHook::After,
					ToolkitCommands,
					FToolBarExtensionDelegate::CreateRaw(this, &FBAToolbar::ExtendToolbarAndProcessTab, TWeakPtr<SDockTab>(TabManager->GetOwnerTab())));
			}
		}
		else
		{
			TSharedRef<const FExtensionBase> Extension = ToolbarExtender->AddToolBarExtension(
				"Asset",
				EExtensionHook::After,
				ToolkitCommands,
				FToolBarExtensionDelegate::CreateRaw(this, &FBAToolbar::ExtendToolbar));
		}

		ToolbarExtenderMap.Add(WeakToolkit, ToolbarExtender);
		AssetEditorToolkit->AddToolbarExtender(ToolbarExtender);
	}
}

void FBAToolbar::SetAutoFormattingStyle(EBAAutoFormatting FormattingStyle)
{
	if (FBAFormatterSettings* FormatterSettings = GetCurrentFormatterSettings())
	{
		UBASettings* BASettings = GetMutableDefault<UBASettings>();
		FormatterSettings->AutoFormatting = FormattingStyle;
		BASettings->PostEditChange();
		BASettings->SaveConfig();
	}
}

bool FBAToolbar::IsAutoFormattingStyleChecked(EBAAutoFormatting FormattingStyle)
{
	if (FBAFormatterSettings* FormatterSettings = GetCurrentFormatterSettings())
	{
		return FormatterSettings->AutoFormatting == FormattingStyle;
	}

	return false;
}

void FBAToolbar::SetParameterStyle(EBAParameterFormattingStyle Style)
{
	UBASettings* BASettings = GetMutableDefault<UBASettings>();
	BASettings->ParameterStyle = Style;
	BASettings->PostEditChange();
	BASettings->SaveConfig();
}

bool FBAToolbar::IsParameterStyleChecked(EBAParameterFormattingStyle Style)
{
	return GetDefault<UBASettings>()->ParameterStyle == Style;
}

void FBAToolbar::SetNodeFormattingStyle(EBANodeFormattingStyle Style)
{
	UBASettings* BASettings = GetMutableDefault<UBASettings>();
	BASettings->FormattingStyle = Style;
	BASettings->PostEditChange();
	BASettings->SaveConfig();
}

bool FBAToolbar::IsNodeFormattingStyleChecked(EBANodeFormattingStyle Style)
{
	return GetDefault<UBASettings>()->FormattingStyle == Style;
}

void FBAToolbar::SetFormatAllStyle(EBAFormatAllStyle Style)
{
	UBASettings* BASettings = GetMutableDefault<UBASettings>();
	BASettings->FormatAllStyle = Style;
	BASettings->PostEditChange();
	BASettings->SaveConfig();
}

bool FBAToolbar::IsFormatAllStyleChecked(EBAFormatAllStyle Style)
{
	return GetDefault<UBASettings>()->FormatAllStyle == Style;
}

void FBAToolbar::SetUseCommentBoxPadding(ECheckBoxState NewCheckedState)
{
	UBASettings* BASettings = GetMutableDefault<UBASettings>();
	BASettings->bApplyCommentPadding = (NewCheckedState == ECheckBoxState::Checked) ? true : false;
	BASettings->PostEditChange();
	BASettings->SaveConfig();
}

void FBAToolbar::SetGraphReadOnly(ECheckBoxState NewCheckedState)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = FBAUtils::GetCurrentGraphHandler();
	if (GraphHandler.IsValid())
	{
		GraphHandler->GetFocusedEdGraph()->bEditable = (NewCheckedState == ECheckBoxState::Checked) ? false : true;
	}
}

void FBAToolbar::OpenBlueprintAssistSettings()
{
	FModuleManager::LoadModuleChecked<ISettingsModule>("Settings").ShowViewer("Editor", "Plugins", "BlueprintAssist");
}

void FBAToolbar::BindToolbarCommands()
{
	const FBAToolbarCommandsImpl& Commands = FBAToolbarCommands::Get();
	BlueprintAssistToolbarActions = MakeShareable(new FUICommandList);
	FUICommandList& ActionList = *BlueprintAssistToolbarActions;

	ActionList.MapAction(
		Commands.AutoFormatting_Never,
		FExecuteAction::CreateStatic(&FBAToolbar::SetAutoFormattingStyle, EBAAutoFormatting::Never),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsAutoFormattingStyleChecked, EBAAutoFormatting::Never)
	);

	ActionList.MapAction(
		Commands.AutoFormatting_FormatNewlyCreated,
		FExecuteAction::CreateStatic(&FBAToolbar::SetAutoFormattingStyle, EBAAutoFormatting::FormatSingleConnected),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsAutoFormattingStyleChecked, EBAAutoFormatting::FormatSingleConnected)
	);

	ActionList.MapAction(
		Commands.AutoFormatting_FormatAll,
		FExecuteAction::CreateStatic(&FBAToolbar::SetAutoFormattingStyle, EBAAutoFormatting::FormatAllConnected),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsAutoFormattingStyleChecked, EBAAutoFormatting::FormatAllConnected)
	);

	ActionList.MapAction(
		Commands.FormattingStyle_Compact,
		FExecuteAction::CreateStatic(&FBAToolbar::SetNodeFormattingStyle, EBANodeFormattingStyle::Compact),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsNodeFormattingStyleChecked, EBANodeFormattingStyle::Compact)
	);

	ActionList.MapAction(
		Commands.FormattingStyle_Expanded,
		FExecuteAction::CreateStatic(&FBAToolbar::SetNodeFormattingStyle, EBANodeFormattingStyle::Expanded),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsNodeFormattingStyleChecked, EBANodeFormattingStyle::Expanded)
	);

	ActionList.MapAction(
		Commands.ParameterStyle_LeftHandSide,
		FExecuteAction::CreateStatic(&FBAToolbar::SetParameterStyle, EBAParameterFormattingStyle::LeftSide),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsParameterStyleChecked, EBAParameterFormattingStyle::LeftSide)
	);

	ActionList.MapAction(
		Commands.ParameterStyle_Helixing,
		FExecuteAction::CreateStatic(&FBAToolbar::SetParameterStyle, EBAParameterFormattingStyle::Helixing),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsParameterStyleChecked, EBAParameterFormattingStyle::Helixing)
	);

	ActionList.MapAction(
		Commands.FormatAllStyle_Simple,
		FExecuteAction::CreateStatic(&FBAToolbar::SetFormatAllStyle, EBAFormatAllStyle::Simple),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsFormatAllStyleChecked, EBAFormatAllStyle::Simple)
	);

	ActionList.MapAction(
		Commands.FormatAllStyle_Smart,
		FExecuteAction::CreateStatic(&FBAToolbar::SetFormatAllStyle, EBAFormatAllStyle::Smart),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsFormatAllStyleChecked, EBAFormatAllStyle::Smart)
	);

	ActionList.MapAction(
		Commands.FormatAllStyle_NodeType,
		FExecuteAction::CreateStatic(&FBAToolbar::SetFormatAllStyle, EBAFormatAllStyle::NodeType),
		FCanExecuteAction(),
		FIsActionChecked::CreateStatic(&FBAToolbar::IsFormatAllStyleChecked, EBAFormatAllStyle::NodeType)
	);

	ActionList.MapAction(
		Commands.BlueprintAssistSettings,
		FExecuteAction::CreateStatic(&FBAToolbar::OpenBlueprintAssistSettings)
	);
}

TSharedRef<SWidget> FBAToolbar::CreateToolbarWidget()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;

	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, BlueprintAssistToolbarActions);

	FText GlobalSettingsDescription = FText::FromString(FString::Printf(TEXT("Other")));

	TSharedPtr<FBAGraphHandler> GraphHandler = FBAUtils::GetCurrentGraphHandler();
	if (GraphHandler.IsValid())
	{
		UEdGraph* Graph = GraphHandler->GetFocusedEdGraph();
		FString GraphClassName = Graph ? Graph->GetClass()->GetName() : FString("Null");
		const FText SectionName = FText::FromString(FString::Printf(TEXT("%s settings"), *GraphClassName));
		MenuBuilder.BeginSection("FormattingSettings", SectionName);
		{
			MenuBuilder.AddSubMenu(
				LOCTEXT("AutoFormattingSubMenu", "Auto Formatting Behaviour"),
				LOCTEXT("AutoFormattingSubMenu_Tooltip", "Allows you to set the auto formatting behavior when a new node is added to the graph"),
				FNewMenuDelegate::CreateRaw(this, &FBAToolbar::MakeAutoFormattingSubMenu));

			MenuBuilder.AddSubMenu(
				LOCTEXT("FormattingStyleSubMenu", "Formatting Style"),
				LOCTEXT("FormattingStyleSubMenu_Tooltip", "Set the formatting style"),
				FNewMenuDelegate::CreateRaw(this, &FBAToolbar::MakeFormattingStyleSubMenu));

			MenuBuilder.AddSubMenu(
				LOCTEXT("ParameterStyleSubMenu", "Parameter Style"),
				LOCTEXT("ParameterStyleSubMenu_Tooltip", "Set the style for parameters when formatting"),
				FNewMenuDelegate::CreateRaw(this, &FBAToolbar::MakeParameterStyleSubMenu));

			MenuBuilder.AddSubMenu(
				LOCTEXT("FormatAllInsertStyleSubMenu", "Format All Insert Style"),
				LOCTEXT("FormatAllInsertStyle_Tooltip", "Set the format all style"),
				FNewMenuDelegate::CreateRaw(this, &FBAToolbar::MakeFormatAllStyleSubMenu));

			TSharedRef<SWidget> ApplyCommentPaddingCheckbox = SNew(SBox)
			[
				SNew(SCheckBox)
				.IsChecked(GetDefault<UBASettings>()->bApplyCommentPadding ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(FOnCheckStateChanged::CreateRaw(this, &FBAToolbar::SetUseCommentBoxPadding))
				.Style(BA_STYLE_CLASS::Get(), "Menu.CheckBox")
				.ToolTipText(LOCTEXT("ApplyCommentPaddingToolTip", "Toggle whether to apply comment padding when formatting"))
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ApplyCommentPadding", "Apply comment padding"))
					]
				]
			];

			MenuBuilder.AddMenuEntry(FUIAction(), ApplyCommentPaddingCheckbox);
		}
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection("MiscSettings");
		{
			TSharedRef<SWidget> GraphReadOnlyCheckbox = SNew(SBox)
			[
				SNew(SCheckBox)
				.IsChecked(GraphHandler->IsGraphReadOnly() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(FOnCheckStateChanged::CreateRaw(this, &FBAToolbar::SetGraphReadOnly))
				.Style(BA_STYLE_CLASS::Get(), "Menu.CheckBox")
				.ToolTipText(LOCTEXT("GraphReadOnlyToolTip", "Set the graph read only state (cannot be undone without the BA plugin!)"))
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GraphReadOnly", "Graph Read Only"))
					]
				]
			];

			MenuBuilder.AddMenuEntry(FUIAction(), GraphReadOnlyCheckbox);
		}
		MenuBuilder.EndSection();
	}
	else
	{
		GlobalSettingsDescription = FText::FromString(FString::Printf(TEXT("Settings hidden: Graph is not focused")));
	}

	// global settings
	MenuBuilder.BeginSection("GlobalSettings", GlobalSettingsDescription);
	{
		const FText OpenDebugMenuLabel = FText::FromString("Open Debug Menu");
		const FText OpenDebugMenuTooltip = FText::FromString("Opens the debug menu");
		FUIAction OpenDebugMenuAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FName("BADebugMenu"));
		}));

		MenuBuilder.AddMenuEntry(OpenDebugMenuLabel, OpenDebugMenuTooltip, FSlateIcon(), OpenDebugMenuAction);

		// open blueprint settings
		{
			MenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().BlueprintAssistSettings);
		}
	}
	MenuBuilder.EndSection();

	TSharedRef<SWidget> MenuBuilderWidget = MenuBuilder.MakeWidget();

	return MenuBuilderWidget;
}

void FBAToolbar::MakeAutoFormattingSubMenu(FMenuBuilder& InMenuBuilder)
{
	InMenuBuilder.BeginSection("AutoFormattingStyle", LOCTEXT("AutoFormattingStyle", "Auto Formatting Style"));
	{
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().AutoFormatting_Never);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().AutoFormatting_FormatNewlyCreated);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().AutoFormatting_FormatAll);
	}
	InMenuBuilder.EndSection();
}

void FBAToolbar::MakeParameterStyleSubMenu(FMenuBuilder& InMenuBuilder)
{
	InMenuBuilder.BeginSection("ParameterStyle", LOCTEXT("ParameterStyle", "Parameter Style"));
	{
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().ParameterStyle_Helixing);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().ParameterStyle_LeftHandSide);
	}
	InMenuBuilder.EndSection();
}

void FBAToolbar::MakeFormattingStyleSubMenu(FMenuBuilder& InMenuBuilder)
{
	InMenuBuilder.BeginSection("FormattingStyle", LOCTEXT("FormattingStyle", "Formatting Style"));
	{
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().FormattingStyle_Compact);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().FormattingStyle_Expanded);
	}
	InMenuBuilder.EndSection();
}

void FBAToolbar::MakeFormatAllStyleSubMenu(FMenuBuilder& InMenuBuilder)
{
	InMenuBuilder.BeginSection("FormatAllStyle", LOCTEXT("FormatAllStyle", "Format All Style"));
	{
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().FormatAllStyle_Simple);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().FormatAllStyle_Smart);
		InMenuBuilder.AddMenuEntry(FBAToolbarCommands::Get().FormatAllStyle_NodeType);
	}
	InMenuBuilder.EndSection();
}

void FBAToolbar::ExtendToolbarAndProcessTab(FToolBarBuilder& ToolbarBuilder, TWeakPtr<SDockTab> Tab)
{
	if (!Tab.IsValid())
	{
		return;
	}

	FBATabHandler::Get().ProcessTab(Tab.Pin());
	ExtendToolbar(ToolbarBuilder);
}

void FBAToolbar::ExtendToolbar(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateRaw(this, &FBAToolbar::CreateToolbarWidget),
		LOCTEXT("BlueprintAssist", "BP Assist"),
		FText::FromString("Blueprint Assist Settings"),
		FSlateIcon(BA_GET_STYLE_SET_NAME(), "LevelEditor.GameSettings")
	);
}

FBAFormatterSettings* FBAToolbar::GetCurrentFormatterSettings()
{
	auto GraphHandler = FBAUtils::GetCurrentGraphHandler();
	if (!GraphHandler.IsValid())
	{
		return nullptr;
	}

	return UBASettings::FindFormatterSettings(GraphHandler->GetFocusedEdGraph());
}

#undef LOCTEXT_NAMESPACE