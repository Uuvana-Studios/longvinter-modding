// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineMessageInterface.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "MessageTypes.generated.h"

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineMessagePayloadData
{
    GENERATED_BODY()

public:
    static FOnlineMessagePayloadData FromNative(const FOnlineMessagePayload &InObj);
    FOnlineMessagePayload ToNative() const;
};