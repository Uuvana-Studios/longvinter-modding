// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBStatsTypes.h"

FOnlineStatsUserStatsBP FOnlineStatsUserStatsBP::FromNative(const FOnlineStatsUserStats &InObj)
{
    FOnlineStatsUserStatsBP Result;
    Result.PlayerId = FUniqueNetIdRepl(InObj.Account);
    for (auto KV : InObj.Stats)
    {
        Result.Stats.Add(KV.Key, FVariantDataBP::FromNative(KV.Value));
    }
    return Result;
}

FOnlineStatsUserStatsBP FOnlineStatsUserStatsBP::FromNative(const TSharedRef<const FOnlineStatsUserStats> InObj)
{
    return FOnlineStatsUserStatsBP::FromNative(*InObj);
}

FOnlineStatsUserStatsBP FOnlineStatsUserStatsBP::FromNative(const TSharedPtr<const FOnlineStatsUserStats> InObj)
{
    return FOnlineStatsUserStatsBP::FromNative(*InObj);
}

FOnlineStatsUserStats FOnlineStatsUserStatsBP::ToNative() const
{
    TMap<FString, FOnlineStatValue> ConvertedMap;
    for (auto KV : this->Stats)
    {
        ConvertedMap.Add(KV.Key, KV.Value.ToNative());
    }
    return FOnlineStatsUserStats(this->PlayerId.GetUniqueNetId().ToSharedRef(), ConvertedMap);
}

FOnlineStatsUserUpdatedStatsBP FOnlineStatsUserUpdatedStatsBP::FromNative(const FOnlineStatsUserUpdatedStats &InObj)
{
    FOnlineStatsUserUpdatedStatsBP Result;
    Result.PlayerId = FUniqueNetIdRepl(InObj.Account);
    for (auto KV : InObj.Stats)
    {
        FOnlineStatUpdateBP Value;
        Value.Type = (EOnlineStatModificationType_)KV.Value.GetModificationType();
        Value.Value = FVariantDataBP::FromNative(KV.Value.GetValue());
        Result.Stats.Add(KV.Key, Value);
    }
    return Result;
}

FOnlineStatsUserUpdatedStats FOnlineStatsUserUpdatedStatsBP::ToNative() const
{
    TMap<FString, FOnlineStatUpdate> ConvertedMap;
    for (auto KV : this->Stats)
    {
        ConvertedMap.Add(
            KV.Key,
            FOnlineStatUpdate(
                KV.Value.Value.ToNative(),
                (FOnlineStatUpdate::EOnlineStatModificationType)KV.Value.Type));
    }
    return FOnlineStatsUserUpdatedStats(this->PlayerId.GetUniqueNetId().ToSharedRef(), ConvertedMap);
}