// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineUserPresenceData.h"
#include "OnlineUserRef.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "FriendTypes.generated.h"

UENUM(BlueprintType, Meta = (ScriptName = "EInviteStatus"))
enum class EInviteStatus_ : uint8
{
    /** unknown state */
    Unknown,
    /** Friend has accepted the invite */
    Accepted,
    /** Friend has sent player an invite, but it has not been accepted/rejected */
    PendingInbound,
    /** Player has sent friend an invite, but it has not been accepted/rejected */
    PendingOutbound,
    /** Player has been blocked */
    Blocked,
    /** Suggested friend */
    Suggested
};

UCLASS(BlueprintType, Blueprintable, Transient, Meta = (DontUseGenericSpawnObject))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineFriendRef : public UOnlineUserRef
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineFriend> Friend;

public:
    static UOnlineFriendRef *FromFriend(const TSharedPtr<FOnlineFriend> &Friend);
    TSharedPtr<FOnlineFriend> GetFriend() const
    {
        return this->Friend;
    }

    /**
     * Returns the invite status of the friend.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    EInviteStatus_ GetInviteStatus();

    /**
     * Returns the presence information for the friend.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    FOnlineUserPresenceData GetPresence();

protected:
    virtual EInviteStatus_ GetInviteStatus_Native();
    virtual FOnlineUserPresenceData GetPresence_Native();
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FReportPlayedWithUserInfo
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FUniqueNetIdRepl UserId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString PresenceStr;

    static FReportPlayedWithUserInfo FromNative(const FReportPlayedWithUser &InObj);
    FReportPlayedWithUser ToNative() const;
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FFriendSettingsData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    TMap<FString, FString> Data;

    static FFriendSettingsData FromNative(const FFriendSettings &InObj);
    FFriendSettings ToNative() const;
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FOnlineFriendSettingsSourceDataConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool NeverShowAgain;

    static FOnlineFriendSettingsSourceDataConfig FromNative(const TSharedRef<FOnlineFriendSettingsSourceData> &InObj);
    FOnlineFriendSettingsSourceData ToNative() const;
};