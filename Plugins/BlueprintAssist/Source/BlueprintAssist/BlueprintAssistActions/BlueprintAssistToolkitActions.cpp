// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintAssistToolkitActions.h"

#include "SourceCodeNavigation.h"
#include "BlueprintAssist/BlueprintAssistUtils/BAMiscUtils.h"
#include "Toolkits/ToolkitManager.h"

bool FBAToolkitActionsBase::HasToolkit() const
{
	if (CanExecuteActions())
	{
		if (UObject* Asset = FBAMiscUtils::GetAssetForActiveTab<UObject>())
		{
			if (TSharedPtr<IToolkit> Toolkit = FToolkitManager::Get().FindEditorForAsset(Asset))
			{
				return true;
			}
		}
	}

	return false;
}

void FBAToolkitActions::Init()
{
	ToolkitCommands = MakeShareable(new FUICommandList());

	ToolkitCommands->MapAction(
		FBACommands::Get().GoToParentClassDefinition,
		FExecuteAction::CreateRaw(this, &FBAToolkitActions::GoToParentClassDefinition),
		FCanExecuteAction::CreateRaw(this, &FBAToolkitActions::HasToolkit)
	);
}

void FBAToolkitActions::GoToParentClassDefinition() const
{
	if (UObject* Asset = FBAMiscUtils::GetAssetForActiveTab<UObject>())
	{
		if (TSharedPtr<IToolkit> Toolkit = FToolkitManager::Get().FindEditorForAsset(Asset))
		{
			if (const TArray<UObject*>* EditedObject = Toolkit->GetObjectsCurrentlyBeingEdited())
			{
				const TArray<UObject*>& Objects = *EditedObject;

				if (Objects.Num() == 1 && Objects[0] != nullptr)
				{
					UObject* Object = Objects[0];
					UClass* ClassToNavigateTo = Object->GetClass();

					// from FBlueprintEditor::OnEditParentClassClicked
					if (UBlueprint* Blueprint = Cast<UBlueprint>(Object)) 
					{
						if (UObject* ParentClass = Blueprint->ParentClass)
						{
							if (UBlueprintGeneratedClass* ParentBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(ParentClass))
							{
								GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(ParentBlueprintGeneratedClass->ClassGeneratedBy);
								return;
							}
						}
					}

					if (FSourceCodeNavigation::CanNavigateToClass(ClassToNavigateTo))
					{
						FSourceCodeNavigation::NavigateToClass(ClassToNavigateTo);
					}
				}
			}
		}
	}
}
