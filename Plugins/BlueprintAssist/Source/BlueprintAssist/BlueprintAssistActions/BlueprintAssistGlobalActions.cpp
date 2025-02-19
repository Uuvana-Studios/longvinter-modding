#include "BlueprintAssistGlobalActions.h"

#include "BlueprintAssistSettings.h"
#include "SGraphPanel.h"
#include "BlueprintAssist/BlueprintAssistUtils/BAMiscUtils.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistCreateAssetMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistHotkeyMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/BlueprintAssistTabSwitcher.h"
#include "BlueprintAssist/BlueprintAssistWidgets/EditDetailsMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/FocusSearchBoxMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/OpenWindowMenu.h"
#include "EdGraph/EdGraph.h"
#include "Framework/Commands/UICommandList.h"
#include "WorkflowOrientedApp/WorkflowCentricApplication.h"

bool FBAGlobalActionsBase::CanOpenEditDetailsMenu() const
{
	return CanExecuteActions() && SEditDetailsMenu::CanOpenMenu();
}

bool FBAGlobalActionsBase::HasWorkflowModes() const
{
	const TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (!ActiveWindow)
	{
		return false;
	}

	TArray<TSharedPtr<SWidget>> ModeWidgets;
	FBAUtils::GetChildWidgets(ActiveWindow, "SModeWidget", ModeWidgets);

	TArray<TSharedPtr<SWidget>> AssetShortcutWidgets;
	FBAUtils::GetChildWidgets(ActiveWindow, "SAssetShortcut", AssetShortcutWidgets);

	return ModeWidgets.Num() > 0 || AssetShortcutWidgets.Num() > 0;
}

void FBAGlobalActions::Init()
{
	GlobalCommands = MakeShareable(new FUICommandList());

	GlobalCommands->MapAction(
		FBACommands::Get().OpenBlueprintAssistDebugMenu,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenBlueprintAssistDebugMenu)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().FocusSearchBoxMenu,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenFocusSearchBoxMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().EditDetailsMenu,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenEditDetailsMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanOpenEditDetailsMenu)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().OpenWindow,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenWindowMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().OpenBlueprintAssistHotkeySheet,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenBlueprintAssistHotkeyMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().TabSwitcherMenu,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenTabSwitcherMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().ToggleFullscreen,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::ToggleFullscreen),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().SwitchWorkflowMode,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::SwitchWorkflowMode),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::HasWorkflowModes)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().OpenAssetCreationMenu,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::OpenAssetCreationMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanExecuteActions)
	);

	GlobalCommands->MapAction(
		FBACommands::Get().FocusSearchBox,
		FExecuteAction::CreateRaw(this, &FBAGlobalActions::FocusSearchBox),
		FCanExecuteAction::CreateRaw(this, &FBAGlobalActions::CanFocusSearchBox)
	);
}

void FBAGlobalActions::OpenBlueprintAssistDebugMenu() const
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("BADebugMenu"));
}

void FBAGlobalActions::OpenFocusSearchBoxMenu()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (!Window.IsValid())
	{
		return;
	}

	TSharedRef<SFocusSearchBoxMenu> Widget = SNew(SFocusSearchBoxMenu);

	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::OpenEditDetailsMenu()
{
	TSharedRef<SEditDetailsMenu> Widget = SNew(SEditDetailsMenu);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::OpenWindowMenu()
{
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();

	TSharedRef<SOpenWindowMenu> Widget = SNew(SOpenWindowMenu);

	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::OpenBlueprintAssistHotkeyMenu()
{
	TSharedRef<SBAHotkeyMenu> Widget = SNew(SBAHotkeyMenu).BindingContextName("BlueprintAssistCommands");
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::OpenTabSwitcherMenu()
{
	TSharedRef<SBATabSwitcher> Widget = SNew(SBATabSwitcher);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::ToggleFullscreen()
{
	TSharedPtr<SWindow> Window = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (Window.IsValid())
	{
		if (Window->IsWindowMaximized())
		{
			Window->Restore();
		}
		else
		{
			Window->Maximize();
		}
	}
}

void FBAGlobalActions::SwitchWorkflowMode()
{
	if (UObject* CurrentAsset = FBAMiscUtils::GetAssetForActiveTab<UObject>())
	{
		if (FWorkflowCentricApplication* App = static_cast<FWorkflowCentricApplication*>(FBAUtils::GetEditorFromActiveTab()))
		{
			const FString AssetClassName = CurrentAsset->GetClass()->GetName();
			if (AssetClassName == "WidgetBlueprint")
			{
				static const FName GraphMode(TEXT("GraphName"));
				static const FName DesignerMode(TEXT("DesignerName"));

				const FName& NewMode = App->IsModeCurrent(DesignerMode) ? GraphMode : DesignerMode;
				App->SetCurrentMode(NewMode);
				return;
			}
			else if (AssetClassName == "BehaviorTree")
			{
				static const FName BehaviorTreeMode(TEXT("BehaviorTree"));
				static const FName BlackboardMode(TEXT("Blackboard"));

				const FName& NewMode = App->IsModeCurrent(BehaviorTreeMode) ? BlackboardMode : BehaviorTreeMode;
				App->SetCurrentMode(NewMode);
				return;
			}
		}
	}
}

void FBAGlobalActions::OpenAssetCreationMenu()
{
	TSharedRef<SBACreateAssetMenu> Widget = SNew(SBACreateAssetMenu);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBAGlobalActions::FocusSearchBox()
{
	if (auto SearchBox = FindSearchBox())
	{
		FSlateApplication::Get().SetKeyboardFocus(SearchBox, EFocusCause::SetDirectly);
	}
}

bool FBAGlobalActions::CanFocusSearchBox() const
{
	return FindSearchBox().IsValid();
}

TSharedPtr<SWidget> FBAGlobalActions::FindSearchBox() const
{
	if (TSharedPtr<SWidget> FocusedWidget = FSlateApplication::Get().GetUserFocusedWidget(0))
	{
		if (TSharedPtr<SWidget> SearchBox = FBAUtils::ScanParentContainersForType(FocusedWidget, "SSearchBox", "SDockingTabStack"))
		{
			return SearchBox;
		}
	}

	return nullptr;
}
