// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "UObject/Object.h"
#include "Widgets/Docking/SDockTab.h"
#include "BAAssetEditorHandlerObject.generated.h"

class IAssetEditorInstance;
class UBABlueprintHandlerObject;

UCLASS()
class BLUEPRINTASSIST_API UBAAssetEditorHandlerObject : public UObject
{
	GENERATED_BODY()

public:
	virtual ~UBAAssetEditorHandlerObject() override;

	static UBAAssetEditorHandlerObject* Get();

	void Init();

	void Cleanup();

	void Tick();

	IAssetEditorInstance* GetEditorFromTab(const TSharedPtr<SDockTab> Tab) const;

	template<class AssetClass, class EditorClass>
	EditorClass* GetEditorFromTabCasted(const TSharedPtr<SDockTab> Tab) const
	{
		if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
		{
			if (FoundAsset->IsValid() && FoundAsset->Get()->IsValidLowLevelFast(false) && FoundAsset->Get()->IsA(AssetClass::StaticClass()))
			{
				if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
				{
					return static_cast<EditorClass*>(AssetEditorSubsystem->FindEditorForAsset(FoundAsset->Get(), false));
				}
			}
		}

		return nullptr;
	}

	template<class AssetClass>
	AssetClass* GetAssetFromTab(const TSharedPtr<SDockTab> Tab) const
	{
		if (const TWeakObjectPtr<UObject>* FoundAsset = AssetsByTab.Find(Tab))
		{
			if (FoundAsset->IsValid() && FoundAsset->Get()->IsValidLowLevelFast(false))
			{
				return Cast<AssetClass>(FoundAsset->Get());
			}
		}

		return nullptr;
	}

	TSharedPtr<SDockTab> GetTabForAsset(UObject* Asset) const;

	TSharedPtr<SDockTab> GetTabForAssetEditor(IAssetEditorInstance* AssetEditor) const;

protected:
	void BindAssetOpenedDelegate();

	void UnbindDelegates();

	void OnAssetOpened(UObject* Asset, class IAssetEditorInstance* AssetEditor);

	void OnAssetClosed(UObject* Asset);

	void CheckInvalidAssetEditors();

private:
	TArray<TWeakObjectPtr<UObject>> OpenAssets;

	UPROPERTY()
	TMap<FGuid, UBABlueprintHandlerObject*> BlueprintHandlers;

	TMap<TWeakPtr<SDockTab>, TWeakObjectPtr<UObject>> AssetsByTab;
};
