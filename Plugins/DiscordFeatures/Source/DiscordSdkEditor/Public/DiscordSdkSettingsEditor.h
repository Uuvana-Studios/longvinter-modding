// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DiscordSdkSettingsEditor.generated.h"

UCLASS(config=DiscordSdkEditor)
class DISCORDSDKEDITOR_API UDiscordEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UDiscordEditorSettings() : Super()
	{
		LoadConfig();
	}

	static void SaveBotToken(const FString& NewBotToken)
	{
		UDiscordEditorSettings* Settings = NewObject<UDiscordEditorSettings>();
		Settings->BotToken = NewBotToken;
		Settings->SaveConfig();
	}

	static FString GetBotToken()
	{
		return NewObject<UDiscordEditorSettings>()->BotToken;
	}

private:
	UPROPERTY(config)
	FString BotToken;
};
