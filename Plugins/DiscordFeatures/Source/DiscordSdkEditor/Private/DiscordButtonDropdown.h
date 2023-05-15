// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Slate.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SCompoundWidget.h"

#if DISCORD_FEATURES_EDITOR_UE_5_0_OR_LATER
using FDFVerticalBoxSlot = SVerticalBox::FSlot::FSlotArguments;
#else
using FDFVerticalBoxSlot = SVerticalBox::FSlot;
#endif

class SDiscordButtonDropdown : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDiscordButtonDropdown)
    {}
    SLATE_END_ARGS();

    SDiscordButtonDropdown();

    /** Constructs this widget with InArgs */
     void Construct(const FArguments& InArgs);

private:
    FReply OnConfigureSdkActionClicked();
    FReply OnManageAchievementsClicked();

private:
    FButtonStyle DropdownActionStyle;

};
