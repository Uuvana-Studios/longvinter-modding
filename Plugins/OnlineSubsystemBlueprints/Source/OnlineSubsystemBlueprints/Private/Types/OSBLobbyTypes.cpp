// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBLobbyTypes.h"

#include "Helpers/UniqueNetIdConversion.h"

ULobbyId *ULobbyId::FromNative(const FOnlineLobbyId &InObj)
{
    auto Ref = NewObject<ULobbyId>();
    Ref->LobbyId = TSharedPtr<const FOnlineLobbyId>(InObj.AsShared());
    return Ref;
}

ULobbyId *ULobbyId::FromNative(const TSharedPtr<const FOnlineLobbyId> &InObj)
{
    auto Ref = NewObject<ULobbyId>();
    Ref->LobbyId = InObj;
    return Ref;
}

ULobbyId *ULobby::GetId()
{
    if (!this->Lobby.IsValid())
    {
        return nullptr;
    }

    return ULobbyId::FromNative(this->Lobby->Id);
}

FUniqueNetIdRepl ULobby::GetOwnerId()
{
    if (!this->Lobby.IsValid())
    {
        return FUniqueNetIdRepl();
    }

    return FUniqueNetIdRepl(this->Lobby->OwnerId);
}

ULobby *ULobby::FromNative(TSharedPtr<FOnlineLobby> InObj)
{
    auto Ref = NewObject<ULobby>();
    Ref->Lobby = InObj;
    return Ref;
}

void UOnlineLobbyTransaction::SetMetadata(const FString &Key, const FVariantDataBP &Value)
{
    this->Txn->SetMetadata.Add(Key, Value.ToNative());
}

void UOnlineLobbyTransaction::SetMetadataByMap(const TMap<FString, FVariantDataBP> &Metadata)
{
    for (auto KV : Metadata)
    {
        this->Txn->SetMetadata.Add(KV.Key, KV.Value.ToNative());
    }
}

void UOnlineLobbyTransaction::DeleteMetadata(const FString &Key)
{
    this->Txn->DeleteMetadata.Add(Key);
}

void UOnlineLobbyTransaction::DeleteMetadataByArray(const TArray<FString> &MetadataKeys)
{
    for (auto Key : MetadataKeys)
    {
        this->Txn->DeleteMetadata.Add(Key);
    }
}

void UOnlineLobbyTransaction::SetLocked(bool Locked)
{
    this->Txn->Locked = Locked;
}

void UOnlineLobbyTransaction::SetCapacity(int64 Capacity)
{
    this->Txn->Capacity = Capacity;
}

void UOnlineLobbyTransaction::SetPublic(bool Public)
{
    this->Txn->Public = Public;
}

UOnlineLobbyTransaction *UOnlineLobbyTransaction::FromNative(TSharedPtr<FOnlineLobbyTransaction> InObj)
{
    auto Ref = NewObject<UOnlineLobbyTransaction>();
    Ref->Txn = InObj;
    return Ref;
}

void UOnlineLobbyMemberTransaction::SetMetadata(const FString &Key, const FVariantDataBP &Value)
{
    this->Txn->SetMetadata.Add(Key, Value.ToNative());
}

void UOnlineLobbyMemberTransaction::SetMetadataByMap(const TMap<FString, FVariantDataBP> &Metadata)
{
    for (auto KV : Metadata)
    {
        this->Txn->SetMetadata.Add(KV.Key, KV.Value.ToNative());
    }
}

void UOnlineLobbyMemberTransaction::DeleteMetadata(const FString &Key)
{
    this->Txn->DeleteMetadata.Add(Key);
}

void UOnlineLobbyMemberTransaction::DeleteMetadataByArray(const TArray<FString> &MetadataKeys)
{
    for (auto Key : MetadataKeys)
    {
        this->Txn->DeleteMetadata.Add(Key);
    }
}

UOnlineLobbyMemberTransaction *UOnlineLobbyMemberTransaction::FromNative(
    TSharedPtr<FOnlineLobbyMemberTransaction> InObj)
{
    auto Ref = NewObject<UOnlineLobbyMemberTransaction>();
    Ref->Txn = InObj;
    return Ref;
}

FOnlineLobbySearchQueryFilterBP FOnlineLobbySearchQueryFilterBP::FromNative(const FOnlineLobbySearchQueryFilter &InObj)
{
    FOnlineLobbySearchQueryFilterBP Result = {};
    Result.Comparison = (EOnlineLobbySearchQueryFilterComparator_)InObj.Comparison;
    Result.Key = InObj.Key;
    Result.Value = FVariantDataBP::FromNative(InObj.Value);
    return Result;
}

FOnlineLobbySearchQueryFilter FOnlineLobbySearchQueryFilterBP::ToNative() const
{
    return FOnlineLobbySearchQueryFilter(
        this->Key,
        this->Value.ToNative(),
        (EOnlineLobbySearchQueryFilterComparator)this->Comparison);
}

FOnlineLobbySearchQueryBP FOnlineLobbySearchQueryBP::FromNative(const FOnlineLobbySearchQuery &InObj)
{
    FOnlineLobbySearchQueryBP Result = {};
    for (auto Filter : InObj.Filters)
    {
        Result.Filters.Add(FOnlineLobbySearchQueryFilterBP::FromNative(Filter));
    }
    Result.HasLimit = InObj.Limit.IsSet();
    Result.Limit = InObj.Limit.Get(0);
    return Result;
}

FOnlineLobbySearchQuery FOnlineLobbySearchQueryBP::ToNative() const
{
    FOnlineLobbySearchQuery Result = {};
    for (auto Filter : this->Filters)
    {
        Result.Filters.Add(Filter.ToNative());
    }
    if (this->HasLimit)
    {
        Result.Limit = this->Limit;
    }
    return Result;
}