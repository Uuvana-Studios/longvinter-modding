// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistTypes.h"

class BLUEPRINTASSIST_API SBADebugMenuRow final : public SHorizontalBox
{
	SLATE_BEGIN_ARGS(SBADebugMenuRow)
		: _Label()
		, _Value()
	{
	}

	SLATE_ATTRIBUTE(FText, Label)
	SLATE_ATTRIBUTE(FText, Value)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
};

class BLUEPRINTASSIST_API SBADebugMenu final : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBADebugMenu) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	FText PinUnderCursor;
	FText NodeUnderCursor;
	FText GraphUnderCursor;
	FText HoveredWidget;
	FText FocusedWidget;
	FText CurrentTab;
	FText FocusedAssetEditor;
	FText CurrentAsset;

	static void RegisterNomadTab();
};
