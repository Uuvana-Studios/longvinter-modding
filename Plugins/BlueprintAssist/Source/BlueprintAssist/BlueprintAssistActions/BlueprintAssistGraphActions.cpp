#include "BlueprintAssistGraphActions.h"

#include "BlueprintAssistCommands.h"
#include "BlueprintAssistGraphHandler.h"
#include "EdGraphSchema_K2_Actions.h"
#include "K2Node_Knot.h"
#include "ScopedTransaction.h"
#include "SGraphPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Kismet2/BlueprintEditorUtils.h"

bool FBAGraphActionsBase::HasGraph() const
{
	if (HasOpenTab())
	{
		if (TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler())
		{
			return GraphHandler->IsWindowActive();
		}
	}

	return false;
}

bool FBAGraphActionsBase::HasGraphNonReadOnly() const
{
	if (HasOpenTab())
	{
		if (TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler())
		{
			return GraphHandler->IsWindowActive() && !GraphHandler->IsGraphReadOnly();
		}
	}

	return false;
}

void FBAGraphActions::Init()
{
	GraphCommands = MakeShareable(new FUICommandList());
	GraphReadOnlyCommands = MakeShareable(new FUICommandList());

	////////////////////////////////////////////////////////////
	// Graph Commands
	////////////////////////////////////////////////////////////

	GraphCommands->MapAction(
		FBACommands::Get().FormatAllEvents,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::OnFormatAllEvents),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	GraphCommands->MapAction(
		FBACommands::Get().OpenContextMenu,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::OnOpenContextMenu),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	GraphCommands->MapAction(
		FBACommands::Get().DisconnectPinLink,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::DisconnectPinOrWire),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	GraphCommands->MapAction(
		FBACommands::Get().SplitPin,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::SplitPin),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	GraphCommands->MapAction(
		FBACommands::Get().RecombinePin,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::RecombinePin),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	GraphCommands->MapAction(
		FBACommands::Get().CreateRerouteNode,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::CreateRerouteNode),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraphNonReadOnly)
	);

	////////////////////////////////////////////////////////////
	// Graph Read Only Commands
	////////////////////////////////////////////////////////////

	GraphReadOnlyCommands->MapAction(
		FBACommands::Get().FocusGraphPanel,
		FExecuteAction::CreateRaw(this, &FBAGraphActions::FocusGraphPanel),
		FCanExecuteAction::CreateRaw(this, &FBAGraphActions::HasGraph)
	);
}

void FBAGraphActions::OnFormatAllEvents() const
{
	if (auto GraphHandler = GetGraphHandler())
	{
		GraphHandler->FormatAllEvents();
	}
}

void FBAGraphActions::OpenContextMenu(const FVector2D& MenuLocation, const FVector2D& NodeSpawnPosition)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraph* Graph = GraphHandler->GetFocusedEdGraph();
	if (!Graph)
	{
		return;
	}

	TSharedPtr<SGraphEditor> GraphEditor = GraphHandler->GetGraphEditor();
	if (!GraphEditor.IsValid())
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	const TArray<UEdGraphPin*> DummyPins;
	TSharedPtr<SWidget> WidgetToFocus = GraphPanel->SummonContextMenu(
		FSlateApplication::Get().GetCursorPos(),
		NodeSpawnPosition,
		nullptr,
		nullptr,
		DummyPins);

	// Focus the newly created context menu
	if (WidgetToFocus.IsValid())
	{
		FSlateApplication& SlateApp = FSlateApplication::Get();
		SlateApp.SetKeyboardFocus(WidgetToFocus);
	}
}

void FBAGraphActions::OpenContextMenuFromPin(UEdGraphPin* Pin, const FVector2D& MenuLocation, const FVector2D& NodeLocation)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	TArray<UEdGraphPin*> DragFromPins;
	DragFromPins.Add(Pin);
	TSharedPtr<SWidget> Widget = GraphPanel->SummonContextMenu(
		MenuLocation,
		NodeLocation,
		nullptr,
		nullptr,
		DragFromPins);

	FSlateApplication::Get().SetKeyboardFocus(Widget);
}

void FBAGraphActions::OnOpenContextMenu()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphEditor> GraphEditor = GraphHandler->GetGraphEditor();
	if (!GraphEditor.IsValid())
	{
		return;
	}

	const FVector2D MenuLocation = FSlateApplication::Get().GetCursorPos();
	const FVector2D SpawnLocation = GraphEditor->GetPasteLocation();

	GraphHandler->NodeToReplace = nullptr;

	UEdGraphPin* SelectedPin = GraphHandler->GetSelectedPin();

	if (SelectedPin != nullptr)
	{
		OpenContextMenuFromPin(SelectedPin, MenuLocation, SpawnLocation);
	}
	else
	{
		OpenContextMenu(MenuLocation, SpawnLocation);
	}
}

void FBAGraphActions::DisconnectPinOrWire()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();

	if (GraphPanel.IsValid())
	{
		TSharedPtr<SGraphPin> HoveredPin = FBAUtils::GetHoveredGraphPin(GraphPanel);
		if (HoveredPin.IsValid())
		{
			if (UEdGraphPin* Pin = HoveredPin->GetPinObj())
			{
				const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "DisconnectPinLink", "Disconnect Pin Link"));
				GraphPanel->GetGraphObj()->GetSchema()->BreakPinLinks(*Pin, true);
				return;
			}
		}
	}

	UEdGraphPin* SelectedPin = GraphHandler->GetSelectedPin();
	if (SelectedPin != nullptr)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "DisconnectPinLink", "DisconectPinLink"));
		GraphPanel->GetGraphObj()->GetSchema()->BreakPinLinks(*SelectedPin, true);
	}
}

void FBAGraphActions::SplitPin()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	auto EdGraph = GraphHandler->GetFocusedEdGraph();
	if (!EdGraph)
	{
		return;
	}

	UEdGraphPin* PinToUse = nullptr;

	TSharedPtr<SGraphPin> HoveredPin = FBAUtils::GetHoveredGraphPin(GraphPanel);
	if (HoveredPin.IsValid())
	{
		PinToUse = HoveredPin->GetPinObj();
	}

	if (PinToUse == nullptr)
	{
		PinToUse = GraphHandler->GetSelectedPin();
	}

	if (PinToUse != nullptr)
	{
		const UEdGraphSchema* Schema = EdGraph->GetSchema();

		if (PinToUse->GetOwningNode()->CanSplitPin(PinToUse))
		{
			const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "SplitPin", "Split Pin"));

			Schema->SplitPin(PinToUse);

			GraphHandler->SetSelectedPin(PinToUse->SubPins[0]);
		}
	}
}

void FBAGraphActions::RecombinePin()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
	if (!GraphPanel.IsValid())
	{
		return;
	}

	auto EdGraph = GraphHandler->GetFocusedEdGraph();
	if (!EdGraph)
	{
		return;
	}

	UEdGraphPin* PinToUse = nullptr;

	TSharedPtr<SGraphPin> HoveredPin = FBAUtils::GetHoveredGraphPin(GraphPanel);
	if (HoveredPin.IsValid())
	{
		PinToUse = HoveredPin->GetPinObj();
	}

	if (PinToUse == nullptr)
	{
		PinToUse = GraphHandler->GetSelectedPin();
	}

	if (PinToUse != nullptr)
	{
		const UEdGraphSchema* Schema = EdGraph->GetSchema();

		if (PinToUse->ParentPin != nullptr)
		{
			const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "RecombinePin", "Recombine Pin"));
			GraphHandler->SetSelectedPin(PinToUse->ParentPin);
			Schema->RecombinePin(PinToUse);
		}
	}
}

void FBAGraphActions::CreateRerouteNode()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
	const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
	const FVector2D GraphPosition = GraphPanel->PanelCoordToGraphCoord(GraphPanel->GetTickSpaceGeometry().AbsoluteToLocal(CursorPos));

	UEdGraphPin* PinToCreateFrom = nullptr;

	// get pin from knot node
	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();
	if (UK2Node_Knot* KnotNode = Cast<UK2Node_Knot>(SelectedNode))
	{
		if (GraphPosition.X > KnotNode->NodePosX)
		{
			PinToCreateFrom = KnotNode->GetOutputPin();
		}
		else
		{
			PinToCreateFrom = KnotNode->GetInputPin();
		}

		// if (!FBAUtils::IsPinLinked(KnotNode->GetOutputPin()))
		// {
		// 	PinToCreateFrom = KnotNode->GetOutputPin();
		// }
		// else if (!FBAUtils::IsPinLinked(KnotNode->GetInputPin()))
		// {
		// 	PinToCreateFrom = KnotNode->GetInputPin();
		// }
	}

	// get selected pin
	if (!PinToCreateFrom)
	{
		PinToCreateFrom = GraphHandler->GetSelectedPin();
	}

	if (!PinToCreateFrom)
	{
		return;
	}

	// get hovered graph pin
	auto HoveredGraphPin = FBAUtils::GetHoveredGraphPin(GraphHandler->GetGraphPanel());
	if (HoveredGraphPin.IsValid())
	{
		UEdGraphPin* HoveredPin = HoveredGraphPin->GetPinObj();

		if (FBAUtils::CanConnectPins(PinToCreateFrom, HoveredPin, true))
		{
			if (FBAUtils::TryCreateConnection(PinToCreateFrom, HoveredPin, true))
			{
				return;
			}
		}
	}

	UEdGraphPin* LinkedPin = PinToCreateFrom->LinkedTo.Num() > 0 ? PinToCreateFrom->LinkedTo[0] : nullptr;

	//@TODO: This constant is duplicated from inside of SGraphNodeKnot
	const FVector2D NodeSpacerSize(42.0f, 14.0f);

	FVector2D KnotTopLeft = GraphPosition;

	// Create a new knot
	UEdGraph* ParentGraph = PinToCreateFrom->GetOwningNode()->GetGraph();
	if (!FBlueprintEditorUtils::IsGraphReadOnly(ParentGraph))
	{
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "CreateRerouteNode_BlueprintAssist", "Create Reroute Node"));

		UK2Node_Knot* NewKnot = FEdGraphSchemaAction_K2NewNode::SpawnNode<UK2Node_Knot>(ParentGraph, KnotTopLeft - NodeSpacerSize * 0.5f, EK2NewNodeFlags::SelectNewNode);

		// Move the connections across (only notifying the knot, as the other two didn't really change)
		UEdGraphPin* NewKnotPin = (PinToCreateFrom->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin();

		PinToCreateFrom->MakeLinkTo(NewKnotPin);

		if (LinkedPin != nullptr)
		{
			PinToCreateFrom->BreakLinkTo(LinkedPin);
			UEdGraphPin* NewKnotPinForLinkedPin = (LinkedPin->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin();
			LinkedPin->MakeLinkTo(NewKnotPinForLinkedPin);
		}

		NewKnot->PostReconstructNode();

		TSharedPtr<SGraphPin> GraphPin = FBAUtils::GetGraphPin(GraphPanel, PinToCreateFrom);
		const float PinY = PinToCreateFrom->GetOwningNode()->NodePosY + GraphPin->GetNodeOffset().Y;
		const float HeightDiff = FMath::Abs(PinY - KnotTopLeft.Y);
		if (HeightDiff < 25)
		{
			NewKnot->NodePosY = PinY - NodeSpacerSize.Y * 0.5f;
		}

		// Dirty the blueprint
		if (UBlueprint* Blueprint = GraphHandler->GetBlueprint())
		{
			FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		}
	}
}

void FBAGraphActions::FocusGraphPanel()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	if (GraphHandler->GetGraphPanel())
	{
		FSlateApplication::Get().SetKeyboardFocus(GraphHandler->GetGraphPanel(), EFocusCause::SetDirectly);
	}
}
