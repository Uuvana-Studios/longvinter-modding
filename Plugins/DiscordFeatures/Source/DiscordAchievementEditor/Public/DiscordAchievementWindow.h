// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Slate.h"
#include "Styling/SlateBrush.h"
#include "Brushes/SlateColorBrush.h"
#include "Widgets/SCompoundWidget.h"

enum class EDiscordAchievementStatus : uint8
{
    Loading,
    Error,
    Over
};

struct FAchievementInfo
{
    FString Name;
    FString Description;
    bool    bSecret;
    FString Id;
    bool    bSecure;
};

DECLARE_DELEGATE_TwoParams(FOnGetAchievementsResponse, const TArray<FAchievementInfo>&, const bool);
DECLARE_DELEGATE_OneParam(FOnAchievementCreated, const bool);

class FAchievementManager
{
public:
    FAchievementManager();

    void LoadAchievements(const FOnGetAchievementsResponse& OnResponse);

    void CreateAchievement(const FAchievementInfo& Achievement, const FString& ImageData, const FOnAchievementCreated& OnCreated);

private:
    static FAchievementInfo CreateAchievementFromJsonObject(const TSharedPtr<class FJsonObject>& Object);
};

class SDiscordCreateAchievement : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDiscordCreateAchievement)
        : _Window(nullptr)
        , _AchievementWindow(nullptr)
    {}
    SLATE_ATTRIBUTE(TSharedPtr<class SWindow>, Window)
    SLATE_ATTRIBUTE(TSharedPtr<class SDiscordAchievementWindow>, AchievementWindow)
    SLATE_END_ARGS();

    SDiscordCreateAchievement(){};
    ~SDiscordCreateAchievement() {};

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);

private:
    FReply OnBrowse();
    FReply OnCreate();

private:
    TSharedPtr<class SEditableTextBox> AchievementName;
    TSharedPtr<class SEditableTextBox> AchievementDescription;
    TSharedPtr<class SCheckBox>        AchievementSecret;
    TSharedPtr<class SCheckBox>        AchievementSecure;
    TSharedPtr<class SEditableTextBox> AchievementImage;

    TSharedPtr<class SWindow> ParentWindow;
    TSharedPtr<class SDiscordAchievementWindow> AchievementWindow;
    
};

class DISCORDACHIEVEMENTEDITOR_API SDiscordAchievementWindow : public SCompoundWidget
{
    
public:
    SLATE_BEGIN_ARGS(SDiscordAchievementWindow)
    {}
    
    SLATE_END_ARGS();

    SDiscordAchievementWindow();
    ~SDiscordAchievementWindow();

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
        
private:

    TSharedRef<class STextBlock> MakeText(const FString Text, const uint16 Size = 16);

    TSharedRef<class SWidget> MakeAchievementList();

    TSharedRef<class SWidget> MakeAchievementRow();

    void LoadAchievements();
    void OnAchievementsReceived(const TArray<FAchievementInfo>& Achievements, const bool bSuccess);

    void BuildAchievements();

public:
    FReply OnButtonRefreshClicked();
    FReply OnButtonCreateClicked();

public:
    static FSlateFontInfo GetDefaultFont(const uint16 Size);
    static const FLinearColor FontColor;
    static const FLinearColor OddRowColor;
    static const FLinearColor EvenRowColor;
    static const FSlateColorBrush BlackStyle;
    static const FSlateColorBrush BackgroundStyle;
    static const FSlateColorBrush AchievementHeadRowStyle;
    static const FSlateColorBrush AchievementOddRowStyle;
    static const FSlateColorBrush AchievementEvenRowStyle;

private:
    TSharedPtr<class SVerticalBox> AchievementsContainer;

    TUniquePtr<FAchievementManager> AchievementManager;

private:
    EDiscordAchievementStatus Status;
    TArray<FAchievementInfo> AchievementsList;
};

