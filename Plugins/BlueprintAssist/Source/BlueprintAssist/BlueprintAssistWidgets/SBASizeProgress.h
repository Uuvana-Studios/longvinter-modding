// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FBAGraphHandler;

class BLUEPRINTASSIST_API SBASizeProgress : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SBASizeProgress) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FBAGraphHandler> InOwnerGraphHandler);

public:
	void RenderGraphToBrush();

	void ShowOverlay();

	void HideOverlay();

	bool bIsVisible = false;

protected:
	void DrawWidgetToRenderTarget(TSharedPtr<SWidget> Widget);

	FText GetCacheProgressText() const;

	TOptional<float> GetCachingPercent() const;

	TSharedPtr<FBAGraphHandler> OwnerGraphHandler;

	TSharedPtr<SOverlay> ProgressCenterPanel;

	FSlateBrush GraphSnapshotBrush;

	const FSlateBrush* CachedBorderBrush = nullptr;

	bool bIsCachingOverlayVisible = false;
};
