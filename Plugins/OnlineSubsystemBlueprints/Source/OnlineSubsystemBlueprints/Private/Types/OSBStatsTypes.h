// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "OSBSessionTypes.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OSBStatsTypes.generated.h"

UENUM(BlueprintType, Meta = (ScriptName = "EOnlineStatModificationType"))
enum class EOnlineStatModificationType_ : uint8
{
    /** Let the backend decide how to update this value (or set to new value if backend does not decide) */
    Unknown,
    /** Add the new value to the previous value */
    Sum,
    /** Overwrite previous value with the new value */
    Set,
    /** Only replace previous value if new value is larger */
    Largest,
    /** Only replace previous value if new value is smaller */
    Smallest
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineStatsUserStatsBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    FUniqueNetIdRepl PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    TMap<FString, FVariantDataBP> Stats;

    static FOnlineStatsUserStatsBP FromNative(const FOnlineStatsUserStats &InObj);
    static FOnlineStatsUserStatsBP FromNative(const TSharedRef<const FOnlineStatsUserStats> InObj);
    static FOnlineStatsUserStatsBP FromNative(const TSharedPtr<const FOnlineStatsUserStats> InObj);
    FOnlineStatsUserStats ToNative() const;
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineStatUpdateBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    EOnlineStatModificationType_ Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    FVariantDataBP Value;
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineStatsUserUpdatedStatsBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    FUniqueNetIdRepl PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Stats")
    TMap<FString, FOnlineStatUpdateBP> Stats;

    static FOnlineStatsUserUpdatedStatsBP FromNative(const FOnlineStatsUserUpdatedStats &InObj);
    FOnlineStatsUserUpdatedStats ToNative() const;
};