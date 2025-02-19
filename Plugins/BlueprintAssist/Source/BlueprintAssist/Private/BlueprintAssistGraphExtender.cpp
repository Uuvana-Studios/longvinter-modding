// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistGraphExtender.h"

#include "BlueprintAssistGraphCommands.h"
#include "BlueprintAssistInputProcessor.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistUtils.h"
#include "BlueprintEditor.h"
#include "GraphEditorModule.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "ScopedTransaction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Widgets/Notifications/SNotificationList.h"

void FBAGraphExtender::ApplyExtender()
{
	FGraphEditorModule& GraphEditorModule = FModuleManager::GetModuleChecked<FGraphEditorModule>(TEXT("GraphEditor"));
	GraphEditorModule.GetAllGraphEditorContextMenuExtender().Add(FGraphEditorModule::FGraphEditorMenuExtender_SelectedNode::CreateStatic(&FBAGraphExtender::ExtendSelectedNode));
}

TSharedRef<FExtender> FBAGraphExtender::ExtendSelectedNode(const TSharedRef<FUICommandList> CommandList, const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bIsEditable)
{
	if (Pin)
	{
		return ExtendPin(CommandList, Graph, Node, Pin, bIsEditable);
	}

	TSharedRef<FExtender> Extender(new FExtender());

	struct FLocal
	{
		static void CallGenerateGetter(const UEdGraph* Graph, const UEdGraphNode* Node)
		{
			GenerateGetter(Graph, Node);
		}

		static void CallGenerateSetter(const UEdGraph* Graph, const UEdGraphNode* Node)
		{
			GenerateSetter(Graph, Node);
		}

		static void AddGenerateGetterSetter(FMenuBuilder& MenuBuilder)
		{
			if (GetDefault<UBASettings>()->bMergeGenerateGetterAndSetterButton)
			{
				MenuBuilder.AddMenuEntry(FBAGraphCommands::Get().GenerateGetterAndSetter);
			}
			else
			{
				MenuBuilder.AddMenuEntry(FBAGraphCommands::Get().GenerateGetter);
				MenuBuilder.AddMenuEntry(FBAGraphCommands::Get().GenerateSetter);
			}
		}

		static void AddConvertGetToSet(FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(FBAGraphCommands::Get().ConvertGetToSet);
		}

		static void AddConvertSetToGet(FMenuBuilder& MenuBuilder)
		{
			MenuBuilder.AddMenuEntry(FBAGraphCommands::Get().ConvertSetToGet);
		}
	};

	UEdGraphNode* MutNode = FBAUtils::GetNodeFromGraph(Graph, Node->NodeGuid);
	if (!MutNode)
	{
		return Extender;
	}

	UK2Node_VariableGet* VariableGetNode = Cast<UK2Node_VariableGet>(MutNode);
	UK2Node_VariableSet* VariableSetNode = Cast<UK2Node_VariableSet>(MutNode);

	if (VariableGetNode || VariableSetNode)
	{
		CommandList->MapAction(
			FBAGraphCommands::Get().GenerateGetter,
			FExecuteAction::CreateStatic(&FLocal::CallGenerateGetter, Graph, Node));

		CommandList->MapAction(
			FBAGraphCommands::Get().GenerateSetter,
			FExecuteAction::CreateStatic(&FLocal::CallGenerateSetter, Graph, Node));

		CommandList->MapAction(
			FBAGraphCommands::Get().GenerateGetterAndSetter,
			FExecuteAction::CreateStatic(&FBAGraphExtender::GenerateGetterAndSetter, Graph, Node));

		if (VariableGetNode)
		{
			CommandList->MapAction(
				FBAGraphCommands::Get().ConvertGetToSet,
				FExecuteAction::CreateStatic(&FBAGraphExtender::ConvertGetToSet, Graph, VariableGetNode));
		}

		if (VariableSetNode)
		{
			CommandList->MapAction(
				FBAGraphCommands::Get().ConvertSetToGet,
				FExecuteAction::CreateStatic(&FBAGraphExtender::ConvertSetToGet, Graph, VariableSetNode));
		}
	}

	if (Node->IsA(UK2Node_VariableGet::StaticClass()))
	{
		Extender->AddMenuExtension(
			"EdGraphSchemaNodeActions",
			EExtensionHook::After,
			CommandList,
			FMenuExtensionDelegate::CreateStatic(&FLocal::AddGenerateGetterSetter));

		Extender->AddMenuExtension(
			"EdGraphSchemaNodeActions",
			EExtensionHook::After,
			CommandList,
			FMenuExtensionDelegate::CreateStatic(&FLocal::AddConvertGetToSet));
	}

	if (Node->IsA(UK2Node_VariableSet::StaticClass()))
	{
		Extender->AddMenuExtension(
			"EdGraphSchemaNodeActions",
			EExtensionHook::After,
			CommandList,
			FMenuExtensionDelegate::CreateStatic(&FLocal::AddConvertSetToGet));
	}

	return Extender;
}

TSharedRef<FExtender> FBAGraphExtender::ExtendPin(const TSharedRef<FUICommandList> CommandList, const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bIsEditable)
{
	struct FLocal
	{
		static void AddGoToDefinition(FMenuBuilder& MenuBuilder, const UEdGraphPin* InPin)
		{
			FString ClassName = "Unknown";
			TWeakObjectPtr<UObject> SubcategoryObject = InPin->PinType.PinSubCategoryObject;
			if (SubcategoryObject.IsValid())
			{
				ClassName = SubcategoryObject->GetName();
			}

			MenuBuilder.AddMenuEntry(
				FText::FromString(FString::Printf(TEXT("Go To Definition (%s)"), *ClassName)),
				FText::FromString(FString::Printf(TEXT("Navigate to the asset or cpp class (%s)"), *ClassName)),
				FSlateIcon(),
				FExecuteAction::CreateStatic(&FBAGraphExtender::GoToDefinition, InPin)
			);
		}
	};

	TSharedRef<FExtender> Extender(new FExtender());

	TWeakObjectPtr<UObject> SubCategoryObject = Pin->PinType.PinSubCategoryObject;
	if (SubCategoryObject.IsValid())
	{
		Extender->AddMenuExtension(
			"EdGraphSchemaPinActions",
			EExtensionHook::After,
			CommandList,
			FMenuExtensionDelegate::CreateStatic(&FLocal::AddGoToDefinition, Pin));
	}

	return Extender;
}

bool FBAGraphExtender::GenerateGetter(const UEdGraph* Graph, const UEdGraphNode* Node)
{
	const UK2Node_VariableGet* SourceVariableGet = Cast<UK2Node_VariableGet>(Node);
	check(SourceVariableGet);

	const FBlueprintEditor* BPEditor = FBAUtils::GetBlueprintEditorForGraph(Graph);
	if (!BPEditor)
	{
		return false;
	}

	UBlueprint* BlueprintObj = BPEditor->GetBlueprintObj();

	const FEdGraphPinType& PinType = SourceVariableGet->GetPinAt(0)->PinType;
	const FString VariableName = FBAUtils::GetVariableName(SourceVariableGet->VariableReference.GetMemberName().ToString(), PinType.PinCategory, PinType.ContainerType);

	const FString FunctionName = FString::Printf(TEXT("Get%s"), *VariableName);

	// Do nothing if function already exists
	if (FindObject<UEdGraph>(BlueprintObj, *FunctionName))
	{
		const FText Message = FText::FromString(FString::Printf(TEXT("Getter '%s' already exists"), *FunctionName));
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 2.0f;
		Info.bUseSuccessFailIcons = true;
		Info.Image = BA_STYLE_CLASS::Get().GetBrush(TEXT("Icons.Warning"));
		FSlateNotificationManager::Get().AddNotification(Info);
		return false;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GenerateGetter_BlueprintAssist", "Generate Getter"));
	BlueprintObj->Modify();

	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(BlueprintObj, FName(*FunctionName), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UClass>(BlueprintObj, NewGraph, true, nullptr);

	UK2Node_EditablePinBase* FunctionEntryNodePtr = FBlueprintEditorUtils::GetEntryNode(NewGraph);
	UK2Node_FunctionResult* NewResultNode = FBlueprintEditorUtils::FindOrCreateFunctionResultNode(FunctionEntryNodePtr);
	NewResultNode->NodePosX = 256;
	NewResultNode->NodePosY = 0;

	UEdGraphPin* Pin = NewResultNode->CreateUserDefinedPin("ReturnValue", SourceVariableGet->GetPinAt(0)->PinType, EGPD_Input);

	const UEdGraphSchema_K2* Schema = Cast<UEdGraphSchema_K2>(NewGraph->GetSchema());
	check(Schema != nullptr);

	// Create variable get
	FVector2D SpawnPos(NewResultNode->NodePosX, 128);

	UK2Node_VariableGet* NewVarGet = CreateVariableGetFromVariable(SpawnPos, NewGraph, SourceVariableGet);

	// Link to output
	FBAUtils::TryCreateConnection(Pin, NewVarGet->GetPinAt(0));

	// Set pure
	UFunction* Function = BlueprintObj->SkeletonGeneratedClass->FindFunctionByName(NewGraph->GetFName());
	Function->FunctionFlags ^= FUNC_BlueprintPure;

	UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNodePtr);
	EntryNode->MetaData.Category = GetDefault<UBASettings>()->DefaultGeneratedGettersCategory;
	EntryNode->SetExtraFlags(EntryNode->GetExtraFlags() ^ FUNC_BlueprintPure);

	{
		const bool bCurDisableOrphanSaving = NewResultNode->bDisableOrphanPinSaving;
		NewResultNode->bDisableOrphanPinSaving = true;
		NewResultNode->ReconstructNode();
		NewResultNode->bDisableOrphanPinSaving = bCurDisableOrphanSaving;
	}

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	K2Schema->HandleParameterDefaultValueChanged(NewResultNode);

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintObj);

	return true;
}

bool FBAGraphExtender::GenerateSetter(const UEdGraph* Graph, const UEdGraphNode* Node)
{
	const UK2Node_VariableGet* SourceVariableGet = Cast<UK2Node_VariableGet>(Node);
	check(SourceVariableGet);

	const FBlueprintEditor* BPEditor = FBAUtils::GetBlueprintEditorForGraph(Graph);
	if (!BPEditor)
	{
		return false;
	}

	UBlueprint* BlueprintObj = BPEditor->GetBlueprintObj();

	const FEdGraphPinType& PinType = SourceVariableGet->GetPinAt(0)->PinType;
	const FString VariableName = FBAUtils::GetVariableName(SourceVariableGet->VariableReference.GetMemberName().ToString(), PinType.PinCategory, PinType.ContainerType);

	const FString FunctionName = FString::Printf(TEXT("Set%s"), *VariableName);

	// Do nothing if function already exists
	if (FindObject<UEdGraph>(BlueprintObj, *FunctionName))
	{
		const FText Message = FText::FromString(FString::Printf(TEXT("Setter '%s' already exists"), *FunctionName));
		FNotificationInfo Info(Message);
		Info.ExpireDuration = 2.0f;
		Info.bUseSuccessFailIcons = true;
		Info.Image = BA_STYLE_CLASS::Get().GetBrush(TEXT("Icons.Warning"));
		FSlateNotificationManager::Get().AddNotification(Info);
		return false;
	}

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GenerateSetter_BlueprintAssist", "Generate Setter"));
	BlueprintObj->Modify();

	UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(BlueprintObj, FName(*FunctionName), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
	FBlueprintEditorUtils::AddFunctionGraph<UClass>(BlueprintObj, NewGraph, true, nullptr);

	UK2Node_EditablePinBase* FunctionEntryNodePtr = FBlueprintEditorUtils::GetEntryNode(NewGraph);

	UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNodePtr);
	EntryNode->MetaData.Category = GetDefault<UBASettings>()->DefaultGeneratedSettersCategory;

	const UEdGraphSchema_K2* Schema = Cast<UEdGraphSchema_K2>(NewGraph->GetSchema());
	check(Schema != nullptr);

	// Create set variable node
	const FVector2D SpawnPos(256, 16);

	UK2Node_VariableSet* SetNode = CreateVariableSetFromVariable(SpawnPos, NewGraph, SourceVariableGet);

	// Create input pin getter
	UEdGraphPin* NewInputPin = FunctionEntryNodePtr->CreateUserDefinedPin("NewValue", SourceVariableGet->GetPinAt(0)->PinType, EGPD_Output);

	// Link nodes
	FBAUtils::TryCreateConnection(FunctionEntryNodePtr->Pins[0], FBAUtils::GetExecPins(SetNode, EGPD_Input)[0]);
	FBAUtils::TryCreateConnection(FBAUtils::GetParameterPins(SetNode, EGPD_Input)[0], NewInputPin);

	{
		const bool bCurDisableOrphanSaving = FunctionEntryNodePtr->bDisableOrphanPinSaving;
		FunctionEntryNodePtr->bDisableOrphanPinSaving = true;
		FunctionEntryNodePtr->ReconstructNode();
		FunctionEntryNodePtr->bDisableOrphanPinSaving = bCurDisableOrphanSaving;
	}

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	K2Schema->HandleParameterDefaultValueChanged(FunctionEntryNodePtr);

	FBlueprintEditorUtils::MarkBlueprintAsModified(BlueprintObj);

	return true;
}

void FBAGraphExtender::GenerateGetterAndSetter(const UEdGraph* Graph, const UEdGraphNode* Node)
{
	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "GenerateGetterAndSetter_BlueprintAssist", "Generate Getter And Setter"));

	bool bSuccess = false;
	bSuccess |= GenerateGetter(Graph, Node);
	bSuccess |= GenerateSetter(Graph, Node);

	if (!bSuccess)
	{
		Transaction.Cancel();
	}
}

void FBAGraphExtender::ConvertGetToSet(const UEdGraph* Graph, UK2Node_VariableGet* VariableGetNode)
{
	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ConvertGetToSet_BlueprintAssist", "Convert Get To Set"));

	const FBlueprintEditor* BPEditor = FBAUtils::GetBlueprintEditorForGraph(Graph);
	if (!BPEditor)
	{
		return;
	}

	UEdGraph* MutGraph = VariableGetNode->GetGraph();

	const UEdGraphSchema_K2* Schema = Cast<UEdGraphSchema_K2>(Graph->GetSchema());
	if (!Schema)
	{
		return;
	}

	// Create the set node
	auto NodePos = FVector2D(VariableGetNode->NodePosX, VariableGetNode->NodePosY);

	UK2Node_VariableSet* SetNode = CreateVariableSetFromVariable(NodePos, MutGraph, VariableGetNode);

	UEdGraphPin* OutPin = SetNode->FindPin(TEXT("Output_Get"));

	// Check if the self pin exists
	TArray<UEdGraphPin*> OriginalSelfLinkedTo;
	if (UEdGraphPin* OriginalSelfPin = Schema->FindSelfPin(*VariableGetNode, EGPD_Input))
	{
		OriginalSelfLinkedTo = OriginalSelfPin->LinkedTo;
	}

	TArray<UEdGraphPin*> PinsToLinkTo = VariableGetNode->GetValuePin()->LinkedTo;

	// Delete the get node
	FBAUtils::DeleteNode(VariableGetNode);

	// replace links
	for (UEdGraphPin* LinkedPin : PinsToLinkTo)
	{
		Graph->GetSchema()->TryCreateConnection(OutPin, LinkedPin);
	}

	// Link self pins
	if (UEdGraphPin* NewSelfPin = Schema->FindSelfPin(*SetNode, EGPD_Input))
	{
		for (UEdGraphPin* Pin : OriginalSelfLinkedTo)
		{
			Graph->GetSchema()->TryCreateConnection(NewSelfPin, Pin);
		}
	}
}

void FBAGraphExtender::ConvertSetToGet(const UEdGraph* Graph, UK2Node_VariableSet* VariableSetNode)
{
	FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ConvertSetToGet_BlueprintAssist", "Convert Set To Get"));

	const FBlueprintEditor* BPEditor = FBAUtils::GetBlueprintEditorForGraph(Graph);
	if (!BPEditor)
	{
		return;
	}

	UEdGraph* MutGraph = VariableSetNode->GetGraph();

	const UEdGraphSchema_K2* Schema = Cast<UEdGraphSchema_K2>(MutGraph->GetSchema());
	if (!Schema)
	{
		return;
	}

	// Create the get node
	const FVector2D NodePos = FVector2D(VariableSetNode->NodePosX, VariableSetNode->NodePosY);

	UK2Node_VariableGet* GetNode = CreateVariableGetFromVariable(NodePos, MutGraph, VariableSetNode);

	UEdGraphPin* OutPin = GetNode->GetValuePin();

	TArray<UEdGraphPin*> PinsToLinkTo = VariableSetNode->FindPin(TEXT("Output_Get"))->LinkedTo;

	// Check if the self pin exists
	TArray<UEdGraphPin*> OriginalSelfLinkedTo;
	if (UEdGraphPin* OriginalSelfPin = Schema->FindSelfPin(*VariableSetNode, EGPD_Input))
	{
		OriginalSelfLinkedTo = OriginalSelfPin->LinkedTo;
	}

	// Delete the set node
	TArray<UEdGraphNode*> NodesToDisconnect{ VariableSetNode };
	FBANodeActions::DisconnectExecutionOfNodes(NodesToDisconnect);
	FBAUtils::DeleteNode(VariableSetNode);

	// replace links for the get node
	for (UEdGraphPin* LinkedPin : PinsToLinkTo)
	{
		Graph->GetSchema()->TryCreateConnection(OutPin, LinkedPin);
	}

	// Link self pin
	if (UEdGraphPin* NewSelfPin = Schema->FindSelfPin(*GetNode, EGPD_Input))
	{
		for (UEdGraphPin* Pin : OriginalSelfLinkedTo)
		{
			Graph->GetSchema()->TryCreateConnection(NewSelfPin, Pin);
		}
	}
}

void FBAGraphExtender::GoToDefinition(const UEdGraphPin* Pin)
{
	if (Pin)
	{
		TWeakObjectPtr<UObject> SubcategoryObject = Pin->PinType.PinSubCategoryObject;
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

UK2Node_VariableSet* FBAGraphExtender::CreateVariableSetFromVariable(FVector2D NodePos, UEdGraph* Graph, const UK2Node_Variable* Variable)
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	if (FProperty* VariableProperty = Variable->GetPropertyForVariable())
	{
		if (UStruct* Outer = VariableProperty->GetOwnerChecked<UStruct>())
		{
			return K2Schema->SpawnVariableSetNode(NodePos, Graph, Variable->VariableReference.GetMemberName(), Outer);
		}
	}

	return nullptr;
}

UK2Node_VariableGet* FBAGraphExtender::CreateVariableGetFromVariable(FVector2D NodePos, UEdGraph* Graph, const UK2Node_Variable* Variable)
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	if (FProperty* VariableProperty = Variable->GetPropertyForVariable())
	{
		if (UStruct* Outer = VariableProperty->GetOwnerChecked<UStruct>())
		{
			return K2Schema->SpawnVariableGetNode(NodePos, Graph, Variable->VariableReference.GetMemberName(), Outer);
		}
	}

	return nullptr;
}
