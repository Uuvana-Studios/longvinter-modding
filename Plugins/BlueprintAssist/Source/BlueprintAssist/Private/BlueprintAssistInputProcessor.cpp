// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistInputProcessor.h"

#include "AssetViewUtils.h"
#include "BlueprintAssistCache.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistModule.h"
#include "BlueprintAssistTabHandler.h"
#include "BlueprintAssistToolbar.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "K2Node_DynamicCast.h"
#include "SGraphPanel.h"
#include "BlueprintAssist/BlueprintAssistObjects/BARootObject.h"
#include "ContentBrowser/Private/SContentBrowser.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"

static TSharedPtr<FBAInputProcessor> BAInputProcessorInstance;

void FBAInputProcessor::Create()
{
	BAInputProcessorInstance = MakeShareable(new FBAInputProcessor());
	FSlateApplication::Get().RegisterInputPreProcessor(BAInputProcessorInstance);
}

FBAInputProcessor& FBAInputProcessor::Get()
{
	return *BAInputProcessorInstance;
}

FBAInputProcessor::FBAInputProcessor()
{
	GlobalActions.Init();
	TabActions.Init();
	ToolkitActions.Init();
	GraphActions.Init();
	NodeActions.Init();
	PinActions.Init();
	BlueprintActions.Init();

#if WITH_SLATE_DEBUGGING
	FSlateDebugging::InputEvent.AddRaw(this, &FBAInputProcessor::HandleSlateInputEvent);
#endif

	CommandLists = {
		GlobalActions.GlobalCommands,
		TabActions.TabCommands,
		TabActions.ActionMenuCommands,
		ToolkitActions.ToolkitCommands,
		GraphActions.GraphCommands,
		GraphActions.GraphReadOnlyCommands,
		NodeActions.SingleNodeCommands,
		NodeActions.MultipleNodeCommands,
		NodeActions.MultipleNodeCommandsIncludingComments,
		PinActions.PinCommands,
		PinActions.PinEditCommands,
		BlueprintActions.BlueprintCommands
	};
}

FBAInputProcessor::~FBAInputProcessor() {}

void FBAInputProcessor::Cleanup()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(BAInputProcessorInstance);
	}

	BAInputProcessorInstance.Reset();
}

void FBAInputProcessor::Tick(
	const float DeltaTime,
	FSlateApplication& SlateApp,
	TSharedRef<ICursor> Cursor)
{
	bIsDisabled = FBAUtils::IsGamePlayingAndHasFocus();

	if (IsDisabled())
	{
		return;
	}

	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();

	FBATabHandler::Get().Tick(DeltaTime);

	if (UBARootObject* RootObject = FBlueprintAssistModule::Get().GetRootObject())
	{
		RootObject->Tick();
	}

	UpdateGroupMovement();
}

bool FBAInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	// ignore repeat keys
	if (InKeyEvent.IsRepeat())
	{
		return false;
	}

	if (OnKeyOrMouseDown(SlateApp, InKeyEvent.GetKey()))
	{
		return true;
	}

	// TODO: Perhaps implement a NavigationConfig, so users can't change focus on widgets
	// See FSlateApplication::SetNavigationConfig

	if (IsDisabled())
	{
		return false;
	}

	if (ProcessFolderBookmarkInput())
	{
		return true;
	}

	if (ProcessContentBrowserInput())
	{
		return true;
	}

	if (SlateApp.IsInitialized())
	{
		TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();

		// process toolbar commands
		if (FBAToolbar::Get().BlueprintAssistToolbarActions->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			SlateApp.GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		if (GlobalActions.GlobalCommands->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			FSlateApplication::Get().GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		if (BlueprintActions.HasOpenBlueprintEditor())
		{
			if (BlueprintActions.BlueprintCommands->ProcessCommandBindings(InKeyEvent.GetKey(), FSlateApplication::Get().GetModifierKeys(), InKeyEvent.IsRepeat()))
			{
				return true;
			}
		}

		// try process toolkit hotkeys
		if (ToolkitActions.ToolkitCommands->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			FSlateApplication::Get().GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		if (!GraphHandler.IsValid())
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Invalid graph handler"));
			return false;
		}

		// cancel graph handler ongoing processes
		if (InKeyEvent.GetKey() == EKeys::Escape)
		{
			GraphHandler->CancelSizeTimeoutNotification();
			GraphHandler->CancelCachingNotification();
			GraphHandler->CancelFormattingNodes();
			GraphHandler->ResetTransactions();
		}

		TSharedPtr<SDockTab> Tab = GraphHandler->GetTab();
		if (!Tab.IsValid() || !Tab->IsForeground())
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Tab invalid or not foreground"));
			return false;
		}

		TSharedPtr<SWidget> KeyboardFocusedWidget = SlateApp.GetKeyboardFocusedWidget();
		// if (KeyboardFocusedWidget.IsValid())
		// {
		// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("%s | %s"), *KeyboardFocusedWidget->GetTypeAsString(), *KeyboardFocusedWidget->ToString());
		// }
		// else
		// {
		// 	UE_LOG(LogBlueprintAssist, Warning, TEXT("No keyboard focused widget!"));
		// }

		// try process graph action menu hotkeys
		TSharedPtr<SWindow> Menu = SlateApp.GetActiveTopLevelWindow();
		if (Menu.IsValid())
		{
			if (GraphActions.HasOpenActionMenu())
			{
				if (TabActions.ActionMenuCommands->ProcessCommandBindings(
					InKeyEvent.GetKey(),
					FSlateApplication::Get().GetModifierKeys(),
					InKeyEvent.IsRepeat()))
				{
					return true;
				}
			}
		}

		// get the keyboard focused widget
		if (!Menu.IsValid() || !KeyboardFocusedWidget.IsValid())
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Focus graph panel"));

			TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel();
			SlateApp.SetKeyboardFocus(GraphPanel);
			KeyboardFocusedWidget = GraphPanel;
		}

		// process commands for when you are editing a user input widget
		if (FBAUtils::IsUserInputWidget(KeyboardFocusedWidget))
		{
			if (FBAUtils::GetParentWidgetOfType(KeyboardFocusedWidget, "SGraphPin").IsValid())
			{
				if (PinActions.PinEditCommands->ProcessCommandBindings(
					InKeyEvent.GetKey(),
					SlateApp.GetModifierKeys(),
					InKeyEvent.IsRepeat()))
				{
					return true;
				}
			}

			if (InKeyEvent.GetKey() == EKeys::Escape)
			{
				SlateApp.SetKeyboardFocus(GraphHandler->GetGraphPanel());
			}

			return false;
		}

		// process commands for when the tab is open
		if (TabActions.TabCommands->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			SlateApp.GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		//UE_LOG(LogBlueprintAssist, Warning, TEXT("Process tab commands"));

		if (!GraphHandler->IsWindowActive())
		{
			//TSharedPtr<SWindow> CurrentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
			//const FString CurrentWindowStr = CurrentWindow.IsValid()
			//	? CurrentWindow->GetTitle().ToString()
			//	: "nullptr";

			//TSharedPtr<SWindow> GHWindow = GraphHandler->GetOrFindWindow();
			//FString GHWindowStr = GHWindow.IsValid() ? GHWindow->GetTitle().ToString() : "Nullptr";
			//UE_LOG(
			//	LogBlueprintAssist,
			//	Warning,
			//	TEXT("Graph Handler window is not active %s current window | GH Window %s"),
			//	*CurrentWindowStr,
			//	*GHWindowStr);
			return false;
		}

		// process commands for when the graph exists but is read only
		if (GraphActions.GraphReadOnlyCommands->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			SlateApp.GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		// skip all other graph commands if read only
		if (GraphHandler->IsGraphReadOnly())
		{
			return false;
		}

		// process general graph commands
		if (GraphActions.GraphCommands->ProcessCommandBindings(
			InKeyEvent.GetKey(),
			SlateApp.GetModifierKeys(),
			InKeyEvent.IsRepeat()))
		{
			return true;
		}

		// process commands for which require a node to be selected
		if (GraphHandler->GetSelectedPin() != nullptr)
		{
			if (PinActions.PinCommands->ProcessCommandBindings(
				InKeyEvent.GetKey(),
				SlateApp.GetModifierKeys(),
				InKeyEvent.IsRepeat()))
			{
				return true;
			}
		}

		// process commands for which require a single node to be selected
		if (GraphHandler->GetSelectedNode() != nullptr)
		{
			//UE_LOG(LogBlueprintAssist, Warning, TEXT("Process node commands"));
			if (NodeActions.SingleNodeCommands->ProcessCommandBindings(
				InKeyEvent.GetKey(),
				SlateApp.GetModifierKeys(),
				InKeyEvent.IsRepeat()))
			{
				return true;
			}

			if (InKeyEvent.GetKey() == EKeys::Escape)
			{
				GraphHandler->GetGraphPanel()->SelectionManager.ClearSelectionSet();
			}
		}

		// process commands for which require multiple nodes to be selected
		if (GraphHandler->GetSelectedNodes().Num() > 0)
		{
			if (NodeActions.MultipleNodeCommands->ProcessCommandBindings(
				InKeyEvent.GetKey(),
				SlateApp.GetModifierKeys(),
				InKeyEvent.IsRepeat()))
			{
				return true;
			}

			if (InKeyEvent.GetKey() == EKeys::Escape)
			{
				GraphHandler->GetGraphPanel()->SelectionManager.ClearSelectionSet();
			}
		}

		// process commands for which require multiple nodes (incl comments) to be selected
		if (GraphHandler->GetSelectedNodes(true).Num() > 0)
		{
			if (NodeActions.MultipleNodeCommandsIncludingComments->ProcessCommandBindings(
				InKeyEvent.GetKey(),
				SlateApp.GetModifierKeys(),
				InKeyEvent.IsRepeat()))
			{
				return true;
			}

			if (InKeyEvent.GetKey() == EKeys::Escape)
			{
				GraphHandler->GetGraphPanel()->SelectionManager.ClearSelectionSet();
			}
		}
	}
	else
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("HandleKeyDown: Slate App not initialized"));
	}
	return false;
}

bool FBAInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (OnKeyOrMouseUp(SlateApp, InKeyEvent.GetKey()))
	{
		return true;
	}

	return false;
}

bool FBAInputProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (OnKeyOrMouseDown(SlateApp, MouseEvent.GetEffectingButton()))
	{
		return true;
	}

	if (IsDisabled())
	{
		return false;
	}

	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();
	if (!GraphHandler)
	{
		return false;
	}

	if (TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel())
	{
		TSharedPtr<SGraphPin> HoveredPin = FBAUtils::GetHoveredGraphPin(GraphPanel);

		// select the hovered pin on LMB or RMB
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton || MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (HoveredPin.IsValid())
			{
				UEdGraphPin* Pin = HoveredPin->GetPinObj();

				GraphHandler->SetSelectedPin(Pin);
			}
		}

		// Fix ongoing transactions being canceled via spawn node event on the graph. See FBlueprintEditor::OnSpawnGraphNodeByShortcut.
		if (GraphHandler->HasActiveTransaction())
		{
			if (GraphPanel->IsHovered())
			{
				return true;
			}
		}
	}

	return false;
}

bool FBAInputProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (OnKeyOrMouseUp(SlateApp, MouseEvent.GetEffectingButton()))
	{
		return true;
	}

	return false;
}

bool FBAInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (IsDisabled())
	{
		return false;
	}

	bool bBlocking = false;
	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();
	if (!GraphHandler.IsValid())
	{
		return false;
	}

	if (TSharedPtr<SGraphPanel> GraphPanel = GraphHandler->GetGraphPanel())
	{
		const FVector2D NewMousePos = FBAUtils::SnapToGrid(FBAUtils::ScreenSpaceToPanelCoord(GraphPanel, MouseEvent.GetScreenSpacePosition()));
		const FVector2D Delta = NewMousePos - LastMousePos;

		bBlocking = OnMouseDrag(SlateApp, NewMousePos, Delta);

		LastMousePos = NewMousePos;
	}

	return bBlocking;
}

void FBAInputProcessor::HandleSlateInputEvent(const FSlateDebuggingInputEventArgs& EventArgs)
{
	if (EventArgs.InputEventType == ESlateDebuggingInputEvent::MouseButtonDoubleClick)
	{
		if (GetDefault<UBASettings>()->bEnableDoubleClickGoToDefinition)
		{
			if (TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler())
			{
				// get the hovered graph node
				TSharedPtr<SGraphNode> GraphNode = FBAUtils::GetHoveredGraphNode(GraphHandler->GetGraphPanel());
				if (!GraphNode)
				{
					return;
				}

				// if we are a dynamic cast, jump to the definition
				if (UK2Node_DynamicCast* DynamicCast = Cast<UK2Node_DynamicCast>(GraphNode->GetNodeObj()))
				{
					TArray<UEdGraphPin*> OutputParameters = FBAUtils::GetParameterPins(DynamicCast, EGPD_Output);
					if (OutputParameters.Num())
					{
						TWeakObjectPtr<UObject> SubcategoryObject = OutputParameters[0]->PinType.PinSubCategoryObject;
						if (SubcategoryObject.IsValid())
						{
							// open using package if it is an asset
							if (SubcategoryObject->IsAsset()) 
							{
								if (UPackage* Outer = Cast<UPackage>(SubcategoryObject->GetOuter()))
								{
									GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Outer->GetName());
								}
							}
							else
							{
								GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(SubcategoryObject.Get());
							}
						}
					}
				}
			}
		}
	}
}

bool FBAInputProcessor::OnMouseDrag(FSlateApplication& SlateApp, const FVector2D& MousePos, const FVector2D& Delta)
{
	TSharedPtr<FBAGraphHandler> MyGraphHandler = FBATabHandler::Get().GetActiveGraphHandler();

	bool bBlocking = false;

	const auto BASettings = GetDefault<UBASettings>();

	// process extra drag nodes
	for (const FInputChord& Chord : BASettings->AdditionalDragNodesChords)
	{
		if (IsInputChordDown(Chord))
		{
			TSet<UEdGraphNode*> NodesToMove = MyGraphHandler->GetSelectedNodes();
			for (UEdGraphNode* Node : NodesToMove)
			{
				Node->Modify();
				Node->NodePosX += Delta.X;
				Node->NodePosY += Delta.Y;
			}

			bBlocking = NodesToMove.Num() > 0;
			break;
		}
	}

	return bBlocking;
}

bool FBAInputProcessor::OnKeyOrMouseDown(FSlateApplication& SlateApp, const FKey& Key)
{
	KeysDown.Add(Key);
	KeysDownStartTime.Add(Key, FSlateApplication::Get().GetCurrentTime());

	if (IsDisabled())
	{
		return false;
	}

	bool bBlocking = false;
	if (TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler())
	{
		if (auto GraphPanel = GraphHandler->GetGraphPanel())
		{
			TSharedPtr<SGraphPin> HoveredPin = FBAUtils::GetHoveredGraphPin(GraphPanel);
			TSharedPtr<SGraphNode> HoveredNode = FBAUtils::GetHoveredGraphNode(GraphPanel);

			const UBASettings* BASettings = GetDefault<UBASettings>();

			// Set the anchor node for group movement
			if (IsAnyInputChordDown(BASettings->GroupMovementChords))
			{
				// if we have a hovered graph pin, this will mess up moving the node
				if (!HoveredPin && HoveredNode)
				{
					AnchorNode = HoveredNode->GetNodeObj();
					LastAnchorPos = HoveredNode->GetPosition();
				}
			}

			// Select the node when pressing additional node drag chord
			if (IsAnyInputChordDown(BASettings->AdditionalDragNodesChords))
			{
				if (HoveredNode)
				{
					const TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes(true);
					UEdGraphNode* HoveredNodeObj = HoveredNode->GetNodeObj();

					// also set the anchor node for group movement
					AnchorNode = HoveredNodeObj;
					LastAnchorPos = HoveredNode->GetPosition();

					if (!SelectedNodes.Contains(HoveredNodeObj))
					{
						GraphHandler->SelectNode(HoveredNodeObj);
						bBlocking = true;
					}
				}
			}
		}
	}

	return bBlocking;
}

bool FBAInputProcessor::OnKeyOrMouseUp(FSlateApplication& SlateApp, const FKey& Key)
{
	bool bBlocking = false;

	// process extra drag nodes
	const auto BASettings = GetDefault<UBASettings>();

	if (IsAnyInputChordDown(BASettings->AdditionalDragNodesChords, Key) && GetKeyDownDuration(Key) > BASettings->ClickTime)
	{
		bBlocking = true;
	}

	if (IsAnyInputChordDown(BASettings->GroupMovementChords, Key))
	{
		AnchorNode = nullptr;
	}

	KeysDown.Remove(Key);
	KeysDownStartTime.Remove(Key);

	return bBlocking;
}

bool FBAInputProcessor::CanExecuteCommand(TSharedRef<const FUICommandInfo> Command) const
{
	for (TSharedPtr<FUICommandList> CommandList : CommandLists)
	{
		if (const FUIAction* Action = CommandList->GetActionForCommand(Command))
		{
			return Action->CanExecute();
		}
	}

	return false;
}

bool FBAInputProcessor::TryExecuteCommand(TSharedRef<const FUICommandInfo> Command)
{
	for (TSharedPtr<FUICommandList> CommandList : CommandLists)
	{
		if (const FUIAction* Action = CommandList->GetActionForCommand(Command))
		{
			if (Action->CanExecute())
			{
				return Action->Execute();
			}
		}
	}

	return false;
}

bool FBAInputProcessor::IsDisabled() const
{
	return bIsDisabled;
}

void FBAInputProcessor::UpdateGroupMovement()
{
	const auto BASettings = GetDefault<UBASettings>();

	if (!IsAnyInputChordDown(BASettings->GroupMovementChords))
	{
		return;
	}

	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();
	if (!GraphHandler || !AnchorNode.IsValid())
	{
		return;
	}

	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	if (!SelectedNodes.Contains(AnchorNode.Get()))
	{
		return;
	}

	const FVector2D NewNodePos(AnchorNode->NodePosX, AnchorNode->NodePosY);
	const FVector2D Delta = NewNodePos - LastAnchorPos;
	LastAnchorPos = NewNodePos;

	if (Delta.SizeSquared() > 0)
	{
		GroupMoveSelectedNodes(Delta);
	}
}

void FBAInputProcessor::GroupMoveSelectedNodes(const FVector2D& Delta)
{
	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();

	TSet<UEdGraphNode*> NodesToMove;

	// grab all linked nodes to move from the selected nodes
	TSet<UEdGraphNode*> SelectedNodes = GraphHandler->GetSelectedNodes();
	for (UEdGraphNode* SelectedNode : SelectedNodes)
	{
		NodesToMove.Append(FBAUtils::GetNodeTree(SelectedNode));
	}

	for (UEdGraphNode* Node : NodesToMove)
	{
		if (!SelectedNodes.Contains(Node))
		{
			Node->Modify();
			Node->NodePosX += Delta.X; 
			Node->NodePosY += Delta.Y;
		}
	}
}

bool FBAInputProcessor::IsInputChordDown(const FInputChord& Chord)
{
	const FModifierKeysState ModKeysState = FSlateApplication::Get().GetModifierKeys();
	const bool AreModifiersDown = ModKeysState.AreModifersDown(EModifierKey::FromBools(Chord.bCtrl, Chord.bAlt, Chord.bShift, Chord.bCmd));
	return KeysDown.Contains(Chord.Key) && AreModifiersDown;
}

bool FBAInputProcessor::IsAnyInputChordDown(const TArray<FInputChord>& Chords)
{
	for (const FInputChord& Chord : Chords)
	{
		if (IsInputChordDown(Chord))
		{
			return true;
		}
	}

	return false;
}

bool FBAInputProcessor::IsInputChordDown(const FInputChord& Chord, const FKey Key)
{
	const FModifierKeysState ModKeysState = FSlateApplication::Get().GetModifierKeys();
	const bool AreModifiersDown = ModKeysState.AreModifersDown(EModifierKey::FromBools(Chord.bCtrl, Chord.bAlt, Chord.bShift, Chord.bCmd));
	return Chord.Key == Key && AreModifiersDown;
}

bool FBAInputProcessor::IsAnyInputChordDown(const TArray<FInputChord>& Chords, const FKey Key)
{
	for (const FInputChord& Chord : Chords)
	{
		if (IsInputChordDown(Chord, Key))
		{
			return true;
		}
	}

	return false;
}

double FBAInputProcessor::GetKeyDownDuration(const FKey Key)
{
	if (const double* FoundTime = KeysDownStartTime.Find(Key))
	{
		return FSlateApplication::Get().GetCurrentTime() - (*FoundTime);
	}

	return -1.0f;
}

bool FBAInputProcessor::ProcessFolderBookmarkInput()
{
	const UBASettings* BASettings = GetDefault<UBASettings>();

	for (int i = 0; i < BASettings->FolderBookmarks.Num(); ++i)
	{
		const FKey& BookmarkKey = BASettings->FolderBookmarks[i];

		if (IsInputChordDown(FInputChord(EModifierKey::Control | EModifierKey::Shift, BookmarkKey)))
		{
			if (FIND_PARENT_WIDGET(FSlateApplication::Get().GetUserFocusedWidget(0), SContentBrowser))
			{
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();

#if BA_UE_VERSION_OR_LATER(5, 0)
				const FString FolderPath = ContentBrowser.GetCurrentPath().GetInternalPathString();
#else
				const FString FolderPath = ContentBrowser.GetCurrentPath();
#endif
				FBACache::Get().SetBookmarkedFolder(FolderPath, i);

				FNotificationInfo Notification(FText::FromString(FString::Printf(TEXT("Saved bookmark %s to %s"), *BookmarkKey.ToString().ToUpper(), *FolderPath)));
				Notification.ExpireDuration = 3.0f;
				FSlateNotificationManager::Get().AddNotification(Notification);
				break;
			}
		}

		if (IsInputChordDown(FInputChord(EModifierKey::Control, BookmarkKey)))
		{
			if (FIND_PARENT_WIDGET(FSlateApplication::Get().GetUserFocusedWidget(0), SContentBrowser))
			{
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();

				if (TOptional<FString> FolderPath = FBACache::Get().FindBookmarkedFolder(i))
				{
					if (!FolderPath.GetValue().IsEmpty())
					{
						ContentBrowser.SetSelectedPaths({ FolderPath.GetValue() });
					}
				}
				break;
			}
		}
	}

	return false;
}


// TODO move these into FBACommands
bool FBAInputProcessor::ProcessContentBrowserInput()
{
	if (TSharedPtr<SContentBrowser> ContentBrowserWidget = FIND_PARENT_WIDGET(FSlateApplication::Get().GetUserFocusedWidget(0), SContentBrowser))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		IContentBrowserSingleton& ContentBrowser = ContentBrowserModule.Get();

		// copy
		if (IsInputChordDown(FInputChord(EModifierKey::Control, EKeys::C)))
		{
			CutAssets.Reset();
			return false;
		}

		// cut
		if (IsInputChordDown(FInputChord(EModifierKey::Control, EKeys::X)))
		{
			TArray<FAssetData> SelectedAssets;
			ContentBrowser.GetSelectedAssets(SelectedAssets);

			CutAssets.Reset();
			for (FAssetData& SelectedAsset : SelectedAssets)
			{
				CutAssets.Add(SelectedAsset);
			}

			return CutAssets.Num() > 0;
		}

		// paste
		if (IsInputChordDown(FInputChord(EModifierKey::Control, EKeys::V)))
		{
			if (CutAssets.Num())
			{
#if BA_UE_VERSION_OR_LATER(5, 0)
				const FContentBrowserItemPath BrowserPath = ContentBrowser.GetCurrentPath();
				const FString Path = BrowserPath.HasInternalPath() ? ContentBrowser.GetCurrentPath().GetInternalPathString() : FString();
#else
				const FString Path = ContentBrowser.GetCurrentPath();
#endif

				TArray<UObject*> AssetsToMove;
				for (const FAssetData& AssetData : CutAssets)
				{
					const bool bSameFolder = Path.Equals(AssetData.PackagePath.ToString());
					if (!bSameFolder)
					{
						if (UObject* Asset = AssetData.GetAsset())
						{
							AssetsToMove.Add(Asset);
						}
					}
				}

				if (!AssetsToMove.Num())
				{
					return false;
				}

				// TODO why do transactions not work when moving assets? (there's no undo when moving with drag / drop)
				// const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "CutPaste_BlueprintAssist", "Cut And Paste"));
				// for (UObject* ToMove : AssetsToMove)
				// {
				// 	ToMove->Modify();
				// }

				AssetViewUtils::MoveAssets(AssetsToMove, Path);

				CutAssets.Reset();
				return true;
			}
		}
	}

	return false;
}
