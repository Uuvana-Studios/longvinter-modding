// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordUserManager.h"
#include "DiscordManagerUtils.h"

/* static */ UDiscordUserManager* UDiscordUserManager::GetUserManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->UserManager)
	{
		return Cast<UDiscordUserManager>(DiscordCore->UserManager);
	}

	UDiscordUserManager* const Manager = NewObject<UDiscordUserManager>();

	DiscordCore->UserManager = Manager;

	DiscordCore->UserOnUpdate.BindUObject(Manager, &UDiscordUserManager::OnCurrentUserUpdateInternal);

	Manager->DiscordCore = DiscordCore;

	return Manager;
}

FDiscordUser UDiscordUserManager::GetCurrentUser()
{
	FRawDiscord::DiscordUser User;
	FMemory::Memzero(User);
	USER_CALL_RAW_CHECKED(get_current_user, &User);
	return FDiscordUser(&User);
}

void UDiscordUserManager::GetUser(const int64 UserId, const FDiscordUserCallback& Callback)
{
	const auto FuncCallback = [](void* Data, FRawDiscord::EDiscordResult Result, FRawDiscord::DiscordUser* Raw)
	{
		FDiscordUserCallback* const Cb = (FDiscordUserCallback*)Data;

		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), FDiscordUser(Raw));

		delete Cb;
	};

	USER_CALL_RAW_CHECKED(get_user, UserId, new FDiscordUserCallback(Callback), FuncCallback);
}

EDiscordPremiumType UDiscordUserManager::GetCurrentUserPremiumType()
{
	FRawDiscord::EDiscordPremiumType Type = FRawDiscord::EDiscordPremiumType::None;
	USER_CALL_RAW_CHECKED(get_current_user_premium_type, &Type);
	return static_cast<EDiscordPremiumType>(Type);
}

bool UDiscordUserManager::CurrentUserHasFlag(const EDiscordUserFlag Flag)
{
	FRawDiscord::EDiscordUserFlag Val = FRawDiscord::EDiscordUserFlag::Partner;

#define CASE(X) case EDiscordUserFlag:: X : Val = FRawDiscord::EDiscordUserFlag:: X; break;
	switch (Flag)
	{
	CASE(Partner);
	CASE(HypeSquadEvents);
	CASE(HypeSquadHouse1);
	CASE(HypeSquadHouse2);
	CASE(HypeSquadHouse3);
	}
#undef CASE

	bool bHasFlag = false;
	USER_CALL_RAW_CHECKED(current_user_has_flag, Val, &bHasFlag);
	return bHasFlag;
}

#undef STORE_CALL_RAW_CHECKED

void UDiscordUserManager::OnCurrentUserUpdateInternal()
{
	OnCurrentUserUpdate.Broadcast(GetCurrentUser());
}
