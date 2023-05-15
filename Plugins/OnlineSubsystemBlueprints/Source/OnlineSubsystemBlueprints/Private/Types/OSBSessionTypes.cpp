// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBSessionTypes.h"

UOnlineSessionInfo *UOnlineSessionInfo::FromNative(TSharedPtr<class FOnlineSessionInfo> InSessionInfo)
{
    if (!InSessionInfo.IsValid())
    {
        return nullptr;
    }

    auto Ref = NewObject<UOnlineSessionInfo>();
    Ref->SessionInfo = InSessionInfo;
    return Ref;
}

FVariantDataBP FVariantDataBP::FromNative(const FVariantData &InObj)
{
    FVariantDataBP Result;
    Result.Type = (EOnlineKeyValuePairDataType_)InObj.GetType();
    switch (Result.Type)
    {
    case EOnlineKeyValuePairDataType_::Int32:
        InObj.GetValue(Result.AsInt);
        break;
    case EOnlineKeyValuePairDataType_::Int64:
        InObj.GetValue(Result.AsInt64);
        break;
    case EOnlineKeyValuePairDataType_::String:
        InObj.GetValue(Result.AsString);
        break;
    case EOnlineKeyValuePairDataType_::Float:
        InObj.GetValue(Result.AsFloat);
        break;
    case EOnlineKeyValuePairDataType_::Bool:
        InObj.GetValue(Result.AsBool);
        break;
    case EOnlineKeyValuePairDataType_::Empty:
    case EOnlineKeyValuePairDataType_::Json_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt32_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt64_NotSupported:
    case EOnlineKeyValuePairDataType_::Double_NotSupported:
    case EOnlineKeyValuePairDataType_::Blob_NotSupported:
    default:
        break;
    }
    return Result;
}

FVariantData FVariantDataBP::ToNative() const
{
    FVariantData Result;
    switch (this->Type)
    {
    case EOnlineKeyValuePairDataType_::Int32:
        Result.SetValue(this->AsInt);
        break;
    case EOnlineKeyValuePairDataType_::Int64:
        Result.SetValue(this->AsInt64);
        break;
    case EOnlineKeyValuePairDataType_::String:
        Result.SetValue(this->AsString);
        break;
    case EOnlineKeyValuePairDataType_::Float:
        Result.SetValue(this->AsFloat);
        break;
    case EOnlineKeyValuePairDataType_::Bool:
        Result.SetValue(this->AsBool);
        break;
    case EOnlineKeyValuePairDataType_::Empty:
    case EOnlineKeyValuePairDataType_::Json_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt32_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt64_NotSupported:
    case EOnlineKeyValuePairDataType_::Double_NotSupported:
    case EOnlineKeyValuePairDataType_::Blob_NotSupported:
    default:
        Result.Empty();
        break;
    }
    return Result;
}

FOnlineSessionSettingBP FOnlineSessionSettingBP::FromNative(const FOnlineSessionSetting &InObj)
{
    FOnlineSessionSettingBP Result;
    Result.Data = FVariantDataBP::FromNative(InObj.Data);
    Result.AdvertisementType = (EOnlineDataAdvertisementType_)InObj.AdvertisementType;
    Result.ID = InObj.ID;
    return Result;
}

FOnlineSessionSetting FOnlineSessionSettingBP::ToNative()
{
    FOnlineSessionSetting Result;
    Result.Data = this->Data.ToNative();
    Result.AdvertisementType = (EOnlineDataAdvertisementType::Type)this->AdvertisementType;
    Result.ID = this->ID;
    return Result;
}

FOnlineSessionSettingsBP FOnlineSessionSettingsBP::FromNative(const FOnlineSessionSettings &InObj)
{
    FOnlineSessionSettingsBP Result;
    Result.NumPublicConnections = InObj.NumPublicConnections;
    Result.NumPrivateConnections = InObj.NumPrivateConnections;
    Result.bShouldAdvertise = InObj.bShouldAdvertise;
    Result.bAllowJoinInProgress = InObj.bAllowJoinInProgress;
    Result.bIsLANMatch = InObj.bIsLANMatch;
    Result.bIsDedicated = InObj.bIsDedicated;
    Result.bUsesStats = InObj.bUsesStats;
    Result.bAllowInvites = InObj.bAllowInvites;
    Result.bUsesPresence = InObj.bUsesPresence;
    Result.bAllowJoinViaPresence = InObj.bAllowJoinViaPresence;
    Result.bAllowJoinViaPresenceFriendsOnly = InObj.bAllowJoinViaPresenceFriendsOnly;
    Result.bAntiCheatProtected = InObj.bAntiCheatProtected;
    Result.BuildUniqueId = InObj.BuildUniqueId;
    for (auto KV : InObj.Settings)
    {
        Result.Settings.Add(KV.Key, FOnlineSessionSettingBP::FromNative(KV.Value));
    }
    return Result;
}

FOnlineSessionSettings FOnlineSessionSettingsBP::ToNative()
{
    FOnlineSessionSettings Result;
    Result.NumPublicConnections = this->NumPublicConnections;
    Result.NumPrivateConnections = this->NumPrivateConnections;
    Result.bShouldAdvertise = this->bShouldAdvertise;
    Result.bAllowJoinInProgress = this->bAllowJoinInProgress;
    Result.bIsLANMatch = this->bIsLANMatch;
    Result.bIsDedicated = this->bIsDedicated;
    Result.bUsesStats = this->bUsesStats;
    Result.bAllowInvites = this->bAllowInvites;
    Result.bUsesPresence = this->bUsesPresence;
    Result.bAllowJoinViaPresence = this->bAllowJoinViaPresence;
    Result.bAllowJoinViaPresenceFriendsOnly = this->bAllowJoinViaPresenceFriendsOnly;
    Result.bAntiCheatProtected = this->bAntiCheatProtected;
    Result.BuildUniqueId = this->BuildUniqueId;
    for (auto KV : this->Settings)
    {
        Result.Settings.Add(KV.Key, KV.Value.ToNative());
    }
    return Result;
}

FOnlineSessionSettings &FOnlineSessionSettingsBP::ToNativeRef()
{
    this->RefTemp = this->ToNative();
    return this->RefTemp;
}

FOnlineSessionBP FOnlineSessionBP::FromNative(const FOnlineSession &InObj)
{
    FOnlineSessionBP Result;
    Result.OwningUserId = FUniqueNetIdRepl(InObj.OwningUserId);
    Result.OwningUserName = InObj.OwningUserName;
    Result.SessionSettings = FOnlineSessionSettingsBP::FromNative(InObj.SessionSettings);
    Result.SessionInfo = UOnlineSessionInfo::FromNative(InObj.SessionInfo);
    Result.NumOpenPrivateConnections = InObj.NumOpenPrivateConnections;
    Result.NumOpenPublicConnections = InObj.NumOpenPublicConnections;
    Result.SessionId = InObj.GetSessionIdStr();
    return Result;
}

FNamedOnlineSessionBP FNamedOnlineSessionBP::FromNative(const FNamedOnlineSession &InObj)
{
    FNamedOnlineSessionBP Result;
    Result.SessionName = InObj.SessionName;
    Result.bHosting = InObj.bHosting;
    Result.LocalOwnerId = FUniqueNetIdRepl(InObj.LocalOwnerId);
    for (int i = 0; i < InObj.RegisteredPlayers.Num(); i++)
    {
        Result.RegisteredPlayers.Add(FUniqueNetIdRepl(InObj.RegisteredPlayers[i]));
    }
    Result.SessionState = (EOnlineSessionState_)InObj.SessionState;
    Result.OwningUserId = FUniqueNetIdRepl(InObj.OwningUserId);
    Result.OwningUserName = InObj.OwningUserName;
    Result.SessionSettings = FOnlineSessionSettingsBP::FromNative(InObj.SessionSettings);
    Result.SessionInfo = UOnlineSessionInfo::FromNative(InObj.SessionInfo);
    Result.NumOpenPrivateConnections = InObj.NumOpenPrivateConnections;
    Result.NumOpenPublicConnections = InObj.NumOpenPublicConnections;
    Result.SessionId = InObj.GetSessionIdStr();
    return Result;
}

FOnlineSession FOnlineSessionBP::ToNative()
{
    FOnlineSession Result;
    Result.OwningUserId = this->OwningUserId.GetUniqueNetId();
    Result.OwningUserName = this->OwningUserName;
    Result.SessionSettings = this->SessionSettings.ToNative();
    Result.SessionInfo = this->SessionInfo == nullptr ? nullptr : this->SessionInfo->ToNative();
    Result.NumOpenPrivateConnections = this->NumOpenPrivateConnections;
    Result.NumOpenPublicConnections = this->NumOpenPublicConnections;
    return Result;
}

FOnlineSessionSearchResultBP FOnlineSessionSearchResultBP::FromNative(const FOnlineSessionSearchResult &InObj)
{
    FOnlineSessionSearchResultBP Result;
    Result.Session = FOnlineSessionBP::FromNative(InObj.Session);
    Result.PingInMs = InObj.PingInMs;
    return Result;
}

FOnlineSessionSearchResult FOnlineSessionSearchResultBP::ToNative()
{
    FOnlineSessionSearchResult Result;
    Result.Session = this->Session.ToNative();
    Result.PingInMs = this->PingInMs;
    return Result;
}

UNamedOnlineSession *UNamedOnlineSession::FromNative(class FNamedOnlineSession *InSession)
{
    if (InSession == nullptr)
    {
        return nullptr;
    }
    else
    {
        auto Ref = NewObject<UNamedOnlineSession>();
        Ref->Value = FNamedOnlineSessionBP::FromNative(*InSession);
        return Ref;
    }
}

UOnlineSessionSettings *UOnlineSessionSettings::FromNative(class FOnlineSessionSettings *InSession)
{
    if (InSession == nullptr)
    {
        return nullptr;
    }
    else
    {
        auto Ref = NewObject<UOnlineSessionSettings>();
        Ref->Value = FOnlineSessionSettingsBP::FromNative(*InSession);
        return Ref;
    }
}

void UOnlineSessionSearch::PostInitProperties()
{
    Super::PostInitProperties();

    if (!this->Search.IsValid())
    {
        this->Search = MakeShared<FOnlineSessionSearch>();
        this->Search->QuerySettings.SearchParams.Reset();
        this->SyncPropertiesFromNative();
    }
}

void UOnlineSessionSearch::SyncPropertiesFromNative()
{
    this->MaxSearchResults = this->Search->MaxSearchResults;
    this->bIsLanQuery = this->Search->bIsLanQuery;
    this->PingBucketSize = this->Search->PingBucketSize;
    this->PlatformHash = this->Search->PlatformHash;
    this->TimeoutInSeconds = this->Search->TimeoutInSeconds;
    this->SearchParams.Reset();
    for (auto SearchParam : this->Search->QuerySettings.SearchParams)
    {
        FSessionSearchParamBP Param;
        Param.Data = FVariantDataBP::FromNative(SearchParam.Value.Data);
        Param.Op = (EOnlineComparisonOp_)SearchParam.Value.ComparisonOp;
        Param.ID = SearchParam.Value.ID;
        this->SearchParams.Add(SearchParam.Key, Param);
    }
}

void UOnlineSessionSearch::SyncPropertiesToNative()
{
    this->Search->MaxSearchResults = this->MaxSearchResults;
    this->Search->bIsLanQuery = this->bIsLanQuery;
    this->Search->PingBucketSize = this->PingBucketSize;
    this->Search->PlatformHash = this->PlatformHash;
    this->Search->TimeoutInSeconds = this->TimeoutInSeconds;
    this->Search->QuerySettings.SearchParams.Reset();
    for (auto SearchParam : this->SearchParams)
    {
        FOnlineSessionSearchParam Param(TEXT(""));
        Param.Data = SearchParam.Value.Data.ToNative();
        Param.ComparisonOp = (EOnlineComparisonOp::Type)SearchParam.Value.Op;
        Param.ID = SearchParam.Value.ID;
        this->Search->QuerySettings.SearchParams.Add(SearchParam.Key, Param);
    }
}

TArray<FOnlineSessionSearchResultBP> UOnlineSessionSearch::GetSearchResults() const
{
    TArray<FOnlineSessionSearchResultBP> Results;
    for (auto Result : this->Search->SearchResults)
    {
        Results.Add(FOnlineSessionSearchResultBP::FromNative(Result));
    }
    return Results;
}

EOnlineAsyncTaskState_ UOnlineSessionSearch::GetSearchState() const
{
    return (EOnlineAsyncTaskState_)this->Search->SearchState;
}

TSharedRef<class FOnlineSessionSearch> UOnlineSessionSearch::ToNative()
{
    this->SyncPropertiesToNative();
    return this->Search.ToSharedRef();
}

TSharedRef<class FOnlineSessionSearch> &UOnlineSessionSearch::ToNativeRef()
{
    this->SyncPropertiesToNative();
    if (!this->SearchTemp.IsValid())
    {
        SearchTemp = MakeShared<TSharedRef<class FOnlineSessionSearch>>(this->Search.ToSharedRef());
    }
    return *this->SearchTemp.Get();
}

bool UOnlineSessionSearch::IsValid() const
{
    return this->Search.IsValid();
}