// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordSdkEditorStyle.h"
#include "DiscordSdkEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FDiscordSdkEditorStyle::StyleInstance = nullptr;
TUniquePtr< FButtonStyle >	 FDiscordSdkEditorStyle::DiscordButtonStyle = nullptr;

void FDiscordSdkEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FDiscordSdkEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	DiscordButtonStyle.Reset();
	StyleInstance.Reset();
}

FName FDiscordSdkEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DiscordSdkEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... )  FSlateImageBrush ( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... )	  FSlateBoxBrush   ( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... )	  FSlateFontInfo   ( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... )	  FSlateFontInfo   ( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

TSharedRef< FSlateStyleSet > FDiscordSdkEditorStyle::Create()
{
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	TSharedRef< FSlateStyleSet > Style = MakeShared<FSlateStyleSet>(GetStyleSetName());
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DiscordFeatures")->GetBaseDir() / TEXT("Resources"));

	Style->Set(TEXT("DiscordSdkEditor.PluginAction"),		new IMAGE_BRUSH(TEXT("ButtonIcon_40x"),		Icon40x40));
	Style->Set(TEXT("DiscordSdkEditor.DiscordBranding"),	new IMAGE_BRUSH(TEXT("DiscordBranding"),	FVector2D{ 300.f, 102.f }));
	Style->Set(TEXT("DiscordSdkEditor.DiscordBlack"),	    new IMAGE_BRUSH(TEXT("DiscordBlack"),	    FVector2D{ 16.f, 16.f}));

	Style->Set(TEXT("Blurple"), new FSlateColorBrush(RGBToSRGB(114, 137, 218)));


	DiscordButtonStyle = MakeUnique<FButtonStyle>();
	DiscordButtonStyle
		->SetNormal(*Style->GetBrush(TEXT("Blurple")))
		.SetHovered(FSlateColorBrush(RGBToSRGB(91, 110, 176)))
		.SetPressed(FSlateColorBrush(RGBToSRGB(70, 85, 138)));

	return Style;	
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FDiscordSdkEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FDiscordSdkEditorStyle::Get()
{
	return *StyleInstance;
}

FButtonStyle* FDiscordSdkEditorStyle::GetDiscordButtonStyle()
{
	return DiscordButtonStyle.IsValid() ? DiscordButtonStyle.Get() : nullptr;
}

