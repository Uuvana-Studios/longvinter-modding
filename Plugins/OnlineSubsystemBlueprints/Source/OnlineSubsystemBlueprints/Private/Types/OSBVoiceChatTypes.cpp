// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBVoiceChatTypes.h"

#if defined(UE_4_25_OR_LATER)

FVoiceChatResultBP FVoiceChatResultBP::FromNative(const FVoiceChatResult &InObj)
{
    FVoiceChatResultBP Result;
    Result.Successful = InObj.IsSuccess();
    Result.ResultCode = (EVoiceChatResult_)InObj.ResultCode;
    Result.ErrorCode = InObj.ErrorCode;
    Result.ErrorNum = InObj.ErrorNum;
    Result.ErrorDesc = InObj.ErrorDesc;
    return Result;
}

FVoiceChatResult FVoiceChatResultBP::ToNative() const
{
    if (this->Successful)
    {
        return FVoiceChatResult::CreateSuccess();
    }

    return FVoiceChatResult((EVoiceChatResult)this->ResultCode, this->ErrorCode, this->ErrorNum, this->ErrorDesc);
}

#if defined(UE_4_26_OR_LATER)

FVoiceChatDeviceInfoBP FVoiceChatDeviceInfoBP::FromNative(const FVoiceChatDeviceInfo &InObj)
{
    FVoiceChatDeviceInfoBP Result;
    Result.DisplayName = InObj.DisplayName;
    Result.Id = InObj.Id;
    return Result;
}

FVoiceChatDeviceInfo FVoiceChatDeviceInfoBP::ToNative() const
{
    FVoiceChatDeviceInfo Result;
    Result.DisplayName = this->DisplayName;
    Result.Id = this->Id;
    return Result;
}

#endif // #if defined(UE_4_26_OR_LATER)

FVoiceChatChannel3dPropertiesBP FVoiceChatChannel3dPropertiesBP::FromNative(
    const TOptional<FVoiceChatChannel3dProperties> &InObj)
{
    FVoiceChatChannel3dPropertiesBP Result;
    if (!InObj.IsSet())
    {
        Result.bSet = false;
    }
    else
    {
        Result.bSet = true;
        Result.AttenuationModel = (EVoiceChatAttenuationModel_)InObj.GetValue().AttenuationModel;
        Result.MinDistance = InObj.GetValue().MinDistance;
        Result.MaxDistance = InObj.GetValue().MaxDistance;
        Result.Rolloff = InObj.GetValue().Rolloff;
    }
    return Result;
}

TOptional<FVoiceChatChannel3dProperties> FVoiceChatChannel3dPropertiesBP::ToNative() const
{
    if (!this->bSet)
    {
        return TOptional<FVoiceChatChannel3dProperties>();
    }

    FVoiceChatChannel3dProperties Result;
    Result.AttenuationModel = (EVoiceChatAttenuationModel)this->AttenuationModel;
    Result.MinDistance = this->MinDistance;
    Result.MaxDistance = this->MaxDistance;
    Result.Rolloff = this->Rolloff;
    return Result;
}

#endif // #if defined(UE_4_25_OR_LATER)

FVoiceAdminChannelCredentialsBP FVoiceAdminChannelCredentialsBP::FromNative(const FVoiceAdminChannelCredentials &InObj)
{
    FVoiceAdminChannelCredentialsBP Result;
    Result.TargetUserId = InObj.TargetUserId;
    Result.PlayerName = InObj.PlayerName;
    Result.ChannelCredentials = InObj.ChannelCredentials;
    return Result;
}

FVoiceAdminChannelCredentials FVoiceAdminChannelCredentialsBP::ToNative() const
{
    FVoiceAdminChannelCredentials Result = {};
    Result.TargetUserId = this->TargetUserId.IsValid() ? this->TargetUserId.GetUniqueNetId() : nullptr;
    Result.PlayerName = this->PlayerName;
    Result.ChannelCredentials = this->ChannelCredentials;
    return Result;
}

#if defined(UE_5_0_OR_LATER)
int32 PlatformUserIdToBlueprintType(const FPlatformUserId &InUserId)
{
    return InUserId.GetInternalId();
}

FPlatformUserId BlueprintTypeToPlatformUserId(int32 InUserId)
{
    return FPlatformUserId::CreateFromInternalId(InUserId);
}
#else
int32 PlatformUserIdToBlueprintType(const FPlatformUserId &InUserId)
{
    return (int32)InUserId;
}

FPlatformUserId BlueprintTypeToPlatformUserId(int32 InUserId)
{
    return (FPlatformUserId)InUserId;
}
#endif // #if defined(UE_5_0_OR_LATER)