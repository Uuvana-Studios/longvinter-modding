// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordRelationshipManager.generated.h"

UENUM(BlueprintType)
enum class EDiscordRelationshipType : uint8
{
	// user has no intrinsic relationship
	None,
	// user is a friend
	Friend,
	// user is blocked
	Blocked,
	// user has a pending incoming friend request to connected user
	PendingIncoming,
	// current user has a pending outgoing friend request to user
	PendingOutgoing,
	// user is not friends, but interacts with current user often(frequency + recency)
	Implicit
};

UENUM(BlueprintType)
enum class EDiscordStatus : uint8
{
	Offline,	
	Online,	
	Idle,	
	DoNotDisturb
};

USTRUCT(BlueprintType)
struct DISCORDRELATIONSHIP_API FDiscordPresence
{
	GENERATED_BODY()
private:
	friend class UDiscordRelationshipManager;
	friend struct FDiscordRelationship;
	FDiscordPresence(FRawDiscord::DiscordPresence* Raw);

public:
	FDiscordPresence() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Relationship")
	EDiscordStatus Status = EDiscordStatus::Offline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Relationship")
	FDiscordActivity Activity;
};

USTRUCT(BlueprintType)
struct DISCORDRELATIONSHIP_API FDiscordRelationship
{
	GENERATED_BODY()
private:
	friend class UDiscordRelationshipManager;
	FDiscordRelationship(FRawDiscord::DiscordRelationship* const Raw);
public:
	FDiscordRelationship() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Relationship")
	EDiscordRelationshipType Type = EDiscordRelationshipType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Relationship")
	FDiscordUser User;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Relationship")
	FDiscordPresence Presence;
};

DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FDiscordFilterFunction, const FDiscordRelationship&, Relationship);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRelationshipRefreshed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelationshipUpdate, const FDiscordRelationship&, Relationship);

UCLASS()
class DISCORDRELATIONSHIP_API UDiscordRelationshipManager : public UDiscordManager
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOnRelationshipRefreshed OnRelationshipRefreshed;

	UPROPERTY(BlueprintAssignable)
	FOnRelationshipUpdate OnRelationshipUpdate;

public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Relationship", BlueprintPure, meta = (CompactNodeTitle = "RELATIONSHIP MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Relationship Manager") UDiscordRelationshipManager* GetRelationshipManager(UDiscordCore* DiscordCore);

	UFUNCTION(BlueprintCallable, Category = "Discord|Relationship")
	void Filter(const FDiscordFilterFunction& FilterFunction);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Relationship")
	FDiscordRelationship Get(const int64 UserId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Relationship")
	FDiscordRelationship GetAt(const int64 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Relationship")
	int32 Count();

private:
	void OnRelationshipUpdateInternal(FRawDiscord::DiscordRelationship* Relationship);
	void OnRelationshipRefreshedInternal();

};

