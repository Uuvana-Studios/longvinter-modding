// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IMessageSanitizerInterface.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "BlockedQueryResultInfo.generated.h"

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FBlockedQueryResultInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool Blocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool BlockedNonFriends;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString UserId;

    static FBlockedQueryResultInfo FromNative(const FBlockedQueryResult &InObj);
    FBlockedQueryResult ToNative();
};