// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordStoreManager.generated.h"

UENUM(BlueprintType)
enum class EDiscordSkuType : uint8
{
	// Invalid. For Blueprint compatibility. Don't use it.
	None = 0 UMETA(Hidden),

	// SKU is a game
	Application	= 1,
	// SKU is a DLC
	DLC = 2,	
	// SKU is a consumable(in - app purchase)
	Consumable = 3,
	// SKU is a bundle(comprising the other 3 types)
	Bundle = 4
};

USTRUCT(BlueprintType)
struct DISCORDSTORE_API FDiscordSkuPrice
{
	GENERATED_BODY()
private:
	friend class UDiscordStoreManager;
	friend struct FDiscordSku;
	FDiscordSkuPrice(FRawDiscord::DiscordSkuPrice* Raw);
public:
	FDiscordSkuPrice() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	int64 Amount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	FString Currency;
};

USTRUCT(BlueprintType)
struct DISCORDSTORE_API FDiscordSku
{
	GENERATED_BODY()
private:
	friend class UDiscordStoreManager;
	FDiscordSku(FRawDiscord::DiscordSku* Sku);
public:
	FDiscordSku() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	int64 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	EDiscordSkuType Type = EDiscordSkuType::Application;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Store")
	FDiscordSkuPrice Price;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordEntitlementEvent, const FDiscordEntitlement&, Entitlement);

UCLASS()
class DISCORDSTORE_API UDiscordStoreManager : public UDiscordManager
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordEntitlementEvent OnEntitlementCreate;

	UPROPERTY(BlueprintAssignable)
	FDiscordEntitlementEvent OnEntitlementDelete;
public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure, meta = (CompactNodeTitle = "STORE MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "Store Manager") UDiscordStoreManager* GetStoreManager(UDiscordCore* const DiscordCore);

	void FetchSkus(const FDiscordResultCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "SKUs Count") int32 CountSkus();

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "SKU") FDiscordSku GetSku(const int64 SkuId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "SKU") FDiscordSku GetSkuAt(const int32 Index);

	void FetchEntitlements(const FDiscordResultCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "Count")  int32 CountEntitlements();

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "Entitlement") FDiscordEntitlement GetEntitlement(const int64 EntitlementId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "Entitlement") FDiscordEntitlement GetEntitlementAt(const int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", BlueprintPure)
	UPARAM(DisplayName = "Has SKU") bool HasSkuEntitlement(const int64 SkuId);

	void StartPurchase(const int64 SkuId, const FDiscordResultCallback& Callback);

private:
	void OnEntitlementCreateInternal(FRawDiscord::DiscordEntitlement* Raw);
	void OnEntitlementDeleteInternal(FRawDiscord::DiscordEntitlement* Raw);

};

