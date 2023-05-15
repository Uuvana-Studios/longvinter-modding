// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordUserManager.generated.h"

UENUM(BlueprintType)
enum class EDiscordPremiumType : uint8
{
	// Not a Nitro subscriber
	None	= 0,
	// Nitro Classic subscriber
	Tier1	= 1 UMETA(DisplayName = "Tier1 (Nitro Classic Subscriber)"),
	// Nitro subscriber
	Tier2	= 2	UMETA(DisplayName = "Tier2 (Nitro Subscriber)")
};

// Be carefull that this enum's value is not the same as the SDK. (uint8 limitation)
UENUM(BlueprintType)
enum class EDiscordUserFlag : uint8
{
	// Discord Partner
	Partner	= 0,
	// HypeSquad Events participant
	HypeSquadEvents	= 1,
	// House Bravery
	HypeSquadHouse1	= 2,
	// House Brilliance
	HypeSquadHouse2	= 3,	
	// House Balance
	HypeSquadHouse3	= 4	
};

DECLARE_DELEGATE_TwoParams(FDiscordUserCallback, EDiscordResult, const FDiscordUser&);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentUserUpdateEvent, const FDiscordUser&, CurrentUser);

UCLASS()
class DISCORDUSER_API UDiscordUserManager : public UDiscordManager
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnCurrentUserUpdateEvent OnCurrentUserUpdate;

public:
	UFUNCTION(BlueprintCallable, Category = "Discord|User", BlueprintPure, meta = (CompactNodeTitle = "USER MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "User Manager") UDiscordUserManager* GetUserManager(UDiscordCore* const DiscordCore);

	UFUNCTION(BlueprintCallable, Category = "Discord|User", BlueprintPure)
	UPARAM(DisplayName = "User") FDiscordUser GetCurrentUser();

	void GetUser(const int64 UserId, const FDiscordUserCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|User", BlueprintPure)
	UPARAM(DisplayName = "Type") EDiscordPremiumType GetCurrentUserPremiumType();

	UFUNCTION(BlueprintCallable, Category = "Discord|User", BlueprintPure)
	UPARAM(DisplayName = "Has Flag") bool CurrentUserHasFlag(const EDiscordUserFlag Flag);

private:
	void OnCurrentUserUpdateInternal();
};

