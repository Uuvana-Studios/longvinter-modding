// Copyright June Rhodes. All Rights Reserved.

#include "Types/OnlineUserPresenceData.h"

FOnlineUserPresenceStatusData FOnlineUserPresenceStatusData::FromNative(const FOnlineUserPresenceStatus &InObj)
{
    FOnlineUserPresenceStatusData Status;
    Status.Status = InObj.StatusStr;
    Status.State = (FOnlineUserPresenceStatusState)InObj.State;
    for (auto Prop : InObj.Properties)
    {
        FString Val;
        if (Prop.Value.GetType() == EOnlineKeyValuePairDataType::String)
        {
            Prop.Value.GetValue(Val);
            Status.Properties.Add(Prop.Key, Val);
        }
    }
    return Status;
}

FOnlineUserPresenceStatus FOnlineUserPresenceStatusData::ToNative() const
{
    FOnlineUserPresenceStatus Result;
    Result.StatusStr = this->Status;
    Result.State = (EOnlinePresenceState::Type)this->State;
    for (auto Prop : this->Properties)
    {
        Result.Properties.Add(Prop.Key, Prop.Value);
    }
    return Result;
}

FOnlineUserPresenceData FOnlineUserPresenceData::FromNative(TSharedPtr<FOnlineUserPresence> InObj)
{
    FOnlineUserPresenceData Data;
    if (InObj == nullptr)
    {
        return Data;
    }

    Data.SessionId = FUniqueNetIdRepl(InObj->SessionId);
    Data.IsOnline = InObj->bIsOnline;
    Data.IsPlaying = InObj->bIsPlaying;
    Data.IsPlayingThisGame = InObj->bIsPlayingThisGame;
    Data.IsJoinable = InObj->bIsJoinable;
    Data.HasVoiceSupport = InObj->bHasVoiceSupport;
#if defined(UE_4_25_OR_LATER)
    Data.LastOnline = InObj->LastOnline;
#endif
    Data.Status = FOnlineUserPresenceStatusData::FromNative(InObj->Status);
    return Data;
}

FOnlineUserPresenceData FOnlineUserPresenceData::FromNative(const FOnlineUserPresence &InObj)
{
    FOnlineUserPresenceData Data;
    Data.SessionId = FUniqueNetIdRepl(InObj.SessionId);
    Data.IsOnline = InObj.bIsOnline;
    Data.IsPlaying = InObj.bIsPlaying;
    Data.IsPlayingThisGame = InObj.bIsPlayingThisGame;
    Data.IsJoinable = InObj.bIsJoinable;
    Data.HasVoiceSupport = InObj.bHasVoiceSupport;
#if defined(UE_4_25_OR_LATER)
    Data.LastOnline = InObj.LastOnline;
#endif
    Data.Status = FOnlineUserPresenceStatusData::FromNative(InObj.Status);
    return Data;
}

FOnlineUserPresence FOnlineUserPresenceData::ToNative() const
{
    FOnlineUserPresence Presence;
    Presence.SessionId = this->SessionId.GetUniqueNetId();
    Presence.bIsOnline = this->IsOnline;
    Presence.bIsPlaying = this->IsPlaying;
    Presence.bIsPlayingThisGame = this->IsPlayingThisGame;
    Presence.bIsJoinable = this->IsJoinable;
    Presence.bHasVoiceSupport = this->HasVoiceSupport;
#if defined(UE_4_25_OR_LATER)
    Presence.LastOnline = this->LastOnline;
#endif
    Presence.Status = this->Status.ToNative();
    return Presence;
}