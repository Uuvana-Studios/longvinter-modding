// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistCommands.h"

#include "BlueprintAssistGlobals.h"

#define LOCTEXT_NAMESPACE "BlueprintAssistCommands"

void FBACommandsImpl::RegisterCommands()
{
	UI_COMMAND(
		OpenContextMenu,
		"Open Blueprint Creation Menu",
		"Opens the blueprint creation menu for the selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Tab));

	UI_COMMAND(
		ReplaceNodeWith,
		"Replace Node With",
		"Opens the blueprint creation menu to replace the current node",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::H));

	UI_COMMAND(
		RenameSelectedNode,
		"Rename Selected Node",
		"Rename the selected variable, macro or function in a graph",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::F2));

	UI_COMMAND(
		FormatNodes,
		"Format Nodes",
		"Automatically positions all connected nodes",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::F));

	UI_COMMAND(
		FormatNodes_Selectively,
		"Format nodes selectively",
		"Only formats the selected nodes. If only 1 node is selected, formats the nodes to the right",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::F));

	UI_COMMAND(
		FormatNodes_Helixing,
		"Format Nodes using Helixing",
		"Forces helixing setting and formats nodes",
		EUserInterfaceActionType::Button,
		FInputChord());

	UI_COMMAND(
		FormatNodes_LHS,
		"Format Nodes using LHS",
		"Forces LHS setting and formats nodes",
		EUserInterfaceActionType::Button,
		FInputChord());

	UI_COMMAND(
		DeleteAndLink,
		"Delete and Keep Links",
		"Deletes the node in a chain A-B-C and links A-C",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::Delete));

	UI_COMMAND(
		LinkNodesBetweenWires,
		"Link nodes between wires",
		"Inserts the selected nodes between the highlighted wire",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Q));

	UI_COMMAND(
		ConnectUnlinkedPins,
		"Connect unlinked pins",
		"Attempts to connect any unlinked pins to any nearby nodes",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Q));

	UI_COMMAND(
		LinkToHoveredPin,
		"Link To Hovered Pin",
		"Links the selected pin to the hovered pin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Q));

	UI_COMMAND(
		SplitPin,
		"Split current pin",
		"Split the selected or hovered pin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt, EKeys::Q));

	UI_COMMAND(
		RecombinePin,
		"Recombine pin",
		"Recombine the selected or hovered pin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt | EModifierKey::Control, EKeys::Q));

	UI_COMMAND(
		FormatAllEvents,
		"Format All Events",
		"Repositions all custom events in the graph",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::R));

	UI_COMMAND(
		ToggleContextSensitive,
		"Toggle Context Sensitive Mode",
		"Toggles context sensitive mode in the blueprint creation menu",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::T));

	UI_COMMAND(
		SelectNodeUp,
		"Select Node Up",
		"Selects a node above the selected one",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Up));
	UI_COMMAND(
		SelectNodeDown,
		"Select Node Down",
		"Selects a node below the selected one",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Down));
	UI_COMMAND(
		SelectNodeLeft,
		"Select Node Left",
		"Selects a node left to the selected one",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Left));
	UI_COMMAND(
		SelectNodeRight,
		"Select Node Right",
		"Selects a node right to the selected one",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Right));

	UI_COMMAND(
		ShiftCameraUp,
		"Shift Camera Up",
		"Shifts the camera up",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::Up));
	UI_COMMAND(
		ShiftCameraDown,
		"Shift Camera Down",
		"Shifts the camera down",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::Down));
	UI_COMMAND(
		ShiftCameraLeft,
		"Shift Camera Left",
		"Shifts the camera left",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::Left));
	UI_COMMAND(
		ShiftCameraRight,
		"Shift Camera Right",
		"Shifts the camera right",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Shift, EKeys::Right));

	UI_COMMAND(
		SwapNodeLeft,
		"Swap Node Left",
		"Swaps node with the linked node to the left",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Left));

	UI_COMMAND(
		SwapNodeRight,
		"Swap Node Right",
		"Swaps node with the linked node to the right",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Right));

	UI_COMMAND(
		GoToInGraph,
		"Go to symbol in graph",
		"Go to a symbol in the current graph",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::G));

	UI_COMMAND(
		OpenWindow,
		"Open Window",
		"Opens the Open Window Menu",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::K));

	UI_COMMAND(
		DuplicateNodeForEachLink,
		"Duplicate Variable Node",
		"Creates a copy of the node for each link",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::V));

	UI_COMMAND(
		RefreshNodeSizes,
		"Refresh node sizes",
		"Recalculates the size of selected nodes (if no nodes are selected, all nodes are refreshed)",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::R));

	UI_COMMAND(
		EditSelectedPinValue,
		"Edit selected pin value",
		"Edit the value of the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::E));

	UI_COMMAND(
		DisconnectNodeExecution,
		"Disconnect execution on selected node",
		"Disconnect all execution pins on selected node",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::D));

	UI_COMMAND(
		DisconnectPinLink,
		"Disconnect pin link",
		"Disconnect selected pin or hovered wire",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::D));

	UI_COMMAND(
		DisconnectAllNodeLinks,
		"Disconnect links on selected node",
		"Disconnect all links on selected node",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::D));

	UI_COMMAND(
		ZoomToNodeTree,
		"Zoom to node tree",
		"Zoom to fit all nodes connected to the currently selected node",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Equals));

	UI_COMMAND(
		GetContextMenuForPin,
		"Get context menu actions for the selected pin",
		"Gets the context menu actions for the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::M));
	UI_COMMAND(
		GetContextMenuForNode,
		"Get context menu actions for the selected node",
		"Gets the context menu actions for the currently selected node",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::M));

	UI_COMMAND(
		SelectPinUp,
		"Select Pin Up",
		"Selects the pin above the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Up));
	UI_COMMAND(
		SelectPinDown,
		"Select Pin Down",
		"Selects the pin below the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Down));
	UI_COMMAND(
		SelectPinLeft,
		"Select Pin Left",
		"Selects the pin to the left of the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Left));
	UI_COMMAND(
		SelectPinRight,
		"Select Pin Right",
		"Selects the pin to the right of the currently selected pin",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::Right));

	UI_COMMAND(
		FocusSearchBoxMenu,
		"Search box menu",
		"Opens a menu that allows you to focus a seach box in the current window",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Alt, EKeys::F));

	UI_COMMAND(
		VariableSelectorMenu,
		"Variable selector menu",
		"Opens a menu that allows you to select variables",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::G));

	UI_COMMAND(
		AddSymbolMenu,
		"Create symbol menu",
		"Opens a menu that allows you create a symbol",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::A));

	UI_COMMAND(
		EditDetailsMenu,
		"Edit details menu",
		"Opens a menu that allows you to edit the current variable details",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::E));

	UI_COMMAND(
		LinkPinMenu,
		"Link pin menu",
		"Opens a menu that allows you to link to another pin on the graph",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::L));

	UI_COMMAND(
		TabSwitcherMenu,
		"Tab switcher menu",
		"Opens a menu that allows you to switch tabs",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::Tab));

	UI_COMMAND(
		ToggleNode,
		"Toggle node(s)",
		"Toggle selected nodes disabled state. Requires the setting 'Allow Explicit Impure Node Disabling' in Editor Preferences.",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::Slash));

	UI_COMMAND(
		CreateRerouteNode,
		"Create reroute node",
		"Creates a reroute node from the currently selected pin (or selected reroute node)",
		EUserInterfaceActionType::Button,
		FInputChord());

	UI_COMMAND(
		OpenBlueprintAssistHotkeySheet,
		"Open blueprint assist hotkey sheet",
		"Opens a menu displaying all the commands and hotkeys for the Blueprint Assist plugin",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::F1));

	UI_COMMAND(
		ToggleFullscreen,
		"Toggle Fullscreen",
		"Toggles fullscreen for the current window",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt, EKeys::Enter));

	UI_COMMAND(
		SwitchWorkflowMode,
		"Switch workflow mode",
		"Opens a menu which allows you to switch workflow mode",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt, EKeys::O));

	UI_COMMAND(
		OpenAssetCreationMenu,
		"Open asset creation menu",
		"Opens a menu which allows you to create a new asset",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Alt | EModifierKey::Control, EKeys::N));

	UI_COMMAND(
		FocusGraphPanel,
		"Focus graph panel",
		"Set keyboard focus to the graph panel if it is open",
		EUserInterfaceActionType::Button,
		FInputChord());

	UI_COMMAND(
		OpenBlueprintAssistDebugMenu,
		"Open Blueprint Assist Debug Menu",
		"Open the blueprint assist debug menu showing info about the asset editor, graph and more",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift | EModifierKey::Alt, EKeys::F12));

	UI_COMMAND(
		FocusSearchBox,
		"Focus search box",
		"Give keyboard focus to the search box in the focused tab",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control, EKeys::F));

	UI_COMMAND(
		GoToParentClassDefinition,
		"Go to parent class definition",
		"Navigate to the current asset's parent class in the Unreal or code editor",
		EUserInterfaceActionType::Button,
		FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::B));
}

void FBACommands::Register()
{
	UE_LOG(LogBlueprintAssist, Log, TEXT("Registered BlueprintAssist Commands"));
	FBACommandsImpl::Register();
}

const FBACommandsImpl& FBACommands::Get()
{
	return FBACommandsImpl::Get();
}

void FBACommands::Unregister()
{
	UE_LOG(LogBlueprintAssist, Log, TEXT("Unregistered BlueprintAssist Commands"));
	return FBACommandsImpl::Unregister();
}

#undef LOCTEXT_NAMESPACE
