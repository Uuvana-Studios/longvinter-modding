// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordStoreManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordStoreNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoreResultEvent, EDiscordResult, Result);

UCLASS(Abstract)
class UBaseStoreAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnStoreResultEvent OnSuccess;
	
	UPROPERTY(BlueprintAssignable)
	FOnStoreResultEvent OnError;

protected:
	UPROPERTY()
	UDiscordStoreManager* Manager;

	FDiscordResultCallback CreateCallback();

	void OnResultInternal(EDiscordResult Result);
};

UCLASS()
class UFetchSkusProxy : public UBaseStoreAsyncProxy
{
	GENERATED_BODY()
public:
	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Fetch SKUs"))
	static UFetchSkusProxy* FetchSkus(UDiscordStoreManager* StoreManager);
};

UCLASS()
class UFetchEntitlementsProxy : public UBaseStoreAsyncProxy
{
	GENERATED_BODY()
public:
	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Fetch Entitlements"))
	static UFetchEntitlementsProxy* FetchEntitlements(UDiscordStoreManager* StoreManager);
};

UCLASS()
class UStartPurchaseProxy : public UBaseStoreAsyncProxy
{
	GENERATED_BODY()
public:
	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Store", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Start Purchase"))
	static UStartPurchaseProxy* StartPurchase(UDiscordStoreManager* StoreManager, const int64 SkuId);

private:
	int64 SkuId;
};

