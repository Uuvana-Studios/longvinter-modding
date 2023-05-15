// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

FORCEINLINE FLinearColor RGBToSRGB(const int32 R, const int32 G, const int32 B)
{
	const FLinearColor Linear = { R / 256.f, G / 256.f, B / 256.f, 0.f };
	return { FMath::Pow(Linear.R, 2.2f), FMath::Pow(Linear.G, 2.2f), FMath::Pow(Linear.B, 2.2f), 1.f };
}

class DISCORDSDKEDITOR_API FDiscordSdkEditorStyle
{
public:

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();

	static struct FButtonStyle* GetDiscordButtonStyle();

private:

	static TSharedRef< class FSlateStyleSet > Create();

private:

	static TSharedPtr< class FSlateStyleSet > StyleInstance;

	static TUniquePtr< struct FButtonStyle > DiscordButtonStyle;
};