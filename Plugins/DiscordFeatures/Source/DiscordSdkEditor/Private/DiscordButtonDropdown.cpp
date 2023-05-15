// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordButtonDropdown.h"
#include "DiscordSdkEditorStyle.h"
#include "DiscordSdkWindow.h"
#include "Interfaces/IMainFrameModule.h"
#include "DiscordSdkEditor.h"
#include "DiscordSdkEditorStyle.h"

#define DropdownMenu(InText)													\
	SVerticalBox::Slot()														\
		.Padding(2.f, 2.f, 2.f, 0.f)											\
		.AutoHeight()															\
	[																			\
		SNew(STextBlock)														\
			.Text(FText::FromString(InText))									\
			.ColorAndOpacity(RGBToSRGB(130, 130, 130))							\
			.Font(TitleFont)													\
	]

#define DropdownAction(InDisplayText, InOnClicked)								\
	SVerticalBox::Slot()														\
		.AutoHeight()															\
	[																			\
		SNew(SButton)															\
			.ContentPadding(FMargin{ 10.f, 5.f })								\
			.ButtonStyle(&DropdownActionStyle)									\
			.OnClicked(FOnClicked::CreateRaw(this, InOnClicked))				\
			.Clipping(EWidgetClipping::ClipToBoundsWithoutIntersecting)			\
		[																		\
			SNew(STextBlock)													\
				.Text(FText::FromString(InDisplayText))							\
				.ColorAndOpacity(FColor::White)									\
		]																		\
	]


SDiscordButtonDropdown::SDiscordButtonDropdown()
	: SCompoundWidget()
{
	DropdownActionStyle
		.SetNormal (FSlateColorBrush(FLinearColor(0.f, 0.f, 0.f, 0.f)))
		.SetHovered(*FDiscordSdkEditorStyle::Get().GetBrush(TEXT("Blurple")))
		.SetPressed(FSlateColorBrush(FLinearColor(RGBToSRGB(68, 82, 130))));
}

void SDiscordButtonDropdown::Construct(const FArguments& InArgs)
{
	const FSlateFontInfo TitleFont(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 8);

	ChildSlot
	[
		SNew(SVerticalBox)

		+ DropdownMenu(TEXT("Discord Actions"))

		+ DropdownAction(TEXT("Configure the Discord SDK"),   &SDiscordButtonDropdown::OnConfigureSdkActionClicked)
		
#if DISCORD_WITH_ACHIEVEMENT
		+ DropdownAction(TEXT("Discord Achievements"), &SDiscordButtonDropdown::OnManageAchievementsClicked)
#endif // DISCORD_WITH_ACHIEVEMENT

	];
}

FReply SDiscordButtonDropdown::OnConfigureSdkActionClicked()
{
	const TSharedRef<SWindow> Window = SConfigureSdkWindow::CreateWindow();

	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

	if (MainFrameModule.GetParentWindow().IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(Window);
	}

	return FReply::Handled();
}

FReply SDiscordButtonDropdown::OnManageAchievementsClicked()
{
	FModuleManager::GetModuleChecked<FDiscordSdkEditorModule>(TEXT("DiscordSdkEditor"))
		.GetOnDiscordAchievementAction().ExecuteIfBound();
	return FReply::Handled();
}
