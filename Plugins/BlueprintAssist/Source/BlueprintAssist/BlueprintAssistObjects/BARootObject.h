// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "BAAssetEditorHandlerObject.h"
#include "UObject/Object.h"

#include "BARootObject.generated.h"

UCLASS()
class BLUEPRINTASSIST_API UBARootObject : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	void Tick();

	void Cleanup();

	UBAAssetEditorHandlerObject* GetAssetHandler() const { return AssetHandler; }

private:
	UPROPERTY()
	UBAAssetEditorHandlerObject* AssetHandler = nullptr;
};
