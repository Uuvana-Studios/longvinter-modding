// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordStoreManager.h"
#include "DiscordManagerUtils.h"

FDiscordSkuPrice::FDiscordSkuPrice(FRawDiscord::DiscordSkuPrice* Raw)
{
	Amount   = Raw->amount;
	Currency = UTF8_TO_TCHAR(Raw->currency);
}
FDiscordSku::FDiscordSku(FRawDiscord::DiscordSku* Sku)
{
	Id	  = Sku->id;
	Type  = static_cast<EDiscordSkuType>(Sku->type);
	Name  = UTF8_TO_TCHAR(Sku->name);
	Price = FDiscordSkuPrice(&Sku->price);
}

/* static */ UDiscordStoreManager* UDiscordStoreManager::GetStoreManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->StoreManager)
	{
		return Cast<UDiscordStoreManager>(DiscordCore->StoreManager);
	}

	UDiscordStoreManager* const Manager = NewObject<UDiscordStoreManager>();

	DiscordCore->StoreManager = Manager;

	DiscordCore->EntitlementOnCreate.BindUObject(Manager, &UDiscordStoreManager::OnEntitlementCreateInternal);
	DiscordCore->EntitlementOnDelete.BindUObject(Manager, &UDiscordStoreManager::OnEntitlementDeleteInternal);

	Manager->DiscordCore = DiscordCore;

	return Manager;
}

void UDiscordStoreManager::FetchSkus(const FDiscordResultCallback& Callback)
{
	STORE_CALL_RAW_CHECKED(fetch_skus, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

int32 UDiscordStoreManager::CountSkus()
{
	int32 Val = 0;
	STORE_CALL_RAW_CHECKED(count_skus, &Val);
	return Val;
}

FDiscordSku UDiscordStoreManager::GetSku(const int64 SkuId)
{
	FRawDiscord::DiscordSku Raw;
	FMemory::Memzero(Raw);

	STORE_CALL_RAW_CHECKED(get_sku, SkuId, &Raw);

	return FDiscordSku(&Raw);
}

FDiscordSku UDiscordStoreManager::GetSkuAt(const int32 Index)
{
	FRawDiscord::DiscordSku Raw;
	FMemory::Memzero(Raw);

	STORE_CALL_RAW_CHECKED(get_sku_at, Index, &Raw);

	return FDiscordSku(&Raw);
}

void UDiscordStoreManager::FetchEntitlements(const FDiscordResultCallback& Callback)
{
	STORE_CALL_RAW_CHECKED(fetch_entitlements, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

int32 UDiscordStoreManager::CountEntitlements()
{
	int32 Val = 0;
	STORE_CALL_RAW_CHECKED(count_entitlements, &Val);
	return Val;
}

FDiscordEntitlement UDiscordStoreManager::GetEntitlement(const int64 EntitlementId)
{
	FRawDiscord::DiscordEntitlement Raw;
	FMemory::Memzero(Raw);

	STORE_CALL_RAW_CHECKED(get_entitlement, EntitlementId, &Raw);

	return FDiscordEntitlement(&Raw);
}

FDiscordEntitlement UDiscordStoreManager::GetEntitlementAt(const int32 Index)
{
	FRawDiscord::DiscordEntitlement Raw;
	FMemory::Memzero(Raw);

	STORE_CALL_RAW_CHECKED(get_entitlement_at, Index, &Raw);

	return FDiscordEntitlement(&Raw);
}

bool UDiscordStoreManager::HasSkuEntitlement(const int64 SkuId)
{
	bool bHas = false;
	STORE_CALL_RAW_CHECKED(has_sku_entitlement, SkuId, &bHas);
	return bHas;
}

void UDiscordStoreManager::StartPurchase(const int64 SkuId, const FDiscordResultCallback& Callback)
{
	STORE_CALL_RAW_CHECKED(start_purchase, SkuId, new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

#undef STORE_CALL_RAW_CHECKED

void UDiscordStoreManager::OnEntitlementCreateInternal(FRawDiscord::DiscordEntitlement* Raw)
{
	OnEntitlementCreate.Broadcast(FDiscordEntitlement(Raw));
}

void UDiscordStoreManager::OnEntitlementDeleteInternal(FRawDiscord::DiscordEntitlement* Raw)
{
	OnEntitlementDelete.Broadcast(FDiscordEntitlement(Raw));
}
