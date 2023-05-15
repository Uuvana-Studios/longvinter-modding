// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordRelationshipManager.h"
#include "DiscordManagerUtils.h"

FDiscordPresence::FDiscordPresence(FRawDiscord::DiscordPresence* Raw)
	: Status(static_cast<EDiscordStatus>(Raw->status))
	, Activity(&Raw->activity)
{
}

FDiscordRelationship::FDiscordRelationship(FRawDiscord::DiscordRelationship* const Raw)
{
	User	 = FDiscordUser(&Raw->user);
	Type	 = static_cast<EDiscordRelationshipType>(Raw->type);
	Presence = FDiscordPresence(&Raw->presence);
}

/* static */ UDiscordRelationshipManager* UDiscordRelationshipManager::GetRelationshipManager(UDiscordCore* DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->RelationshipManager)
	{
		return Cast<UDiscordRelationshipManager>(DiscordCore->RelationshipManager);
	}

	UDiscordRelationshipManager* const Manager = NewObject<UDiscordRelationshipManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->RelationshipManager = Manager;

	DiscordCore->RelationshipOnUpdate   .BindUObject(Manager, &UDiscordRelationshipManager::OnRelationshipUpdateInternal);
	DiscordCore->RelationshipOnRefreshed.BindUObject(Manager, &UDiscordRelationshipManager::OnRelationshipRefreshedInternal);

	return Manager;
}

void UDiscordRelationshipManager::Filter(const FDiscordFilterFunction& FilterFunction)
{
	if (!FilterFunction.IsBound())
	{
		return;
	}

	const auto Callback = [](void* Data, FRawDiscord::DiscordRelationship* Relationship) -> bool
	{
		FDiscordFilterFunction* const Func = (FDiscordFilterFunction*)Data;

		FDiscordRelationship Rel(Relationship);

		return Func->Execute(Rel);
	};

	RELATIONSHIP_CALL_RAW_CHECKED(filter, (void*)&FilterFunction, Callback);
}

FDiscordRelationship UDiscordRelationshipManager::Get(const int64 UserId)
{
	FRawDiscord::DiscordRelationship Raw;
	RELATIONSHIP_CALL_RAW_CHECKED(get, UserId, &Raw);
	return FDiscordRelationship(&Raw);
}

FDiscordRelationship UDiscordRelationshipManager::GetAt(const int64 Index)
{
	FRawDiscord::DiscordRelationship Raw;
	RELATIONSHIP_CALL_RAW_CHECKED(get_at, Index, &Raw);
	return FDiscordRelationship(&Raw);
}

int32 UDiscordRelationshipManager::Count()
{
	int32 Count = 0;
	RELATIONSHIP_CALL_RAW_CHECKED(count, &Count);
	return Count;
}

#undef RELATIONSHIP_CALL_FUNCTION

void UDiscordRelationshipManager::OnRelationshipUpdateInternal(FRawDiscord::DiscordRelationship* Relationship)
{
	FDiscordRelationship Rel(Relationship);

	OnRelationshipUpdate.Broadcast(Rel);
}

void UDiscordRelationshipManager::OnRelationshipRefreshedInternal()
{
	OnRelationshipRefreshed.Broadcast();
}
