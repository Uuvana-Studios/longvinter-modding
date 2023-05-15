// Copyright Charmillot Clement 2020. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SCompoundWidget.h"

class SWindow;

class SConfigureSdkWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SConfigureSdkWindow)
    {}
    SLATE_END_ARGS()

    SConfigureSdkWindow();

    /** Constructs this widget with InArgs */
     void Construct(const FArguments& InArgs);

public:
	static TSharedRef<SWindow> CreateWindow();

private:
    UFUNCTION()
    FReply DownloadSdk();

    static void SaveSdk(SConfigureSdkWindow* const Self, const TArray<uint8> Sdk, TSharedPtr<FSlowTask> Task);

    TSharedRef<class SButton> SHyperlinkText(const FString& DisplayText, const FString& Url);

private:
    FLinearColor Blurple;
    FLinearColor Dark;
    FLinearColor LightRed;

    FSlateColorBrush MissingSdkBackgroundColor;
    FSlateColorBrush BlurpleBrush;
    FSlateColorBrush DarkBrush;

    static const FString DiscordSdkLocation;

    TSharedPtr<class SBorder> InvalidSdkNotice;

    FButtonStyle HyperlinkStyle;

    FSlateBrush DiscordBrandingBrush;
};
