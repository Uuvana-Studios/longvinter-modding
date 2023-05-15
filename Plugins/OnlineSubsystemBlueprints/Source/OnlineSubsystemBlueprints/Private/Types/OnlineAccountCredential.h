// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineIdentityInterface.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OnlineAccountCredential.generated.h"

/**
 * Represents the credential information for logging into an identity system.
 */
USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineAccountCredential
{
    GENERATED_BODY()

public:
    /** Type of account. Needed to identity the auth method to use (epic, internal, facebook, etc) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Type;

    /** Id of the user logging in (email, display name, facebook id, etc) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Id;

    /** Credentials of the user logging in (password or auth token) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Token;

    static FOnlineAccountCredential FromNative(const FOnlineAccountCredentials &InCred);
    FOnlineAccountCredentials ToNative();
};