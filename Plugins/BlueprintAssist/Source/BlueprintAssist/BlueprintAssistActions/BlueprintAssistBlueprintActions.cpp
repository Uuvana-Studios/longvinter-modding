#include "BlueprintAssistBlueprintActions.h"

#include "BlueprintAssistCommands.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditor.h"
#include "BlueprintAssist/BlueprintAssistUtils/BAMiscUtils.h"
#include "BlueprintAssist/BlueprintAssistWidgets/AddSymbolMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/GoToSymbolMenu.h"
#include "BlueprintAssist/BlueprintAssistWidgets/VariableSelectorMenu.h"
#include "Framework/Commands/UICommandList.h"

bool FBABlueprintActionsBase::HasOpenBlueprintEditor() const
{
	FBlueprintEditor* BPEditor = FBAMiscUtils::GetEditorFromActiveTabCasted<UBlueprint, FBlueprintEditor>();
	return BPEditor != nullptr && BPEditor->InEditingMode();
}

void FBABlueprintActions::Init()
{
	BlueprintCommands = MakeShareable(new FUICommandList());

	BlueprintCommands->MapAction(
		FBACommands::Get().VariableSelectorMenu,
		FExecuteAction::CreateRaw(this, &FBABlueprintActions::OpenVariableSelectorMenu),
		FCanExecuteAction::CreateRaw(this, &FBABlueprintActions::HasOpenBlueprintEditor)
	);

	BlueprintCommands->MapAction(
		FBACommands::Get().AddSymbolMenu,
		FExecuteAction::CreateRaw(this, &FBABlueprintActions::OpenCreateSymbolMenu),
		FCanExecuteAction::CreateRaw(this, &FBABlueprintActions::HasOpenBlueprintEditor)
	);

	BlueprintCommands->MapAction(
		FBACommands::Get().GoToInGraph,
		FExecuteAction::CreateRaw(this, &FBABlueprintActions::OpenGoToSymbolMenu),
		FCanExecuteAction::CreateRaw(this, &FBABlueprintActions::HasOpenBlueprintEditor)
	);
}

void FBABlueprintActions::OpenVariableSelectorMenu()
{
	TSharedRef<SVariableSelectorMenu> Widget = SNew(SVariableSelectorMenu);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBABlueprintActions::OpenCreateSymbolMenu()
{
	TSharedRef<SAddSymbolMenu> Widget = SNew(SAddSymbolMenu);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}

void FBABlueprintActions::OpenGoToSymbolMenu()
{
	TSharedRef<SGoToSymbolMenu> Widget = SNew(SGoToSymbolMenu);
	FBAUtils::OpenPopupMenu(Widget, Widget->GetWidgetSize());
}
