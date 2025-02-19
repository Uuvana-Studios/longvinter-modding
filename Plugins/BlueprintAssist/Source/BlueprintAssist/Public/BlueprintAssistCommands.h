// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistGlobals.h"
#include "EditorStyleSet.h"
#include "Framework/Commands/Commands.h"

/**
 * 
 */
class FBACommandsImpl : public TCommands<FBACommandsImpl>
{
public:
	FBACommandsImpl()
		: TCommands<FBACommandsImpl>(
			TEXT("BlueprintAssistCommands"),
			NSLOCTEXT("Contexts", "BlueprintAssistCommands", "Blueprint Assist Commands"),
			NAME_None,
			BA_GET_STYLE_SET_NAME()) { }

	virtual ~FBACommandsImpl() override { }

	virtual void RegisterCommands() override;

	/** Opens the blueprint context menu */
	TSharedPtr<FUICommandInfo> OpenContextMenu;

	/** Replace the selected node */
	TSharedPtr<FUICommandInfo> ReplaceNodeWith;

	/** Rename the selected node */
	TSharedPtr<FUICommandInfo> RenameSelectedNode;

	/** Formats the position of a chain of nodes */
	TSharedPtr<FUICommandInfo> FormatNodes;

	/** Formats the position for only the selected nodes */
	TSharedPtr<FUICommandInfo> FormatNodes_Selectively;

	/** Formats the position of a chain of nodes */
	TSharedPtr<FUICommandInfo> FormatNodes_Helixing;

	/** Formats the position of a chain of nodes */
	TSharedPtr<FUICommandInfo> FormatNodes_LHS;

	/** Formats the position of all custom events nodes on the graph */
	TSharedPtr<FUICommandInfo> FormatAllEvents;

	/** Deletes a node B in chain A-B-C and links A-B */
	TSharedPtr<FUICommandInfo> DeleteAndLink;

	/** Connects free pins of a node to nearby pins */
	TSharedPtr<FUICommandInfo> ConnectUnlinkedPins;

	TSharedPtr<FUICommandInfo> LinkNodesBetweenWires;

	/** Toggle context sensitive in the graph action menu */
	TSharedPtr<FUICommandInfo> ToggleContextSensitive;

	/** Selects the closest node in a direction */
	TSharedPtr<FUICommandInfo> SelectNodeUp;
	TSharedPtr<FUICommandInfo> SelectNodeDown;
	TSharedPtr<FUICommandInfo> SelectNodeLeft;
	TSharedPtr<FUICommandInfo> SelectNodeRight;

	/** Selects the pin in a direction */
	TSharedPtr<FUICommandInfo> SelectPinUp;
	TSharedPtr<FUICommandInfo> SelectPinDown;
	TSharedPtr<FUICommandInfo> SelectPinLeft;
	TSharedPtr<FUICommandInfo> SelectPinRight;

	/** Shifts the camera in a direction */
	TSharedPtr<FUICommandInfo> ShiftCameraUp;
	TSharedPtr<FUICommandInfo> ShiftCameraLeft;
	TSharedPtr<FUICommandInfo> ShiftCameraRight;
	TSharedPtr<FUICommandInfo> ShiftCameraDown;

	/** Selects all nodes down the chain in a direction */
	TSharedPtr<FUICommandInfo> SelectChainLeft;
	TSharedPtr<FUICommandInfo> SelectChainRight;

	/** Swap a node in a direction */
	TSharedPtr<FUICommandInfo> SwapNodeLeft;
	TSharedPtr<FUICommandInfo> SwapNodeRight;

	/** Link the selected pin to the hovered pin */
	TSharedPtr<FUICommandInfo> LinkToHoveredPin;

	/** Split the selected or hovered pin */
	TSharedPtr<FUICommandInfo> SplitPin;

	/** Recombine the selected or hovered pin */
	TSharedPtr<FUICommandInfo> RecombinePin;

	/** Go to command */
	TSharedPtr<FUICommandInfo> GoToInGraph;

	/** Open window menu */
	TSharedPtr<FUICommandInfo> OpenWindow;

	/** Duplicate node for each link */
	TSharedPtr<FUICommandInfo> DuplicateNodeForEachLink;

	/** Refresh node sizes */
	TSharedPtr<FUICommandInfo> RefreshNodeSizes;

	TSharedPtr<FUICommandInfo> EditSelectedPinValue;
	TSharedPtr<FUICommandInfo> DisconnectPinLink;
	TSharedPtr<FUICommandInfo> DisconnectNodeExecution;
	TSharedPtr<FUICommandInfo> DisconnectAllNodeLinks;

	TSharedPtr<FUICommandInfo> ZoomToNodeTree;

	TSharedPtr<FUICommandInfo> GetContextMenuForPin;
	TSharedPtr<FUICommandInfo> GetContextMenuForNode;

	TSharedPtr<FUICommandInfo> FocusSearchBoxMenu;
	TSharedPtr<FUICommandInfo> VariableSelectorMenu;
	TSharedPtr<FUICommandInfo> AddSymbolMenu;
	TSharedPtr<FUICommandInfo> EditDetailsMenu;
	TSharedPtr<FUICommandInfo> LinkPinMenu;
	TSharedPtr<FUICommandInfo> TabSwitcherMenu;

	TSharedPtr<FUICommandInfo> ToggleNode;

	TSharedPtr<FUICommandInfo> CreateRerouteNode;

	TSharedPtr<FUICommandInfo> OpenBlueprintAssistHotkeySheet;

	TSharedPtr<FUICommandInfo> ToggleFullscreen;

	TSharedPtr<FUICommandInfo> SwitchWorkflowMode;

	TSharedPtr<FUICommandInfo> OpenAssetCreationMenu;

	TSharedPtr<FUICommandInfo> FocusGraphPanel;

	TSharedPtr<FUICommandInfo> OpenBlueprintAssistDebugMenu;

	TSharedPtr<FUICommandInfo> FocusSearchBox;

	TSharedPtr<FUICommandInfo> GoToParentClassDefinition;
};

class BLUEPRINTASSIST_API FBACommands
{
public:
	static void Register();

	static const FBACommandsImpl& Get();

	static void Unregister();
};
