// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBAchievementTypes.h"

FOnlineAchievementBP FOnlineAchievementBP::FromNative(FOnlineAchievement Achievement)
{
    FOnlineAchievementBP Result;
    Result.Id = Achievement.Id;
    Result.Progress = Achievement.Progress;
    return Result;
}

FOnlineAchievementDescBP FOnlineAchievementDescBP::FromNative(FOnlineAchievementDesc AchievementDescription)
{
    FOnlineAchievementDescBP Result;
    Result.Title = AchievementDescription.Title;
    Result.LockedDesc = AchievementDescription.LockedDesc;
    Result.UnlockedDesc = AchievementDescription.UnlockedDesc;
    Result.bIsHidden = AchievementDescription.bIsHidden;
    Result.UnlockTime = AchievementDescription.UnlockTime;
    return Result;
}