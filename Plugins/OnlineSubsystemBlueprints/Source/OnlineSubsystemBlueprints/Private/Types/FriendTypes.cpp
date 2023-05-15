// Copyright June Rhodes. All Rights Reserved.

#include "Types/FriendTypes.h"

#include "Interfaces/OnlinePresenceInterface.h"

UOnlineFriendRef *UOnlineFriendRef::FromFriend(const TSharedPtr<FOnlineFriend> &Friend)
{
    auto Ref = NewObject<UOnlineFriendRef>();
    Ref->Friend = Friend;
    Ref->AssignFromUser(Friend);
    return Ref;
}

EInviteStatus_ UOnlineFriendRef::GetInviteStatus_Implementation()
{
    return this->GetInviteStatus_Native();
}

FOnlineUserPresenceData UOnlineFriendRef::GetPresence_Implementation()
{
    return this->GetPresence_Native();
}

EInviteStatus_ UOnlineFriendRef::GetInviteStatus_Native()
{
    return (EInviteStatus_)this->Friend->GetInviteStatus();
}

FOnlineUserPresenceData UOnlineFriendRef::GetPresence_Native()
{
    const FOnlineUserPresence &Presence = this->Friend->GetPresence();
    return FOnlineUserPresenceData::FromNative(Presence);
}

FReportPlayedWithUserInfo FReportPlayedWithUserInfo::FromNative(const FReportPlayedWithUser &InObj)
{
    FReportPlayedWithUserInfo Result;
    Result.UserId = FUniqueNetIdRepl(InObj.UserId);
    Result.PresenceStr = InObj.PresenceStr;
    return Result;
}

FReportPlayedWithUser FReportPlayedWithUserInfo::ToNative() const
{
    FReportPlayedWithUser Result(this->UserId.GetUniqueNetId().ToSharedRef(), this->PresenceStr);
    return Result;
}

FFriendSettingsData FFriendSettingsData::FromNative(const FFriendSettings &InObj)
{
    FFriendSettingsData Result;
    Result.Data = InObj.SettingsMap;
    return Result;
}

FFriendSettings FFriendSettingsData::ToNative() const
{
    FFriendSettings Result;
    Result.SettingsMap = this->Data;
    return Result;
}

FOnlineFriendSettingsSourceDataConfig FOnlineFriendSettingsSourceDataConfig::FromNative(
    const TSharedRef<FOnlineFriendSettingsSourceData> &InObj)
{
    FOnlineFriendSettingsSourceDataConfig Result;
    Result.NeverShowAgain = InObj->bNeverShowAgain;
    return Result;
}

FOnlineFriendSettingsSourceData FOnlineFriendSettingsSourceDataConfig::ToNative() const
{
    FOnlineFriendSettingsSourceData Result;
    Result.bNeverShowAgain = this->NeverShowAgain;
    return Result;
}