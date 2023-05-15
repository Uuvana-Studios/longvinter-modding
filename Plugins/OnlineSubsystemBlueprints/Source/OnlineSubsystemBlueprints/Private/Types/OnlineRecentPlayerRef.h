// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Types/OnlineUserRef.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OnlineRecentPlayerRef.generated.h"

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineRecentPlayerRef : public UOnlineUserRef
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineRecentPlayer> RecentPlayer;

public:
    static UOnlineRecentPlayerRef *FromRecentPlayer(const TSharedPtr<FOnlineRecentPlayer> &RecentPlayer);
    TSharedPtr<FOnlineRecentPlayer> GetRecentPlayer() const
    {
        return this->RecentPlayer;
    }

    /**
     * Returns the last time this user was seen.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FDateTime GetLastSeen();
};