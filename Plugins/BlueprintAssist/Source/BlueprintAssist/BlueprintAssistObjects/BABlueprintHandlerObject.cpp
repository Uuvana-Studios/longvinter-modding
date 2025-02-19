// Fill out your copyright notice in the Description page of Project Settings.

#include "BABlueprintHandlerObject.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistSettings.h"
#include "BlueprintAssistUtils.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Knot.h"
#include "K2Node_Tunnel.h"
#include "ScopedTransaction.h"
#include "SGraphActionMenu.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Logging/MessageLog.h"

#if BA_UE_VERSION_OR_LATER(5, 0)
	#define BA_GET_ON_OBJECTS_REPLACED FCoreUObjectDelegates::OnObjectsReplaced
#else
	#define BA_GET_ON_OBJECTS_REPLACED GEditor->OnObjectsReplaced()
#endif

UBABlueprintHandlerObject::~UBABlueprintHandlerObject()
{
	if (BlueprintPtr.IsValid())
	{
		BlueprintPtr->OnChanged().RemoveAll(this);
		BlueprintPtr->OnCompiled().RemoveAll(this);
	}

	if (GEditor)
	{
		BA_GET_ON_OBJECTS_REPLACED.RemoveAll(this);
	}
}

void UBABlueprintHandlerObject::BindBlueprintChanged(UBlueprint* Blueprint)
{
	if (!Blueprint->IsValidLowLevelFast(false))
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("BAObject: Tried to bind to invalid blueprint"));
		return;
	}

	BlueprintPtr = TWeakObjectPtr<UBlueprint>(Blueprint);
	SetLastVariables(Blueprint);
	SetLastFunctionGraphs(Blueprint);
	bProcessedChangesThisFrame = false;
	bActive = true;

	Blueprint->OnChanged().RemoveAll(this);
	Blueprint->OnChanged().AddUObject(this, &UBABlueprintHandlerObject::OnBlueprintChanged);

	Blueprint->OnCompiled().RemoveAll(this);
	Blueprint->OnCompiled().AddUObject(this, &UBABlueprintHandlerObject::OnBlueprintCompiled);

	if (GEditor)
	{
		BA_GET_ON_OBJECTS_REPLACED.RemoveAll(this);
		BA_GET_ON_OBJECTS_REPLACED.AddUObject(this, &UBABlueprintHandlerObject::OnObjectsReplaced);
	}
}

void UBABlueprintHandlerObject::UnbindBlueprintChanged(UBlueprint* Blueprint)
{
	LastVariables.Empty();
	bProcessedChangesThisFrame = false;
	bActive = false;

	if (BlueprintPtr.IsValid() && BlueprintPtr->IsValidLowLevelFast())
	{
		BlueprintPtr->OnChanged().RemoveAll(this);
		BlueprintPtr->OnCompiled().RemoveAll(this);
	}

	Blueprint->OnChanged().RemoveAll(this);
	Blueprint->OnCompiled().RemoveAll(this);
}

void UBABlueprintHandlerObject::SetLastVariables(UBlueprint* Blueprint)
{
	if (!Blueprint->IsValidLowLevelFast(false))
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("BAObject: Tried to update variables from an invalid blueprint"));
		return;
	}

	LastVariables = Blueprint->NewVariables;
}

void UBABlueprintHandlerObject::SetLastFunctionGraphs(UBlueprint* Blueprint)
{
	if (!Blueprint->IsValidLowLevelFast(false))
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("BAObject: Tried to update function graphs from an invalid blueprint"));
		return;
	}

	const int NumFunctionGraphs = Blueprint->FunctionGraphs.Num();
	LastFunctionGraphs.Empty(NumFunctionGraphs);
	for (UEdGraph* Graph : Blueprint->FunctionGraphs)
	{
		if (Graph && Graph->IsValidLowLevelFast(false))
		{
			TWeakObjectPtr<UEdGraph> WeakGraph(Graph);
			LastFunctionGraphs.Add(WeakGraph);
		}
		else
		{
			UE_LOG(LogBlueprintAssist, Warning, TEXT("BAObject: Found invalid graph from Blueprint function graphs"));
		}
	}
}

// See UControlRigBlueprint::OnPostVariableChange
void UBABlueprintHandlerObject::OnBlueprintChanged(UBlueprint* Blueprint)
{
	// Blueprint should always be valid?
	if (!Blueprint->IsValidLowLevelFast(false))
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("BAObject: Invalid blueprint was changed, please report this on github"));
		return;
	}

	if (!BlueprintPtr.IsValid())
	{
		BlueprintPtr = Blueprint;
	}

	if (Blueprint != BlueprintPtr)
	{
		const FString OldBlueprintName = BlueprintPtr.IsValid() ? BlueprintPtr->GetName() : FString("nullptr");
		UE_LOG(LogBlueprintAssist, Warning, TEXT("BAObject: Blueprint was changed but it's the wrong blueprint? %s %s"), *Blueprint->GetName(), *OldBlueprintName);
		return;
	}

	if (!bActive)
	{
		return;
	}

	if (bProcessedChangesThisFrame)
	{
		return;
	}

	bProcessedChangesThisFrame = true;
	GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UBABlueprintHandlerObject::ResetProcessedChangesThisFrame));

	// This shouldn't happen!
	check(Blueprint->IsValidLowLevelFast(false));

	TMap<FGuid, int32> OldVariablesByGuid;
	for (int32 VarIndex = 0; VarIndex < LastVariables.Num(); VarIndex++)
	{
		OldVariablesByGuid.Add(LastVariables[VarIndex].VarGuid, VarIndex);
	}

	for (FBPVariableDescription& NewVariable : Blueprint->NewVariables)
	{
		if (!OldVariablesByGuid.Contains(NewVariable.VarGuid))
		{
			OnVariableAdded(Blueprint, NewVariable);
			continue;
		}

		const int32 OldVarIndex = OldVariablesByGuid.FindChecked(NewVariable.VarGuid);
		const FBPVariableDescription& OldVariable = LastVariables[OldVarIndex];

		// Make set instance editable to true when you set expose on spawn to true
		if (FBAUtils::HasMetaDataChanged(OldVariable, NewVariable, FBlueprintMetadata::MD_ExposeOnSpawn))
		{
			const bool bNotInstanceEditable = (NewVariable.PropertyFlags & CPF_DisableEditOnInstance) == CPF_DisableEditOnInstance;
			if (bNotInstanceEditable && NewVariable.HasMetaData(FBlueprintMetadata::MD_ExposeOnSpawn) && NewVariable.GetMetaData(FBlueprintMetadata::MD_ExposeOnSpawn) == TEXT("true"))
			{
				FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, NewVariable.VarName, false);
			}
		}

		// Check if a variable has been renamed (use string cause names are not case-sensitive!)
		if (!OldVariable.VarName.ToString().Equals(NewVariable.VarName.ToString()))
		{
			OnVariableRenamed(Blueprint, OldVariable, NewVariable);
		}

		// Check if a variable type has changed
		if (OldVariable.VarType != NewVariable.VarType)
		{
			OnVariableTypeChanged(Blueprint, OldVariable, NewVariable);
		}
	}

	SetLastVariables(Blueprint);

	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		// This means we created a new function?
		if (!LastFunctionGraphs.Contains(FunctionGraph))
		{
			OnFunctionAdded(Blueprint, FunctionGraph);
		}
	}

	SetLastFunctionGraphs(Blueprint);
}

void UBABlueprintHandlerObject::ResetProcessedChangesThisFrame()
{
	bProcessedChangesThisFrame = false;
}

void UBABlueprintHandlerObject::OnObjectsReplaced(const TMap<UObject*, UObject*>& ReplacementMap)
{
	if (BlueprintPtr.IsValid())
	{
		if (UObject* Replacement = ReplacementMap.FindRef(BlueprintPtr.Get()))
		{
			UE_LOG(LogBlueprintAssist, Warning, TEXT("BAObject: Blueprint was replaced with %s"), *Replacement->GetName());
			UnbindBlueprintChanged(BlueprintPtr.Get());

			if (UBlueprint* NewBlueprint = Cast<UBlueprint>(Replacement))
			{
				BindBlueprintChanged(NewBlueprint);
			}
			else
			{
				BlueprintPtr = nullptr;
			}
		}
	}
}

void UBABlueprintHandlerObject::OnVariableAdded(UBlueprint* Blueprint, FBPVariableDescription& Variable)
{
	const UBASettings* BASettings = GetDefault<UBASettings>();
	if (BASettings->bEnableVariableDefaults)
	{
		if (BASettings->bDefaultVariableInstanceEditable)
		{
			FBlueprintEditorUtils::SetBlueprintOnlyEditableFlag(Blueprint, Variable.VarName, false);
		}

		if (BASettings->bDefaultVariableBlueprintReadOnly)
		{
			FBlueprintEditorUtils::SetBlueprintPropertyReadOnlyFlag(Blueprint, Variable.VarName, true);
		}

		if (BASettings->bDefaultVariableExposeOnSpawn)
		{
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_ExposeOnSpawn, TEXT("true"));
		}

		if (BASettings->bDefaultVariablePrivate)
		{
			FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_Private, TEXT("true"));
		}

		if (BASettings->bDefaultVariableExposeToCinematics)
		{
			FBlueprintEditorUtils::SetInterpFlag(Blueprint, Variable.VarName, true);
		}

		FBlueprintEditorUtils::SetBlueprintVariableCategory(Blueprint, Variable.VarName, nullptr, BASettings->DefaultVariableCategory);

		FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint, Variable.VarName, nullptr, FBlueprintMetadata::MD_Tooltip, BASettings->DefaultVariableTooltip.ToString());
	}
}

void UBABlueprintHandlerObject::OnVariableRenamed(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable)
{
	if (GetDefault<UBASettings>()->bAutoRenameGettersAndSetters)
	{
		RenameGettersAndSetters(Blueprint, OldVariable, NewVariable);
	}
}

void UBABlueprintHandlerObject::OnVariableTypeChanged(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable)
{
	// Boolean variables may need to be renamed as well!
	if (GetDefault<UBASettings>()->bAutoRenameGettersAndSetters)
	{
		RenameGettersAndSetters(Blueprint, OldVariable, NewVariable);
	}
}

void UBABlueprintHandlerObject::RenameGettersAndSetters(UBlueprint* Blueprint, const FBPVariableDescription& OldVariable, FBPVariableDescription& NewVariable)
{
	const FString OldVariableName = FBAUtils::GetVariableName(OldVariable.VarName.ToString(), OldVariable.VarType.PinCategory, OldVariable.VarType.ContainerType);
	const FString NewVariableName = FBAUtils::GetVariableName(NewVariable.VarName.ToString(), NewVariable.VarType.PinCategory, NewVariable.VarType.ContainerType);

	// Do nothing if our names didn't change
	if (OldVariableName == NewVariableName)
	{
		return;
	}

	const FString GetterName = FString::Printf(TEXT("Get%s"), *OldVariableName);
	const FString SetterName = FString::Printf(TEXT("Set%s"), *OldVariableName);

	const FString NewGetterName = FString::Printf(TEXT("Get%s"), *NewVariableName);
	const FString NewSetterName = FString::Printf(TEXT("Set%s"), *NewVariableName);

	for (UEdGraph* VariableGraph : Blueprint->FunctionGraphs)
	{
		if (VariableGraph->GetName() == GetterName)
		{
			FBlueprintEditorUtils::RenameGraph(VariableGraph, NewGetterName);
		}
		else if (VariableGraph->GetName() == SetterName)
		{
			FBlueprintEditorUtils::RenameGraph(VariableGraph, NewSetterName);
		}
	}
}

void UBABlueprintHandlerObject::OnFunctionAdded(UBlueprint* Blueprint, UEdGraph* FunctionGraph)
{
	const UBASettings* BASettings = GetDefault<UBASettings>();
	if (!BASettings->bEnableFunctionDefaults)
	{
		return;
	}

	TArray<UK2Node_FunctionEntry*> EntryNodes;
	FunctionGraph->GetNodesOfClass(EntryNodes);

	if (EntryNodes.Num() <= 0 || !IsValid(EntryNodes[0]) || !EntryNodes[0]->IsEditable())
	{
		return;
	}

	UK2Node_EditablePinBase* FunctionEntryNode = EntryNodes[0];

	UFunction* Function = FindFunctionFromEntryNode(FunctionEntryNode, FunctionGraph);
	if (!Function)
	{
		return;
	}


	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "EditFunctionDefaults", "Edit Function Defaults"));

	Function->Modify();
	FunctionEntryNode->Modify();

	const bool bInsideInterface = Function->GetOuterUClass()->IsChildOf(UInterface::StaticClass());
	
	EFunctionFlags AccessSpecifier = FUNC_Public;
	if (!bInsideInterface)
	{
		switch (BASettings->DefaultFunctionAccessSpecifier)
		{
			case EBAFunctionAccessSpecifier::Public:
				AccessSpecifier = FUNC_Public;
			break;
			case EBAFunctionAccessSpecifier::Protected:
				AccessSpecifier = FUNC_Protected;
			break;
			case EBAFunctionAccessSpecifier::Private:
				AccessSpecifier = FUNC_Private;
			break;
		}
	}

	const EFunctionFlags ClearAccessSpecifierMask = ~FUNC_AccessSpecifiers;
	if (UK2Node_FunctionEntry* EntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNode))
	{
		// Set const
		if (BASettings->bDefaultFunctionConst)
		{
			EntryNode->SetExtraFlags(EntryNode->GetExtraFlags() ^ FUNC_Const);
		}

		// Set exec
		if (BASettings->bDefaultFunctionExec)
		{
			EntryNode->SetExtraFlags(EntryNode->GetExtraFlags() ^ FUNC_Exec);
		}

		// Set pure
		if (!bInsideInterface && BASettings->bDefaultFunctionPure)
		{
			EntryNode->SetExtraFlags(EntryNode->GetExtraFlags() ^ FUNC_BlueprintPure);
		}

		int32 ExtraFlags = EntryNode->GetExtraFlags();
		ExtraFlags &= ClearAccessSpecifierMask;
		ExtraFlags |= AccessSpecifier;
		EntryNode->SetExtraFlags(ExtraFlags);
	}
	else if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(FunctionEntryNode))
	{
		EventNode->FunctionFlags &= ClearAccessSpecifierMask;
		EventNode->FunctionFlags |= AccessSpecifier;
	}

	Function->FunctionFlags &= ClearAccessSpecifierMask;
	Function->FunctionFlags |= AccessSpecifier;

	if (FKismetUserDeclaredFunctionMetadata* Metadata = GetMetadataBlock(FunctionEntryNode))
	{
		// Set default keywords
		const FText& DefaultKeywords = BASettings->DefaultFunctionKeywords;
		// Remove excess whitespace and prevent keywords with just spaces
		const FText& Keywords = FText::TrimPrecedingAndTrailing(DefaultKeywords);
		if (!Keywords.EqualTo(Metadata->Keywords))
		{
			Metadata->Keywords = Keywords;
			Function->SetMetaData(FBlueprintMetadata::MD_FunctionKeywords, *Keywords.ToString());
		}

		// Set default tooltip
		const FText& DefaultDescription = BASettings->DefaultFunctionTooltip;
		Metadata->ToolTip = DefaultDescription;
		Function->SetMetaData(FBlueprintMetadata::MD_Tooltip, *DefaultDescription.ToString());

		// Set default category
		const FText& DefaultFunctionCategory = BASettings->DefaultFunctionCategory;
		Metadata->Category = DefaultFunctionCategory;
		if (Function)
		{
			check(!Function->IsNative()); // Should never get here with a native function, as we wouldn't have been able to find metadata for it
			Function->Modify();
			Function->SetMetaData(FBlueprintMetadata::MD_FunctionCategory, *DefaultFunctionCategory.ToString());
		}
		
		// Refresh category in editor? See FBlueprintGraphActionDetails::OnCategoryTextCommitted | SMyBlueprint::Refresh
		FBlueprintEditorUtils::ConformImplementedInterfaces(Blueprint);
		if (TSharedPtr<SGraphActionMenu> GraphActionMenu = FBAUtils::GetGraphActionMenu())
		{
			GraphActionMenu->RefreshAllActions(true);
		}
	}

	// Refresh the node after editing properties (from FBaseBlueprintGraphActionDetails::OnParamsChanged)
	{
		const bool bCurDisableOrphanSaving = FunctionEntryNode->bDisableOrphanPinSaving;
		FunctionEntryNode->bDisableOrphanPinSaving = true;

		FunctionEntryNode->ReconstructNode();

		FunctionEntryNode->bDisableOrphanPinSaving = bCurDisableOrphanSaving;
	}
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	K2Schema->HandleParameterDefaultValueChanged(FunctionEntryNode);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
}

UFunction* UBABlueprintHandlerObject::FindFunctionFromEntryNode(UK2Node_EditablePinBase* FunctionEntry, UEdGraph* Graph)
{
	if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(FunctionEntry))
	{
		return FFunctionFromNodeHelper::FunctionFromNode(EventNode);
	}
	else if (Graph)
	{
		if (UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
		{
			UClass* Class = Blueprint->SkeletonGeneratedClass;

			for (TFieldIterator<UFunction> FunctionIt(Class, EFieldIteratorFlags::IncludeSuper); FunctionIt; ++FunctionIt)
			{
				UFunction* Function = *FunctionIt;
				if (Function->GetName() == Graph->GetName())
				{
					return Function;
				}
			}
		}
	}
	return nullptr;
}

FKismetUserDeclaredFunctionMetadata* UBABlueprintHandlerObject::GetMetadataBlock(UK2Node_EditablePinBase* FunctionEntryNode) const
{
	if (UK2Node_FunctionEntry* TypedEntryNode = Cast<UK2Node_FunctionEntry>(FunctionEntryNode))
	{
		return &(TypedEntryNode->MetaData);
	}
	else if (UK2Node_Tunnel* TunnelNode = ExactCast<UK2Node_Tunnel>(FunctionEntryNode))
	{
		// Must be exactly a tunnel, not a macro instance
		return &(TunnelNode->MetaData);
	}
	else if (UK2Node_CustomEvent* EventNode = Cast<UK2Node_CustomEvent>(FunctionEntryNode))
	{
		return &(EventNode->GetUserDefinedMetaData());
	}

	return nullptr;
}

void UBABlueprintHandlerObject::OnBlueprintCompiled(UBlueprint* Blueprint)
{
	if (!IsValid(Blueprint))
	{
		return;
	}

	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);

	for (UEdGraph* Graph : Graphs)
	{
		DetectGraphIssues(Graph);
	}
}

void UBABlueprintHandlerObject::DetectGraphIssues(UEdGraph* Graph)
{
	if (!IsValid(Graph))
	{
		return;
	}

	struct FLocal
	{
		static void FocusNode(TWeakObjectPtr<UK2Node_Knot> Node)
		{
			if (Node.IsValid())
			{
				FKismetEditorUtilities::BringKismetToFocusAttentionOnObject(Node.Get(), false);
			}
		}
	};

	FMessageLog BlueprintAssistLog("BlueprintAssist");

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		// Detect bad knot nodes
		if (UK2Node_Knot* KnotNode = Cast<UK2Node_Knot>(Node))
		{
			// Detect empty knot nodes to be deleted
			if (FBAUtils::GetLinkedPins(KnotNode).Num() == 0)
			{
				TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Info);
				const FText MessageText = FText::FromString(FString::Printf(TEXT("Unlinked reroute node %s"), *KnotNode->NodeGuid.ToString()));
				Message->AddToken(FTextToken::Create(MessageText));
				Message->AddToken(FActionToken::Create(
					FText::FromString("GoTo"),
					FText::FromString("Go to node"),
					FOnActionTokenExecuted::CreateStatic(&FLocal::FocusNode, TWeakObjectPtr<UK2Node_Knot>(KnotNode))));

				BlueprintAssistLog.AddMessage(Message);
			}
			else
			{
				bool bOpenMessageLog = false;

				// Detect badly linked exec knot nodes
				for (UEdGraphPin* Pin : FBAUtils::GetLinkedPins(KnotNode, EGPD_Output).FilterByPredicate(FBAUtils::IsExecPin))
				{
					if (Pin->LinkedTo.Num() > 1)
					{
						TSharedRef<FTokenizedMessage> Message = FTokenizedMessage::Create(EMessageSeverity::Error);
						const FText MessageText = FText::FromString(FString::Printf(TEXT("Badly linked reroute node (manually delete and remake this node) %s"), *KnotNode->NodeGuid.ToString()));
						Message->AddToken(FTextToken::Create(MessageText));
						Message->AddToken(FActionToken::Create(
							FText::FromString("GoTo"),
							FText::FromString("Go to node"),
							FOnActionTokenExecuted::CreateStatic(&FLocal::FocusNode, TWeakObjectPtr<UK2Node_Knot>(KnotNode))));

						BlueprintAssistLog.AddMessage(Message);

						bOpenMessageLog = true;
					}
				}

				if (bOpenMessageLog)
				{
					BlueprintAssistLog.Open();
				}
			}
		}
	}
}
