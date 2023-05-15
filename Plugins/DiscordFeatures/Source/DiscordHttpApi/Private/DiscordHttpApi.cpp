// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordHttpApi.h"
#include "DiscordWebhookLibrary.h"

#if WITH_EDITOR
#	include "ISettingsModule.h"
#	include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FDiscordWebhookModule"

DECLARE_LOG_CATEGORY_CLASS(LogDiscordHttpApi, Log, All);

UDiscordHttpApiSettings::UDiscordHttpApiSettings()
	: Super()
{
	LoadConfig();
}

void FDiscordHttpApiModule::StartupModule()
{
	AuthenticationType = EDiscordHttpApiAuthenticationType::Bot;
	
	BotToken = GetMutableDefault<UDiscordHttpApiSettings>()->BotToken;

#if WITH_EDITOR
	// register settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "Discord HTTP API",
			LOCTEXT("DiscordHttpApiName", "Discord HTTP API"),
			LOCTEXT("DiscordHttpApiDescription", "Configure the Discord HTTP API settings."),
			GetMutableDefault<UDiscordHttpApiSettings>()
		);
		
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FDiscordHttpApiModule::OnSettingsSaved);
		}
	}
#endif // WITH_EDITOR

}

void FDiscordHttpApiModule::ShutdownModule()
{
}

bool FDiscordHttpApiModule::OnSettingsSaved()
{
	BotToken = GetMutableDefault<UDiscordHttpApiSettings>()->BotToken;
	return true;
}

FDiscordHttpApiModule& FDiscordHttpApiModule::Get()
{
	static const FName DiscordHttpApiModuleName = TEXT("DiscordHttpApi");
	return FModuleManager::LoadModuleChecked<FDiscordHttpApiModule>(DiscordHttpApiModuleName);
}

FHttpRequestRef FDiscordHttpApiModule::CreateAuthorizedRequest()
{
	const FHttpRequestRef Ref = FHttpModule::Get().CreateRequest();

	switch (AuthenticationType)
	{
	case EDiscordHttpApiAuthenticationType::None:
		break;
	case EDiscordHttpApiAuthenticationType::Bot: 

#if WITH_EDITOR 
		// Bot token might not be initialized if it has been changed without restart.
		BotToken = GetMutableDefault<UDiscordHttpApiSettings>()->BotToken;
#endif
		if (BotToken.IsEmpty())
		{
			UE_LOG(LogDiscordHttpApi, Warning, TEXT("Created bot authorized request but the bot token is empty. Fill in your bot token under Settings > Discord HTTP API > Bot Token."));
		}

		Ref->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bot %s"), *BotToken));
		break;
	case EDiscordHttpApiAuthenticationType::Bearer:
		Ref->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *BearerToken));
	default: 
		break;
	}

#if WITH_EDITOR
	UE_LOG(LogDiscordHttpApi, Log, TEXT("New HTTP request created with authorization of type %s."), 
		*StaticEnum<EDiscordHttpApiAuthenticationType>()->GetNameStringByValue((int64)AuthenticationType));
#endif

	return Ref;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FDiscordHttpApiModule, DiscordHttpApi)

