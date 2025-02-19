#include "BlueprintAssistNodeActions.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Variable.h"
#include "ScopedTransaction.h"
#include "SGraphActionMenu.h"
#include "SGraphPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Kismet2/BlueprintEditorUtils.h"

bool FBANodeActionsBase::HasSingleNodeSelected() const
{
	return HasGraphNonReadOnly() ? (GetGraphHandler()->GetSelectedNode() != nullptr) : false;
}

bool FBANodeActionsBase::HasMultipleNodesSelected() const
{
	return HasGraphNonReadOnly() ? (GetGraphHandler()->GetSelectedNodes().Num() > 0) : false;
}

bool FBANodeActionsBase::HasMultipleNodesSelectedInclComments() const
{
	return HasGraphNonReadOnly() ? (GetGraphHandler()->GetSelectedNodes(true).Num() > 0) : false;
}

void FBANodeActions::Init()
{
	SingleNodeCommands = MakeShareable(new FUICommandList());
	MultipleNodeCommands = MakeShareable(new FUICommandList());
	MultipleNodeCommandsIncludingComments = MakeShareable(new FUICommandList());

	////////////////////////////////////////////////////////////
	// Single Node Commands
	////////////////////////////////////////////////////////////

	SingleNodeCommands->MapAction(
		FBACommands::Get().ConnectUnlinkedPins,
		FExecuteAction::CreateRaw(this, &FBANodeActions::OnSmartWireSelectedNode),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasSingleNodeSelected)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().ZoomToNodeTree,
		FExecuteAction::CreateRaw(this, &FBANodeActions::ZoomToNodeTree),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasSingleNodeSelected)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().DisconnectAllNodeLinks,
		FExecuteAction::CreateRaw(this, &FBANodeActions::DisconnectAllNodeLinks),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasSingleNodeSelected)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().SelectPinUp,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SelectPinInDirection, 0, -1),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanSelectPinInDirection)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().SelectPinDown,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SelectPinInDirection, 0, 1),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanSelectPinInDirection)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().SelectPinLeft,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SelectPinInDirection, -1, 0),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanSelectPinInDirection)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().SelectPinRight,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SelectPinInDirection, 1, 0),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanSelectPinInDirection)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().GetContextMenuForNode,
		FExecuteAction::CreateStatic(&FBANodeActions::OnGetContextMenuActions, false),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasSingleNodeSelected)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().ReplaceNodeWith,
		FExecuteAction::CreateRaw(this, &FBANodeActions::ReplaceNodeWith),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasSingleNodeSelected)
	);

	SingleNodeCommands->MapAction(
		FBACommands::Get().RenameSelectedNode,
		FExecuteAction::CreateRaw(this, &FBANodeActions::RenameSelectedNode),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanRenameSelectedNode)
	);

	////////////////////////////////////////////////////////////
	// Multiple Node Commands
	////////////////////////////////////////////////////////////

	MultipleNodeCommands->MapAction(
		FBACommands::Get().FormatNodes,
		FExecuteAction::CreateRaw(this, &FBANodeActions::FormatNodes),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().FormatNodes_Selectively,
		FExecuteAction::CreateRaw(this, &FBANodeActions::FormatNodesSelectively),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().FormatNodes_Helixing,
		FExecuteAction::CreateRaw(this, &FBANodeActions::FormatNodesWithHelixing),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().FormatNodes_LHS,
		FExecuteAction::CreateRaw(this, &FBANodeActions::FormatNodesWithLHS),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().LinkNodesBetweenWires,
		FExecuteAction::CreateRaw(this, &FBANodeActions::LinkNodesBetweenWires),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().DisconnectNodeExecution,
		FExecuteAction::CreateRaw(this, &FBANodeActions::DisconnectExecutionOfSelectedNode),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().SwapNodeLeft,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SwapNodeInDirection, EGPD_Input),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().SwapNodeRight,
		FExecuteAction::CreateRaw(this, &FBANodeActions::SwapNodeInDirection, EGPD_Output),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().DeleteAndLink,
		FExecuteAction::CreateRaw(this, &FBANodeActions::DeleteAndLink),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelected)
	);

	MultipleNodeCommands->MapAction(
		FBACommands::Get().ToggleNode,
		FExecuteAction::CreateRaw(this, &FBANodeActions::ToggleNodes),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::CanToggleNodes)
	);

	////////////////////////////////////////////////////////////
	// Multiple Node Including Comments Commands
	////////////////////////////////////////////////////////////

	MultipleNodeCommandsIncludingComments->MapAction(
		FBACommands::Get().RefreshNodeSizes,
		FExecuteAction::CreateRaw(this, &FBANodeActions::RefreshNodeSizes),
		FCanExecuteAction::CreateRaw(this, &FBANodeActions::HasMultipleNodesSelectedInclComments)
	);
}

void FBANodeActions::SmartWireNode(UEdGraphNode* Node)
{
	auto GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	if (!FBAUtils::IsGraphNode(Node))
	{
		return;
	}

	UEdGraph* Graph = GraphHandler->GetFocusedEdGraph();
	if (!Graph)
	{
		return;
	}

	TSet<UEdGraphNode*> LHSNodes, RHSNodes;
	TSet<UEdGraphPin*> LHSPins, RHSPins;
	FBAUtils::SortNodesOnGraphByDistance(Node, Graph, LHSNodes, RHSNodes, LHSPins, RHSPins);

	TArray<TArray<UEdGraphPin*>> PinsByType;
	TArray<UEdGraphPin*> ExecPins = FBAUtils::GetExecPins(Node);
	TArray<UEdGraphPin*> ParamPins = FBAUtils::GetParameterPins(Node);
	PinsByType.Add(ExecPins);
	PinsByType.Add(ParamPins);
	for (const TArray<UEdGraphPin*>& Pins : PinsByType)
	{
		for (UEdGraphPin* PinA : Pins)
		{
			// skip if pin is hidden or if the pin already is connected
			if (PinA->bHidden || PinA->LinkedTo.Num() > 0 || PinA->Direction == EGPD_MAX)
			{
				continue;
			}

			// check all pins to the left if we are an input pin
			// check all pins to the right if we are an output pin
			bool IsInputPin = PinA->Direction == EGPD_Input;
			for (UEdGraphPin* PinB : IsInputPin ? LHSPins : RHSPins)
			{
				// skip if has connection
				if (PinB->LinkedTo.Num() > 0)
				{
					continue;
				}

				// UE_LOG(LogBlueprintAssist, Warning, TEXT("Checking pins %s %s"), *FBAUtils::GetPinName(PinA), *FBAUtils::GetPinName(PinB));

				//bool bShouldOverrideLink = FBlueprintAssistUtils::IsExecPin(PinA);
				if (!FBAUtils::CanConnectPins(PinA, PinB, false, false, false))
				{
					// UE_LOG(LogBlueprintAssist, Warning, TEXT("\tSkipping"));
					continue;
				}

				TSharedPtr<FScopedTransaction> Transaction = MakeShareable(
					new FScopedTransaction(
						NSLOCTEXT("UnrealEd", "ConnectUnlinkedPins", "Connect Unlinked Pins")
					));

				FBAUtils::TryLinkPins(PinA, PinB);

				if (UBASettings::GetFormatterSettings(Graph).GetAutoFormatting() != EBAAutoFormatting::Never)
				{
					FEdGraphFormatterParameters FormatterParams;
					if (UBASettings::GetFormatterSettings(Graph).GetAutoFormatting() == EBAAutoFormatting::FormatSingleConnected)
					{
						FormatterParams.NodesToFormat.Add(PinA->GetOwningNode());
						FormatterParams.NodesToFormat.Add(PinB->GetOwningNode());
					}

					GraphHandler->AddPendingFormatNodes(PinA->GetOwningNode(), Transaction, FormatterParams);
				}
				else
				{
					Transaction.Reset();
				}

				return;
			}
		}
	}
}

void FBANodeActions::DisconnectExecutionOfNodes(TArray<UEdGraphNode*> Nodes)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	// TODO: Make this work for pure nodes
	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "DisconnectExecutionForNodes", "Disconnect Execution for Nodes"));

	if (Nodes.Num() == 0)
	{
		Transaction.Cancel();
		return;
	}

	int CountError = 0;

	const UEdGraphSchema* Schema = GraphHandler->GetFocusedEdGraph()->GetSchema();

	while (Nodes.Num() > 0)
	{
		CountError += 1;
		if (CountError > 1000)
		{
			UE_LOG(LogBlueprintAssist, Error, TEXT("DisconnectExecutionOfNodes caused infinite loop: Please report this on the wiki"));
			return;
		}

		UEdGraphNode* NextNode = Nodes[0];

		const auto Filter = [&Nodes](UEdGraphNode* Node)
		{
			return Nodes.Contains(Node);
		};

		TArray<UEdGraphNode*> NodeTree = FBAUtils::GetExecutionTreeWithFilter(NextNode, Filter).Array();
		if (NodeTree.Num() > 0)
		{
			const auto InNodeTree = [&NodeTree](UEdGraphNode* Node)
			{
				return NodeTree.Contains(Node);
			};

			UEdGraphNode* LeftMostNode = FBAUtils::GetTopMostWithFilter(NodeTree[0], EGPD_Input, InNodeTree);
			UEdGraphNode* RightMostNode = FBAUtils::GetTopMostWithFilter(NodeTree[0], EGPD_Output, InNodeTree);

			TArray<UEdGraphPin*> LinkedToInput = FBAUtils::GetLinkedToPins(LeftMostNode, EGPD_Input).FilterByPredicate(FBAUtils::IsExecPin);
			TArray<UEdGraphPin*> LinkedToOutput = FBAUtils::GetLinkedToPins(RightMostNode, EGPD_Output).FilterByPredicate(FBAUtils::IsExecPin);

			for (UEdGraphPin* Input : LinkedToInput)
			{
				for (UEdGraphPin* Output : LinkedToOutput)
				{
					if (FBAUtils::CanConnectPins(Input, Output, true, false))
					{
						Input->MakeLinkTo(Output);
						break;
					}
				}
			}

			const TArray<UEdGraphPin*> LeftMostInputPins = FBAUtils::GetExecPins(LeftMostNode, EGPD_Input);
			const TArray<UEdGraphPin*> RightMostOutputPins = FBAUtils::GetExecPins(RightMostNode, EGPD_Output);

			TArray<FBANodePinHandle> PinsToBreak;
			PinsToBreak.Reserve(LeftMostInputPins.Num() + RightMostOutputPins.Num());

			for (UEdGraphPin* Pin : LeftMostInputPins)
			{
				PinsToBreak.Add(FBANodePinHandle(Pin));
			}

			for (UEdGraphPin* Pin : RightMostOutputPins)
			{
				PinsToBreak.Add(FBANodePinHandle(Pin));
			}

			for (auto& Pin : PinsToBreak)
			{
				Schema->BreakPinLinks(*Pin.GetPin(), true);
			}
		}

		for (UEdGraphNode* Node : NodeTree)
		{
			Nodes.RemoveSwap(Node);
		}
	}
}

void FBANodeActions::OnSmartWireSelectedNode()
{
	UEdGraphNode* SelectedNode = GetGraphHandler()->GetSelectedNode();
	if (SelectedNode == nullptr)
	{
		return;
	}

	//const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "SmartWire", "Smart Wire Node"));
	SmartWireNode(SelectedNode);
}

void FBANodeActions::ZoomToNodeTree()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();
	if (SelectedNode == nullptr)
	{
		return;
	}

	TSet<UEdGraphNode*> NodeTree = FBAUtils::GetNodeTree(SelectedNode);

	// selecting a set of nodes requires the ptrs to be const
	TSet<const UEdGraphNode*> ConstNodeTree;
	for (UEdGraphNode* Node : NodeTree)
	{
		ConstNodeTree.Add(Node);
	}

	TSharedPtr<SGraphEditor> GraphEditor = GraphHandler->GetGraphEditor();
	GraphHandler->GetFocusedEdGraph()->SelectNodeSet(ConstNodeTree);
	GraphHandler->GetGraphEditor()->ZoomToFit(true);
}

void FBANodeActions::DisconnectAllNodeLinks()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();
	const UEdGraphSchema* Schema = GraphHandler->GetFocusedEdGraph()->GetSchema();
	if (SelectedNode != nullptr)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "DisconnectAllNodeLinks", "Disconnect All Node Links"));

		Schema->BreakNodeLinks(*SelectedNode);
	}
}

bool FBANodeActions::CanSelectPinInDirection()
{
	return HasSingleNodeSelected() && !FBAUtils::IsKnotNode(GetGraphHandler()->GetSelectedNode());
}

void FBANodeActions::SelectPinInDirection(int X, int Y) const
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();
	if (SelectedNode == nullptr)
	{
		GraphHandler->SetSelectedPin(nullptr);
		return;
	}

	if (FBAUtils::IsCommentNode(SelectedNode) || FBAUtils::IsKnotNode(SelectedNode))
	{
		GraphHandler->SetSelectedPin(nullptr);
		return;
	}

	const TArray<UEdGraphPin*> PinsOnSelectedNode = FBAUtils::GetPinsByDirection(SelectedNode);
	if (PinsOnSelectedNode.Num() == 0)
	{
		GraphHandler->SetSelectedPin(nullptr);
		return;
	}

	UEdGraphPin* SelectedPin = GraphHandler->GetSelectedPin();

	if (SelectedPin == nullptr)
	{
		GraphHandler->SetSelectedPin(FBAUtils::GetPinsByDirection(SelectedNode)[0]);
	}
	else
	{
		if (SelectedPin->GetOwningNode() != SelectedNode)
		{
			GraphHandler->SetSelectedPin(FBAUtils::GetPinsByDirection(SelectedNode)[0]);
		}
		else
		{
			const auto& IsPinVisibleAsAdvanced = [&](UEdGraphPin* Pin)
			{
				TSharedPtr<SGraphPin> GraphPin = FBAUtils::GetGraphPin(GraphHandler->GetGraphPanel(), Pin);
				return GraphPin.IsValid() &&
					GraphPin->IsPinVisibleAsAdvanced() == EVisibility::Visible;
			};

			if (X != 0) // x direction - switch to the opposite pins on the current node
			{
				// if we try to move the same direction as the selected pin, move to linked node instead
				if (X < 0 && SelectedPin->Direction == EGPD_Input ||
					X > 0 && SelectedPin->Direction == EGPD_Output)
				{
					const TArray<UEdGraphPin*> LinkedToIgnoringKnots = FBAUtils::GetPinLinkedToIgnoringKnots(SelectedPin);
					if (LinkedToIgnoringKnots.Num() > 0)
					{
						GraphHandler->SetSelectedPin(LinkedToIgnoringKnots[0], true);
					}
					return;
				}

				auto Direction = UEdGraphPin::GetComplementaryDirection(SelectedPin->Direction);

				TArray<UEdGraphPin*> Pins = FBAUtils::GetPinsByDirection(SelectedNode, Direction).FilterByPredicate(IsPinVisibleAsAdvanced);

				if (Pins.Num() > 0)
				{
					const int32 PinIndex = FBAUtils::GetPinIndex(SelectedPin);

					if (PinIndex != -1)
					{
						const int32 NextPinIndex = FMath::Min(Pins.Num() - 1, PinIndex);
						if (Pins.Num() > 0)
						{
							GraphHandler->SetSelectedPin(Pins[NextPinIndex]);
						}
					}
				}
			}
			else if (Y != 0) // y direction - move the selected pin up / down
			{
				TArray<UEdGraphPin*> Pins =
					FBAUtils::GetPinsByDirection(SelectedNode, SelectedPin->Direction)
					.FilterByPredicate(IsPinVisibleAsAdvanced);

				if (Pins.Num() > 1)
				{
					int32 PinIndex;
					Pins.Find(SelectedPin, PinIndex);
					if (PinIndex != -1) // we couldn't find the pin index
					{
						int32 NextPinIndex = PinIndex + Y;

						if (NextPinIndex < 0)
						{
							NextPinIndex = Pins.Num() + NextPinIndex;
						}
						else
						{
							NextPinIndex = NextPinIndex % Pins.Num();
						}

						GraphHandler->SetSelectedPin(Pins[NextPinIndex]);
					}
				}
			}
		}
	}
}

void FBANodeActions::OnGetContextMenuActions(const bool bUsePin)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraph* EdGraph = GraphHandler->GetFocusedEdGraph();
	if (EdGraph == nullptr)
	{
		return;
	}

	const UEdGraphSchema* Schema = EdGraph->GetSchema();
	if (Schema == nullptr)
	{
		return;
	}

	TSharedPtr<SGraphEditor> GraphEditor = GraphHandler->GetGraphEditor();
	const FVector2D MenuLocation = FSlateApplication::Get().GetCursorPos();
	const FVector2D SpawnLocation = GraphEditor->GetPasteLocation();

	UEdGraphNode* Node = GraphHandler->GetSelectedNode();

	UEdGraphPin* Pin = bUsePin
		? GraphHandler->GetSelectedPin()
		: nullptr;

	const TArray<UEdGraphPin*> DummyPins;
	GraphHandler->GetGraphPanel()->SummonContextMenu(MenuLocation, SpawnLocation, Node, Pin, DummyPins);
}

void FBANodeActions::ReplaceNodeWith()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();
	if (SelectedNode == nullptr || !SelectedNode->CanUserDeleteNode())
	{
		return;
	}

	TSharedPtr<SGraphEditor> GraphEditor = GraphHandler->GetGraphEditor();
	if (!GraphEditor.IsValid())
	{
		return;
	}

	const FVector2D MenuLocation = FSlateApplication::Get().GetCursorPos();
	const FVector2D SpawnLocation(SelectedNode->NodePosX, SelectedNode->NodePosY);

	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "ReplaceNodeWith", "Replace Node With")));

	FBAGraphActions::OpenContextMenu(MenuLocation, SpawnLocation);

	GraphHandler->NodeToReplace = SelectedNode;
	GraphHandler->SetReplaceNewNodeTransaction(Transaction);

	FSlateApplication& SlateApp = FSlateApplication::Get();
	if (SlateApp.IsInitialized())
	{
		TSharedPtr<SWindow> Menu = SlateApp.GetActiveTopLevelWindow();
		if (Menu.IsValid())
		{
			if (FBAUtils::GetGraphActionMenu().IsValid())
			{
#if ENGINE_MINOR_VERSION < 22 && ENGINE_MAJOR_VERSION == 4
				Menu->SetOnWindowClosed(FOnWindowClosed::CreateRaw(this, &FBAInputProcessor::OnReplaceNodeMenuClosed));
#else
				Menu->GetOnWindowClosedEvent().AddRaw(this, &FBANodeActions::OnReplaceNodeMenuClosed);
#endif
			}
		}
	}
}

void FBANodeActions::OnReplaceNodeMenuClosed(const TSharedRef<SWindow>& Window)
{
	GetGraphHandler()->ResetSingleNewNodeTransaction();
}

bool FBANodeActions::CanRenameSelectedNode()
{
	if (HasSingleNodeSelected())
	{
		UEdGraphNode* SelectedNode = GetGraphHandler()->GetSelectedNode();
		return SelectedNode->IsA(UK2Node_Variable::StaticClass()) ||
			SelectedNode->IsA(UK2Node_CallFunction::StaticClass()) ||
			SelectedNode->IsA(UK2Node_MacroInstance::StaticClass());
	}

	return false;
}

void FBANodeActions::RenameSelectedNode()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	UEdGraphNode* SelectedNode = GraphHandler->GetSelectedNode();

	FName ItemName;

	if (UK2Node_Variable* VariableNode = Cast<UK2Node_Variable>(SelectedNode))
	{
		ItemName = VariableNode->GetVarName();
	}
	else if (UK2Node_CallFunction* FunctionCall = Cast<UK2Node_CallFunction>(SelectedNode))
	{
		ItemName = FunctionCall->FunctionReference.GetMemberName();
	}
	else if (UK2Node_MacroInstance* Macro = Cast<UK2Node_MacroInstance>(SelectedNode))
	{
		ItemName = Macro->GetMacroGraph()->GetFName();
	}

	TSharedPtr<SGraphActionMenu> ActionMenu = FBAUtils::GetGraphActionMenu();
	if (!ActionMenu)
	{
		return;
	}

	if (!ItemName.IsNone())
	{
		ActionMenu->SelectItemByName(ItemName, ESelectInfo::OnKeyPress);
		if (ActionMenu->CanRequestRenameOnActionNode())
		{
			ActionMenu->OnRequestRenameOnActionNode();
		}
	}
}

void FBANodeActions::FormatNodes()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "FormatNode", "Format Node")));
	for (UEdGraphNode* Node : SelectedNodes)
	{
		if (FBAUtils::IsGraphNode(Node))
		{
			GraphHandler->AddPendingFormatNodes(Node, Transaction);
		}
	}
}

void FBANodeActions::FormatNodesSelectively()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	// TODO: Make selective formatting work with formatters other than EdGraph
	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "FormatOnlySelectedNodes", "Format Only Selected Nodes")));

	if (SelectedNodes.Num() == 1)
	{
		UEdGraphNode* SelectedNode = SelectedNodes.Array()[0];

		EEdGraphPinDirection Direction = FBAUtils::IsNodeImpure(SelectedNode) ? EGPD_Output : EGPD_Input;

		SelectedNodes = FBAUtils::GetNodeTree(SelectedNode, Direction, true);
	}

	for (UEdGraphNode* Node : SelectedNodes)
	{
		if (FBAUtils::IsGraphNode(Node))
		{
			FEdGraphFormatterParameters FormatterParameters;
			FormatterParameters.NodesToFormat = SelectedNodes.Array();
			GraphHandler->AddPendingFormatNodes(Node, Transaction, FormatterParameters);
		}
	}
}

void FBANodeActions::FormatNodesWithHelixing()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "FormatNodeHelixing", "Format Node with Helixing")));
	for (UEdGraphNode* Node : SelectedNodes)
	{
		if (FBAUtils::IsGraphNode(Node))
		{
			FEdGraphFormatterParameters FormatterParameters;
			FormatterParameters.OverrideFormattingStyle = MakeShareable(new EBAParameterFormattingStyle(EBAParameterFormattingStyle::Helixing));
			GraphHandler->AddPendingFormatNodes(Node, Transaction, FormatterParameters);
		}
	}
}

void FBANodeActions::FormatNodesWithLHS()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "FormatNodeLHS", "Format Node with LHS")));
	for (UEdGraphNode* Node : SelectedNodes)
	{
		if (FBAUtils::IsGraphNode(Node))
		{
			FEdGraphFormatterParameters FormatterParameters;
			FormatterParameters.OverrideFormattingStyle = MakeShareable(new EBAParameterFormattingStyle(EBAParameterFormattingStyle::LeftSide));
			GraphHandler->AddPendingFormatNodes(Node, Transaction, FormatterParameters);
		}
	}
}

void FBANodeActions::LinkNodesBetweenWires()
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

	TSharedPtr<SGraphPin> GraphPinForHoveredWire = FBAUtils::GetHoveredGraphPin(GraphHandler->GetGraphPanel());
	if (!GraphPinForHoveredWire.IsValid())
	{
		return;
	}

	UEdGraphPin* PinForHoveredWire = GraphPinForHoveredWire->GetPinObj();
	if (PinForHoveredWire == nullptr)
	{
		return;
	}

	TArray<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes().Array();

	if (SelectedNodes.Num() == 0)
	{
		return;
	}

	const auto LeftMostSort = [](const UEdGraphNode& NodeA, const UEdGraphNode& NodeB)
	{
		return NodeA.NodePosX < NodeB.NodePosX;
	};
	SelectedNodes.Sort(LeftMostSort);

	const auto IsSelected = [&SelectedNodes](UEdGraphNode* Node)
	{
		return SelectedNodes.Contains(Node);
	};

	UEdGraphNode* LeftMostNode =
		FBAUtils::GetTopMostWithFilter(SelectedNodes[0], EGPD_Input, IsSelected);

	UEdGraphNode* RightMostNode =
		FBAUtils::GetTopMostWithFilter(SelectedNodes[0], EGPD_Output, IsSelected);

	TSharedPtr<FScopedTransaction> Transaction =
		MakeShareable(
			new FScopedTransaction(
				NSLOCTEXT("UnrealEd", "LinkNodesBetweenWires", "Link Nodes Between Wires")));

	UEdGraphNode* First = PinForHoveredWire->Direction == EGPD_Output
		? LeftMostNode
		: RightMostNode;

	bool bCancelTransaction = true;

	TArray<FPinLink> PendingLinks;
	PendingLinks.Reserve(2);

	for (UEdGraphPin* Pin : First->Pins)
	{
		if (FBAUtils::CanConnectPins(PinForHoveredWire, Pin, true, false, false))
		{
			PendingLinks.Add(FPinLink(Pin, PinForHoveredWire));
			break;
		}
	}

	UEdGraphPin* ConnectedPin
		= PinForHoveredWire->LinkedTo.Num() > 0
		? PinForHoveredWire->LinkedTo[0]
		: nullptr;

	if (ConnectedPin != nullptr)
	{
		UEdGraphNode* ConnectedNode =
			PinForHoveredWire->Direction == EGPD_Output ? RightMostNode : LeftMostNode;

		for (UEdGraphPin* Pin : ConnectedNode->Pins)
		{
			if (FBAUtils::CanConnectPins(ConnectedPin, Pin, true, false, false))
			{
				PendingLinks.Add(FPinLink(Pin, ConnectedPin));
				break;
			}
		}
	}

	FEdGraphFormatterParameters FormatterParams;
	if (UBASettings::GetFormatterSettings(Graph).GetAutoFormatting() == EBAAutoFormatting::FormatSingleConnected)
	{
		FormatterParams.NodesToFormat.Append(SelectedNodes);
		FormatterParams.NodesToFormat.Add(PinForHoveredWire->GetOwningNode());
	}

	for (FPinLink& Link : PendingLinks)
	{
		Link.From->BreakAllPinLinks();

		const bool bMadeLink = FBAUtils::TryCreateConnection(Link.From, Link.To);
		if (bMadeLink)
		{
			if (UBASettings::GetFormatterSettings(Graph).GetAutoFormatting() != EBAAutoFormatting::Never)
			{
				GraphHandler->AddPendingFormatNodes(Link.GetFromNode(), Transaction, FormatterParams);
				GraphHandler->AddPendingFormatNodes(Link.GetToNode(), Transaction, FormatterParams);
			}

			bCancelTransaction = false;
		}
	}

	if (bCancelTransaction)
	{
		Transaction->Cancel();
	}
}

void FBANodeActions::DisconnectExecutionOfSelectedNode()
{
	TArray<UEdGraphNode*> SelectedNodes = GetGraphHandler()->GetSelectedNodes().Array();
	DisconnectExecutionOfNodes(SelectedNodes);
}

void FBANodeActions::SwapNodeInDirection(EEdGraphPinDirection Direction)
{
	// PinA: Linked to pin in direction
	// PinB: Linked to pin opposite
	// PinC: Linked to PinA's Node in direction

	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	auto GraphHandlerCapture = GraphHandler;
	const auto TopMostPinSort = [GraphHandlerCapture](UEdGraphPin& PinA, UEdGraphPin& PinB)
	{
		return GraphHandlerCapture->GetPinY(&PinA) < GraphHandlerCapture->GetPinY(&PinB);
	};

	TArray<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes().Array();

	if (SelectedNodes.Num() == 0)
	{
		return;
	}

	const UEdGraphSchema* Schema = GraphHandler->GetFocusedEdGraph()->GetSchema();
	if (!Schema)
	{
		return;
	}

	const auto IsSelectedAndPure = [&SelectedNodes](UEdGraphNode* Node)
	{
		return FBAUtils::IsNodeImpure(Node) && SelectedNodes.Contains(Node) && FBAUtils::HasExecInOut(Node);
	};

	UEdGraphNode* LeftMostNode = FBAUtils::GetTopMostWithFilter(SelectedNodes[0], EGPD_Input, IsSelectedAndPure);

	UEdGraphNode* RightMostNode = FBAUtils::GetTopMostWithFilter(SelectedNodes[0], EGPD_Output, IsSelectedAndPure);

	UEdGraphNode* NodeInDirection = Direction == EGPD_Input ? LeftMostNode : RightMostNode;
	UEdGraphNode* NodeOpposite = Direction == EGPD_Input ? RightMostNode : LeftMostNode;

	// Process NodeInDirection
	TArray<UEdGraphPin*> LinkedPins =
		FBAUtils::GetLinkedPins(NodeInDirection, Direction).FilterByPredicate(FBAUtils::IsExecPin);

	if (LinkedPins.Num() == 0)
	{
		return;
	}

	FBANodePinHandle PinInDirection(LinkedPins[0]);
	if (PinInDirection.GetPin()->LinkedTo.Num() == 0)
	{
		return;
	}

	// Process NodeOpposite
	const auto OppositeDirection = UEdGraphPin::GetComplementaryDirection(Direction);
	TArray<UEdGraphPin*> PinsOpposite = FBAUtils::GetPinsByDirection(NodeOpposite, OppositeDirection).FilterByPredicate(FBAUtils::IsExecPin);
	if (PinsOpposite.Num() == 0)
	{
		return;
	}

	FBANodePinHandle PinOpposite = PinsOpposite[0];

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("PinInDirection %s (%s)"), *FBAUtils::GetPinName(PinInDirection), *FBAUtils::GetNodeName(PinInDirection->GetOwningNode()));
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("PinOpposite %s (%s)"), *FBAUtils::GetPinName(PinOpposite), *FBAUtils::GetNodeName(PinOpposite->GetOwningNode()));

	// Process NodeA
	auto PinInDLinkedTo = PinInDirection.GetPin()->LinkedTo;
	PinInDLinkedTo.StableSort(TopMostPinSort);
	FBANodePinHandle PinA(PinInDLinkedTo[0]);
	// UE_LOG(LogBlueprintAssist, Warning, TEXT("PinA %s (%s)"), *FBAUtils::GetPinName(PinA), *FBAUtils::GetNodeName(PinA->GetOwningNode()));
	UEdGraphNode* NodeA = PinA.GetNode();

	if (!FBAUtils::HasExecInOut(NodeA))
	{
		return;
	}

	TArray<FPinLink> PendingConnections;
	PendingConnections.Reserve(3);

	TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "SwapNodes", "Swap Nodes")));

	{
		TArray<UEdGraphPin*> PinsAInDirection = FBAUtils::GetPinsByDirection(NodeA, Direction).FilterByPredicate(FBAUtils::IsExecPin);
		if (PinsAInDirection.Num() > 0)
		{
			UEdGraphPin* PinAInDirection = PinsAInDirection[0];
			PendingConnections.Add(FPinLink(PinAInDirection, PinOpposite.GetPin()));

			// Optional PinB
			if (PinAInDirection->LinkedTo.Num() > 0)
			{
				PinAInDirection->LinkedTo.StableSort(TopMostPinSort);
				UEdGraphPin* PinB = PinAInDirection->LinkedTo[0];
				// UE_LOG(LogBlueprintAssist, Warning, TEXT("PinB %s (%s)"), *FBAUtils::GetPinName(PinB), *FBAUtils::GetNodeName(PinB->GetOwningNode()));
				PendingConnections.Add(FPinLink(PinB, PinInDirection.GetPin()));
				Schema->BreakSinglePinLink(PinB, PinAInDirection);
			}
		}
	}

	{
		// Optional PinC
		TArray<UEdGraphPin*> LinkedToPinOpposite = PinOpposite.GetPin()->LinkedTo;
		if (LinkedToPinOpposite.Num() > 0)
		{
			LinkedToPinOpposite.StableSort(TopMostPinSort);
			UEdGraphPin* PinC = PinOpposite.GetPin()->LinkedTo[0];
			// UE_LOG(LogBlueprintAssist, Warning, TEXT("PinC %s (%s)"), *FBAUtils::GetPinName(PinC), *FBAUtils::GetNodeName(PinC->GetOwningNode()));
			PendingConnections.Add(FPinLink(PinC, PinA.GetPin()));
			Schema->BreakSinglePinLink(PinC, PinOpposite.GetPin());
		}
	}

	if (PendingConnections.Num() == 0)
	{
		Transaction->Cancel();
	}

	Schema->BreakSinglePinLink(PinInDirection.GetPin(), PinA.GetPin());

	for (FPinLink& Link : PendingConnections)
	{
		Schema->TryCreateConnection(Link.GetFromPin(), Link.GetToPin());
	}

	auto AutoFormatting = UBASettings::GetFormatterSettings(GraphHandler->GetFocusedEdGraph()).GetAutoFormatting();

	if (AutoFormatting != EBAAutoFormatting::Never)
	{
		FEdGraphFormatterParameters FormatterParams;
		if (AutoFormatting == EBAAutoFormatting::FormatSingleConnected)
		{
			FormatterParams.NodesToFormat.Append(SelectedNodes);
			FormatterParams.NodesToFormat.Add(PinInDirection.GetNode());
		}

		GraphHandler->AddPendingFormatNodes(NodeInDirection, Transaction, FormatterParams);
	}
	else
	{
		UEdGraphNode* SelectedNodeToUse = Direction == EGPD_Output ? NodeOpposite : NodeInDirection;

		int32 DeltaX_Selected = NodeA->NodePosX - SelectedNodeToUse->NodePosX;
		int32 DeltaY_Selected = NodeA->NodePosY - SelectedNodeToUse->NodePosY;

		int32 DeltaX_A = SelectedNodeToUse->NodePosX - NodeA->NodePosX;
		int32 DeltaY_A = SelectedNodeToUse->NodePosY - NodeA->NodePosY;

		// Selected nodes: move node and parameters
		for (UEdGraphNode* SelectedNode : SelectedNodes)
		{
			TArray<UEdGraphNode*> NodeAndParams = FBAUtils::GetNodeAndParameters(SelectedNode);
			for (UEdGraphNode* Node : NodeAndParams)
			{
				Node->NodePosX += DeltaX_Selected;
				Node->NodePosY += DeltaY_Selected;
			}
		}

		// NodeA: move node and parameters
		for (UEdGraphNode* Node : FBAUtils::GetNodeAndParameters(NodeA))
		{
			Node->NodePosX += DeltaX_A;
			Node->NodePosY += DeltaY_A;
		}

		Transaction.Reset();
	}
}

void FBANodeActions::DeleteAndLink()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	const auto& ShouldDeleteNode = [](UEdGraphNode* Node)
	{
		return Node->CanUserDeleteNode();
	};

	TArray<UEdGraphNode*> NodesToDelete = GraphHandler->GetSelectedNodes().Array().FilterByPredicate(ShouldDeleteNode);
	if (NodesToDelete.Num() > 0)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "DeleteAndLink", "Delete and link"));

		DisconnectExecutionOfNodes(NodesToDelete);
		for (int i = NodesToDelete.Num() - 1; i >= 0; --i)
		{
			FBAUtils::SafeDelete(GraphHandler, NodesToDelete[i]);
		}
	}
}

bool FBANodeActions::CanToggleNodes()
{
	return HasMultipleNodesSelected() && GetGraphHandler()->GetBlueprint() != nullptr;
}

// TODO: figure out a nice way to make this work for non-bp graphs as well
void FBANodeActions::ToggleNodes()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();

	auto OnlyPureNodes = [](UEdGraphNode* Node)
	{
		return !FBAUtils::IsKnotNode(Node) && !FBAUtils::IsCommentNode(Node) && FBAUtils::IsNodeImpure(Node);
	};

	TArray<UEdGraphNode*> FilteredNodes = SelectedNodes.Array().FilterByPredicate(OnlyPureNodes);

	if (FilteredNodes.Num() == 0)
	{
		return;
	}

	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ToggleNodes", "Toggle Nodes"));

	bool bAllNodesDisabled = true;
	for (UEdGraphNode* Node : FilteredNodes)
	{
		if (Node->GetDesiredEnabledState() != ENodeEnabledState::Disabled)
		{
			bAllNodesDisabled = false;
			break;
		}
	}

	for (UEdGraphNode* Node : FilteredNodes)
	{
		if (bAllNodesDisabled) // Set nodes to their default state
		{
			ENodeEnabledState DefaultEnabledState = ENodeEnabledState::Enabled;

			if (UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(Node))
			{
				if (UFunction* Function = CallFunctionNode->GetTargetFunction())
				{
					if (Function->HasMetaData(FBlueprintMetadata::MD_DevelopmentOnly))
					{
						DefaultEnabledState = ENodeEnabledState::DevelopmentOnly;
					}
				}
			}

			Node->Modify();
			Node->SetEnabledState(DefaultEnabledState);
		}
		else // Set all nodes to disabled
		{
			if (Node->GetDesiredEnabledState() != ENodeEnabledState::Disabled)
			{
				Node->Modify();
				Node->SetEnabledState(ENodeEnabledState::Disabled);
			}
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GraphHandler->GetBlueprint());
}

void FBANodeActions::RefreshNodeSizes()
{
	TSharedPtr<FBAGraphHandler> GraphHandler = GetGraphHandler();
	if (!GraphHandler)
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes(true);

	auto Graph = GraphHandler->GetFocusedEdGraph();

	auto AutoFormatting = UBASettings::GetFormatterSettings(Graph).GetAutoFormatting();

	if (SelectedNodes.Num() > 0)
	{
		TSharedPtr<FScopedTransaction> Transaction = MakeShareable(new FScopedTransaction(NSLOCTEXT("UnrealEd", "RefreshNodeSize", "Refresh Node Size")));

		FEdGraphFormatterParameters FormatterParams;

		if (AutoFormatting == EBAAutoFormatting::FormatSingleConnected)
		{
			TSet<UEdGraphNode*> NodeSet;
			for (UEdGraphNode* Node : SelectedNodes)
			{
				if (FBAUtils::IsGraphNode(Node))
				{
					NodeSet.Add(Node);
					if (UEdGraphNode* Linked = FBAUtils::GetFirstLinkedNodePreferringInput(Node))
					{
						NodeSet.Add(Linked);
					}
				}
			}

			FormatterParams.NodesToFormat = NodeSet.Array();
		}

		for (UEdGraphNode* Node : SelectedNodes)
		{
			GraphHandler->RefreshNodeSize(Node);

			if (AutoFormatting != EBAAutoFormatting::Never)
			{
				GraphHandler->AddPendingFormatNodes(Node, Transaction, FormatterParams);
			}
			else
			{
				Transaction->Cancel();
			}
		}
	}
}
