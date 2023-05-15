// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordAchievementManager.generated.h"

USTRUCT(BlueprintType)
struct FDiscordUserAchievement
{
	GENERATED_BODY()
private:
	friend class UDiscordAchievementManager;
	FDiscordUserAchievement(FRawDiscord::DiscordUserAchievement* RawAchievement);
public:
	FDiscordUserAchievement();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Achievement")
	int64 AchievementId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Achievement")
	uint8 AchievementPercentComplete;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Achievement")
	FString AchievementUnlockedAt;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Achievement")
	int64 UserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAchievementOnUserAchievementUpdate, const FDiscordUserAchievement&, Achievement);

/**
 *	Wrapper around the SDK's Network Manager.
*/
UCLASS()
class DISCORDACHIEVEMENT_API UDiscordAchievementManager : public UDiscordManager
{
	GENERATED_BODY()

private:
	UDiscordAchievementManager();

	static UDiscordAchievementManager* CreateAchievementManager(UDiscordCore* const DiscordCore);

	void OnUserAchievementUpdateInternal(FRawDiscord::DiscordUserAchievement* const Achievement);

public:
	/**
	 * @return The Network Manager associated with this core.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Achievement", BlueprintPure, meta =(CompactNodeTitle = "ACHIEVEMENT MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Achievement Manager") UDiscordAchievementManager* GetAchievementManager(UDiscordCore* const DiscordCore);

	
	void SetUserAchievement(const int64 AchievementId, uint8 PercentComplete, FDiscordResultCallback Callback) const;

	void FetchUserAchievements(FDiscordResultCallback Callback) const;

	UFUNCTION(BlueprintCallable, Category = "Discord|Achievement", BlueprintPure)
	UPARAM(DisplayName = "Number of Achievements") int32 CountUserAchievements() const;

	UFUNCTION(BlueprintCallable, Category = "Discord|Achievement", BlueprintPure)
	UPARAM(DisplayName = "Achievement") FDiscordUserAchievement GetUserAchievementAt(const int32 Index);

	UPROPERTY(BlueprintAssignable)
	FAchievementOnUserAchievementUpdate OnUserAchievementUpdate;

};

