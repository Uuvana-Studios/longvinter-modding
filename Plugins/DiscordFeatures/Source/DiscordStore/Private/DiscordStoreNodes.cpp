// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordStoreNodes.h"

FDiscordResultCallback UBaseStoreAsyncProxy::CreateCallback()
{
	return FDiscordResultCallback::CreateUObject(this, &UBaseStoreAsyncProxy::OnResultInternal);
}

void UBaseStoreAsyncProxy::OnResultInternal(EDiscordResult Result)
{
	(Result == EDiscordResult::Ok ? OnSuccess : OnError).Broadcast(Result);
	SetReadyToDestroy();
}

/* static */ UFetchSkusProxy* UFetchSkusProxy::FetchSkus(UDiscordStoreManager* StoreManager)
{
	UFetchSkusProxy* const Proxy = NewObject<UFetchSkusProxy>();

	Proxy->Manager = StoreManager;

	return Proxy;
}

void UFetchSkusProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Fetch SKUs Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResultInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->FetchSkus(CreateCallback());
}

/* static */ UFetchEntitlementsProxy* UFetchEntitlementsProxy::FetchEntitlements(UDiscordStoreManager* StoreManager)
{
	UFetchEntitlementsProxy* const Proxy = NewObject<UFetchEntitlementsProxy>();

	Proxy->Manager = StoreManager;

	return Proxy;
}

void UFetchEntitlementsProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Fetch Entitlements Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResultInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->FetchEntitlements(CreateCallback());
}

/* static */ UStartPurchaseProxy* UStartPurchaseProxy::StartPurchase(UDiscordStoreManager* StoreManager, const int64 SkuId)
{
	UStartPurchaseProxy* const Proxy = NewObject<UStartPurchaseProxy>();

	Proxy->Manager = StoreManager;
	Proxy->SkuId = SkuId;

	return Proxy;
}

void UStartPurchaseProxy::Activate()
{
	if (!Manager)
	{
		FFrame::KismetExecutionMessage(TEXT("Start Purchase Failed. Passed nullptr for Manager."), ELogVerbosity::Error);
		OnResultInternal(EDiscordResult::InternalError);
		return;
	}

	Manager->StartPurchase(SkuId, CreateCallback());
}

