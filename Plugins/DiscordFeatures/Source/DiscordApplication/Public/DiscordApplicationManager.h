// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordApplicationManager.generated.h"

USTRUCT(BlueprintType)
struct DISCORDAPPLICATION_API FDiscordOAuth2Token
{
	GENERATED_BODY()
public:
	// A bearer token for the current user
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	FString AccessToken;
	// A list of oauth2 scopes as a single string, delineated by spaces like "identify rpc gdm.join"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	FString Scopes;
	// The timestamp at which the token expires
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	int64 Expires;
};

USTRUCT(BlueprintType)
struct DISCORDAPPLICATION_API FDiscordSignedAppTicket
{
	GENERATED_BODY()
public:
	// The application id for the ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	int64 ApplicationId;
	// The user for the ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	FDiscordUser User;
	// The list of the user's entitlements for this application
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	TArray<FDiscordEntitlement> Entitlements;
	// The ISO 8601 timestamp for the ticket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Application")
	FString Timestamp;
};

DECLARE_DELEGATE_TwoParams(FOnOAuth2TokenReceived, EDiscordResult, const FDiscordOAuth2Token&);
DECLARE_DELEGATE_TwoParams(FOnTicketReceived, EDiscordResult, const FString&);

UCLASS()
class DISCORDAPPLICATION_API UDiscordApplicationManager : public UDiscordManager
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Application", BlueprintPure, meta = (CompactNodeTitle = "APPLICATION MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "Application Manager") UDiscordApplicationManager* GetApplicationManager(UDiscordCore* const DiscordCore);

	// Get's the locale the current user has Discord set to.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Application")
	UPARAM(DisplayName = "Locale") FString GetCurrentLocale();

	// Get the name of pushed branch on which the game is running. These are branches that you created and pushed using Dispatch.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Application")
	UPARAM(DisplayName = "Branch") FString GetCurrentBranch();

	// Retrieve an OAuth2 bearer token for the current user.
	// If your game was launched from Discord and you call this function, 
	// you will automatically receive the token.
	// If the game was not launched from Discordand this method is called, 
	// Discord will focus itself and prompt the user for authorization.
	void GetOAuth2Token(const FOnOAuth2TokenReceived& OnOAuth2TokenReceived);

	// Checks if the current user has the entitlement to run this game.
	void ValidateOrExit(const FDiscordResultCallback& OnValidated);

	// Get the signed app ticket for the current user.
	void GetTicket(const FOnTicketReceived& OnTicketReceived);
};
