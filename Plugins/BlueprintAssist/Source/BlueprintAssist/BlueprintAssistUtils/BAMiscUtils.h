// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistTabHandler.h"
#include "BlueprintAssist/BlueprintAssistObjects/BAAssetEditorHandlerObject.h"

class BA_SUBOBJECT_EDITOR_TREE_NODE;

struct BLUEPRINTASSIST_API FBAMiscUtils
{
	static AActor* GetSCSNodeDefaultActor(TSharedPtr<BA_SUBOBJECT_EDITOR_TREE_NODE> Node, UBlueprint* Blueprint);

	static bool IsSCSActorNode(TSharedPtr<BA_SUBOBJECT_EDITOR_TREE_NODE> Node);
	
	template<class AssetClass, class EditorClass>
	static EditorClass* GetEditorFromActiveTabCasted()
	{
		if (UBAAssetEditorHandlerObject* EditorHandler = UBAAssetEditorHandlerObject::Get())
		{
			return EditorHandler->GetEditorFromTabCasted<AssetClass, EditorClass>(FBATabHandler::Get().GetLastMajorTab());
		}

		return nullptr;
	}

	template<class AssetClass>
	static AssetClass* GetAssetForActiveTab()
	{
		if (UBAAssetEditorHandlerObject* EditorHandler = UBAAssetEditorHandlerObject::Get())
		{
			return EditorHandler->GetAssetFromTab<AssetClass>(FBATabHandler::Get().GetLastMajorTab());
		}

		return nullptr;
	}

	static TArray<FString> ParseStringIntoArray(const FString& String, bool bToLower = true);
	static FString ParseSearchTextByFilter(const FString& SearchText, const FString& FilterString);
};
