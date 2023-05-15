// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordAchievementManager.h"
#include "DiscordAchievement.h"
#include "DiscordManagerUtils.h"


FDiscordUserAchievement::FDiscordUserAchievement(FRawDiscord::DiscordUserAchievement* RawAchievement)
	: AchievementId(RawAchievement->achievement_id)
	, AchievementPercentComplete(RawAchievement->percent_complete)
	, AchievementUnlockedAt(UTF8_TO_TCHAR(RawAchievement->unlocked_at))
	, UserId(RawAchievement->user_id)
{}


FDiscordUserAchievement::FDiscordUserAchievement()
	: AchievementId(0)
	, AchievementPercentComplete(0)
	, AchievementUnlockedAt()
	, UserId(0)
{}

/* static */ UDiscordAchievementManager* UDiscordAchievementManager::CreateAchievementManager(UDiscordCore* const DiscordCore)
{
	if (DiscordCore->AchievementManager)
	{
		return Cast<UDiscordAchievementManager>(DiscordCore->AchievementManager);
	}

	UDiscordAchievementManager* const Manager = NewObject<UDiscordAchievementManager>();
	
	Manager->DiscordCore = DiscordCore;
	
	DiscordCore->AchievementManager = Manager;

	DiscordCore->AchievementOnUserAchivementUpdate.BindUObject(Manager, &UDiscordAchievementManager::OnUserAchievementUpdateInternal);

	return Manager;
}

/* static */ UDiscordAchievementManager* UDiscordAchievementManager::GetAchievementManager(UDiscordCore* const DiscordCore)
{
	if (DiscordCore && DiscordCore->App.achievements) 
	{
		return CreateAchievementManager(DiscordCore);
	}
	return nullptr;
}

UDiscordAchievementManager::UDiscordAchievementManager()
	: Super()
{
}

void UDiscordAchievementManager::OnUserAchievementUpdateInternal(FRawDiscord::DiscordUserAchievement* const Achievement)
{
	OnUserAchievementUpdate.Broadcast(FDiscordUserAchievement(Achievement));
}

void UDiscordAchievementManager::SetUserAchievement(const int64 AchievementId, uint8 PercentComplete, FDiscordResultCallback Callback) const
{
	ACHIEVEMENT_CALL_RAW_CHECKED(set_user_achievement, AchievementId, PercentComplete, new FDiscordResultCallback(MoveTemp(Callback)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordAchievementManager::FetchUserAchievements(FDiscordResultCallback Callback) const
{
	ACHIEVEMENT_CALL_RAW_CHECKED(fetch_user_achievements, new FDiscordResultCallback(MoveTemp(Callback)), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

int32 UDiscordAchievementManager::CountUserAchievements() const
{
	int32 AchievementsCount = 0;
	ACHIEVEMENT_CALL_RAW_CHECKED(count_user_achievements, &AchievementsCount);
	return AchievementsCount;
}

FDiscordUserAchievement UDiscordAchievementManager::GetUserAchievementAt(const int32 Index)
{
	FRawDiscord::DiscordUserAchievement Achievement;
	FMemory::Memzero(Achievement);

	ACHIEVEMENT_CALL_RAW_CHECKED(get_user_achievement_at, Index, &Achievement);
	return FDiscordUserAchievement(&Achievement);
}

	