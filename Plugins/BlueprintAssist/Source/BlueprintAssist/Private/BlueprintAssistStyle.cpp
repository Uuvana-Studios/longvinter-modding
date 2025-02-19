#include "BlueprintAssistStyle.h"

#include "BlueprintAssistGlobals.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

#if ENGINE_MAJOR_VERSION >= 5
#include "Styling/StyleColors.h"
#endif

TSharedPtr<FSlateStyleSet> FBAStyle::StyleSet = nullptr;

#define BA_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleSet->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BA_BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( StyleSet->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BA_BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( StyleSet->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BA_DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)
#define BA_ICON_FONT(...) FSlateFontInfo(StyleSet->RootToContentDir("Fonts/FontAwesome", TEXT(".ttf")), __VA_ARGS__)

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

void FBAStyle::Initialize()
{
	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet("BlueprintAssistStyle"));

	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

#if ENGINE_MAJOR_VERSION >= 5
	StyleSet->Set("BlueprintAssist.WhiteBorder", new FSlateRoundedBoxBrush(FStyleColors::White, 4.0f));
	StyleSet->Set("BlueprintAssist.PanelBorder", new FSlateRoundedBoxBrush(FStyleColors::Panel, 4.0f));
#else
	StyleSet->Set("BlueprintAssist.WhiteBorder", new BA_BOX_BRUSH("Common/RoundedSelection_16x", FMargin(4.0f/16.0f)));
	StyleSet->Set("BlueprintAssist.PanelBorder", new BA_BOX_BRUSH("Common/DarkGroupBorder", FMargin(4.0f/16.0f)));
#endif

	StyleSet->Set("BlueprintAssist.PlainBorder", new BA_BORDER_BRUSH("Common/PlainBorder", 2.f/8.f));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

#undef BA_IMAGE_BRUSH
#undef BA_BOX_BRUSH
#undef BA_BORDER_BRUSH
#undef BA_DEFAULT_FONT
#undef BA_ICON_FONT

void FBAStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}

const ISlateStyle& FBAStyle::Get()
{
	return *(StyleSet.Get());
}

const FName& FBAStyle::GetStyleSetName()
{
	return StyleSet->GetStyleSetName();
}
