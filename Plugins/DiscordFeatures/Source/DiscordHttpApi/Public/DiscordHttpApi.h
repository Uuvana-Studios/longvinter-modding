// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Modules/ModuleManager.h"

#include "DiscordHttpApi.generated.h"

UENUM(BlueprintType)
enum class EDiscordHttpApiAuthenticationType : uint8
{
	None,
	Bot,
	Bearer
};

UCLASS(config=DiscordHttpApi)
class DISCORDHTTPAPI_API UDiscordHttpApiSettings : public UObject
{
	GENERATED_BODY()
public:
	UDiscordHttpApiSettings();

	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Authentication")
	FString BotToken;

	// TODO: Move it to its own lib

	/**
	 * This function exists because there's an engine bug with int64 numbers being
	 * truncated to a the number ending by 32 in the editor even when the range-limit 
	 * is far from being reached.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Misc", meta=(CompactNodeTitle = "INT64"))
	static FORCEINLINE int64 MakeInt64(const FString& Int64)
	{
		return FCString::Atoi64(*Int64);
	}
};


class DISCORDHTTPAPI_API FDiscordHttpApiModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FDiscordHttpApiModule& Get();

	FHttpRequestRef CreateAuthorizedRequest();

private:
	bool OnSettingsSaved();

private:
	EDiscordHttpApiAuthenticationType AuthenticationType;

	FString BotToken;
	FString BearerToken;

};

