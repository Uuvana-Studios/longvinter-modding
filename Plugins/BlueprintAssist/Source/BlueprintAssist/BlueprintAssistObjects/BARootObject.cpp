// Fill out your copyright notice in the Description page of Project Settings.

#include "BARootObject.h"

#include "BAAssetEditorHandlerObject.h"

void UBARootObject::Init()
{
	AssetHandler = NewObject<UBAAssetEditorHandlerObject>();
	AssetHandler->Init();
}

void UBARootObject::Tick()
{
	AssetHandler->Tick();
}

void UBARootObject::Cleanup()
{
	AssetHandler->Cleanup();
}
