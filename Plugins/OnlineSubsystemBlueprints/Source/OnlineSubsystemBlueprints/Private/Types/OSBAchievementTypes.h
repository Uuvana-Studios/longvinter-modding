// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineAchievementsInterface.h"
#include "OSBSessionTypes.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OSBAchievementTypes.generated.h"

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineAchievementBP
{
    GENERATED_BODY()

public:
    /** The id of the achievement */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    FString Id;

    /** The progress towards completing this achievement: 0.0-100.0 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    float Progress;

    static FOnlineAchievementBP FromNative(FOnlineAchievement Achievement);
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineAchievementDescBP
{
    GENERATED_BODY()

public:
    /** The localized title of the achievement */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    FText Title;

    /** The localized locked description of the achievement */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    FText LockedDesc;

    /** The localized unlocked description of the achievement */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    FText UnlockedDesc;

    /** Flag for whether the achievement is hidden */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    bool bIsHidden;

    /** The date/time the achievement was unlocked */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Achievements")
    FDateTime UnlockTime;

    static FOnlineAchievementDescBP FromNative(FOnlineAchievementDesc AchievementDescription);
};