// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif
#if defined(UE_4_25_OR_LATER)
#include "VoiceChat.h"
#include "VoiceChatResult.h"
#endif
#include "Interfaces/OnlineVoiceAdminInterface.h"

#include "OSBVoiceChatTypes.generated.h"

UENUM(BlueprintType, Meta = (ScriptName = "EVoiceChatChannelType"))
enum class EVoiceChatChannelType_ : uint8
{
    /** Non positional/2d audio channel */
    NonPositional,
    /** Positional/3d audio channel */
    Positional,
    /** Echo channel. Will only ever have one player and will echo anything you say */
    Echo
};

UENUM(BlueprintType, Meta = (ScriptName = "EVoiceChatTransmitMode"))
enum class EVoiceChatTransmitMode_ : uint8
{
    /** Transmit to none of the channels you are in */
    None,
    /** Transmit to all of the channels you are in */
    All,
    /** Transmit to a specific channel */
    Channel
};

UENUM(BlueprintType, Meta = (ScriptName = "EVoiceChatAttenuationModel"))
enum class EVoiceChatAttenuationModel_ : uint8
{
    /** No attenuation is applied. The audio will drop to 0 at MaxDistance */
    None,
    /** The attenuation increases in inverse proportion to the distance. The Rolloff is the inverse of the slope of the
     * attenuation curve. */
    InverseByDistance,
    /** The attenuation increases in linear proportion to the distance. The Rolloff is the negative slope of the
     * attenuation curve. */
    LinearByDistance,
    /** The attenuation increases in inverse proportion to the distance raised to the power of the Rolloff. */
    ExponentialByDistance
};

UENUM(BlueprintType, Meta = (ScriptName = "EVoiceChatResult"))
enum class EVoiceChatResult_ : uint8
{
    // The operation succeeded
    Success = 0,

    // Common state errors
    InvalidState,
    NotInitialized,
    NotConnected,
    NotLoggedIn,
    NotPermitted,
    Throttled,

    // Common argument errors
    InvalidArgument,
    CredentialsInvalid,
    CredentialsExpired,

    // Common connection errors
    ClientTimeout,
    ServerTimeout,
    DnsFailure,
    ConnectionFailure,

    // Error does not map to any common categories
    ImplementationError
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FVoiceChatResultBP
{
    GENERATED_BODY()

public:
    /** If the operation was successful */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    bool Successful;

    /** Success, or an error category */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    EVoiceChatResult_ ResultCode;

    /** If we failed, the code for the error */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    FString ErrorCode;

    /** If we failed, a numeric error from the implementation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    int ErrorNum;

    /** If we failed, more details about the error condition */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    FString ErrorDesc;

#if defined(UE_4_25_OR_LATER)
    static FVoiceChatResultBP FromNative(const FVoiceChatResult &InObj);
    FVoiceChatResult ToNative() const;
#endif
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FVoiceChatDeviceInfoBP
{
    GENERATED_BODY()

public:
    /** The display name for the device */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    FString DisplayName;

    /** The unique id for the device */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    FString Id;

#if defined(UE_4_26_OR_LATER)
    static FVoiceChatDeviceInfoBP FromNative(const FVoiceChatDeviceInfo &InObj);
    FVoiceChatDeviceInfo ToNative() const;
#endif
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FVoiceChatChannel3dPropertiesBP
{
    GENERATED_BODY()

public:
    /** If the channel's 3D properties are set */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    bool bSet;

    /** The model used to determine how loud audio is at different distances */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    EVoiceChatAttenuationModel_ AttenuationModel;

    /** The distance at which the sound will start to attenuate */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    float MinDistance;

    /** The distance at which sound will no longer be audible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    float MaxDistance;

    /** How fast the sound attenuates with distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceChat")
    float Rolloff;

#if defined(UE_4_25_OR_LATER)
    static FVoiceChatChannel3dPropertiesBP FromNative(const TOptional<FVoiceChatChannel3dProperties> &InObj);
    TOptional<FVoiceChatChannel3dProperties> ToNative() const;
#endif
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FVoiceAdminChannelCredentialsBP
{
    GENERATED_BODY()

public:
    /** The user that this credential is for. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceAdmin")
    FUniqueNetIdRepl TargetUserId;

    /** The player name that the target user should pass to IVoiceChatUser::Login. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceAdmin")
    FString PlayerName;

    /** The channel credentials that the target user should pass to IVoiceChatUser::Login. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|VoiceAdmin")
    FString ChannelCredentials;

    static FVoiceAdminChannelCredentialsBP FromNative(const FVoiceAdminChannelCredentials &InObj);
    FVoiceAdminChannelCredentials ToNative() const;
};

int32 PlatformUserIdToBlueprintType(const FPlatformUserId& InUserId);
FPlatformUserId BlueprintTypeToPlatformUserId(int32 InUserId);