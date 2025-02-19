// Fill out your copyright notice in the Description page of Project Settings.

#include "SBASizeProgress.h"

#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistStyle.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Notifications/SProgressBar.h"

void SBASizeProgress::Construct(const FArguments& InArgs, TSharedPtr<FBAGraphHandler> InOwnerGraphHandler)
{
	OwnerGraphHandler = InOwnerGraphHandler;

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&GraphSnapshotBrush)
		]
		+ SOverlay::Slot()
		.VAlign(VAlign_Center).HAlign(HAlign_Center)
		[
			SAssignNew(ProgressCenterPanel, SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(FBAStyle::GetBrush("BlueprintAssist.PlainBorder"))
				.ColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.1f, 1.0f))
			]
			+ SOverlay::Slot()
			.Padding(1.0f)
			[
				SNew(SBorder)
				.BorderImage(FBAStyle::GetBrush("BlueprintAssist.PanelBorder"))
				.Padding(8.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
						SNew(STextBlock)
						.Text_Raw(this, &SBASizeProgress::GetCacheProgressText)
						.TextStyle(BA_STYLE_CLASS::Get(), TEXT("DetailsView.CategoryTextStyle"))
					]
					+ SVerticalBox::Slot()
					[
						SNew(SSpacer).Size(FVector2D(0, 16.0f))
					]
					+ SVerticalBox::Slot()
					[
						SNew(SBox).WidthOverride(256.0f)
						[
							SNew(SProgressBar)
							.BorderPadding(FVector2D::ZeroVector)
							.FillColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 1.0f)))
							.Percent(this, &SBASizeProgress::GetCachingPercent)
						]
					]
				]
			]
		]
	];

	SetVisibility(EVisibility::Collapsed);
}

void SBASizeProgress::RenderGraphToBrush()
{
	TSharedPtr<SGraphEditor> GraphEditor = OwnerGraphHandler->GetGraphEditor();
	DrawWidgetToRenderTarget(GraphEditor);
}

void SBASizeProgress::ShowOverlay()
{
	if (bIsVisible)
	{
		return;
	}

	bIsVisible = true;
	RenderGraphToBrush();
	SetVisibility(EVisibility::HitTestInvisible);

	if (OwnerGraphHandler->GetNumberOfPendingNodesToCache() > GetDefault<UBASettings>()->RequiredNodesToShowOverlayProgressBar)
	{
		ProgressCenterPanel->SetVisibility(EVisibility::Visible);
	}
	else
	{
		ProgressCenterPanel->SetVisibility(EVisibility::Hidden);
	}
}

void SBASizeProgress::HideOverlay()
{
	bIsVisible = false;
	SetVisibility(EVisibility::Collapsed);
}

void SBASizeProgress::DrawWidgetToRenderTarget(TSharedPtr<SWidget> Widget)
{
	if (!Widget.IsValid())
	{
		return;
	}

	const FIntPoint RenderSize = Widget->GetTickSpaceGeometry().GetLocalSize().IntPoint();

	FWidgetRenderer* WidgetRenderer = new FWidgetRenderer(false, true);
	if (!WidgetRenderer)
	{
		return;
	}

	UTextureRenderTarget2D* RenderTarget = WidgetRenderer->DrawWidget(Widget.ToSharedRef(), RenderSize);
	if (!RenderTarget)
	{
		return;
	}

	FlushRenderingCommands();

	BeginCleanup(WidgetRenderer);

	GraphSnapshotBrush.SetResourceObject(RenderTarget);
	GraphSnapshotBrush.SetImageSize(FVector2D(RenderTarget->SizeX, RenderTarget->SizeY));
}

FText SBASizeProgress::GetCacheProgressText() const
{
	return FText::FromString(FString::Printf(TEXT("Caching Node Sizes (%d)"), OwnerGraphHandler->GetNumberOfPendingNodesToCache()));
}

TOptional<float> SBASizeProgress::GetCachingPercent() const
{
	return OwnerGraphHandler->GetPendingNodeSizeProgress();
}
