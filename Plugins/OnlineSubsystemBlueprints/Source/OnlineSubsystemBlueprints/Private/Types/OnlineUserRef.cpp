// Copyright June Rhodes. All Rights Reserved.

#include "Types/OnlineUserRef.h"

void UOnlineUserRef::AssignFromUser(const TSharedPtr<FOnlineUser> &InUser)
{
    this->User = InUser;
}

void UOnlineUserRef::AssignFromBlockedPlayer(const TSharedPtr<FOnlineBlockedPlayer> &InBlockedPlayer)
{
    this->User = InBlockedPlayer;
    this->BlockedPlayer = InBlockedPlayer;
}

UOnlineUserRef *UOnlineUserRef::FromUser(const TSharedPtr<FOnlineUser> &User)
{
    auto Ref = NewObject<UOnlineUserRef>();
    Ref->AssignFromUser(User);
    return Ref;
}

UOnlineUserRef *UOnlineUserRef::FromBlockedPlayer(const TSharedPtr<FOnlineBlockedPlayer> &BlockedPlayer)
{
    auto Ref = NewObject<UOnlineUserRef>();
    Ref->AssignFromBlockedPlayer(BlockedPlayer);
    return Ref;
}

FUniqueNetIdRepl UOnlineUserRef::GetUserId_Implementation()
{
    return this->GetUserId_Native();
}

FString UOnlineUserRef::GetRealName_Implementation()
{
    return this->GetRealName_Native();
}

FString UOnlineUserRef::GetDisplayName_Implementation()
{
    return this->GetDisplayName_Native();
}

FString UOnlineUserRef::GetUserAttribute_Implementation(const FString &Key, bool &Found)
{
    return this->GetUserAttribute_Native(Key, Found);
}

void UOnlineUserRef::SetUserLocalAttribute_Implementation(const FString &Key, const FString &Value, bool &Success)
{
    this->SetUserLocalAttribute_Native(Key, Value, Success);
}

FUniqueNetIdRepl UOnlineUserRef::GetUserId_Native()
{
    if (!this->User.IsValid())
    {
        return FUniqueNetIdRepl();
    }

    return FUniqueNetIdRepl(this->User->GetUserId());
}

FString UOnlineUserRef::GetRealName_Native()
{
    if (!this->User.IsValid())
    {
        return TEXT("");
    }

    return this->User->GetRealName();
}

FString UOnlineUserRef::GetDisplayName_Native()
{
    if (!this->User.IsValid())
    {
        return TEXT("");
    }

    return this->User->GetDisplayName();
}

FString UOnlineUserRef::GetUserAttribute_Native(const FString &Key, bool &Found)
{
    if (!this->User.IsValid())
    {
        return TEXT("");
    }

    FString Out;
    Found = this->User->GetUserAttribute(Key, Out);
    return Out;
}

void UOnlineUserRef::SetUserLocalAttribute_Native(const FString &Key, const FString &Value, bool &Success)
{
    if (!this->User.IsValid())
    {
        Success = false;
        return;
    }

    Success = this->User->SetUserLocalAttribute(Key, Value);
}

FExternalIdQueryOptionsBP FExternalIdQueryOptionsBP::FromNative(const FExternalIdQueryOptions &Obj)
{
    FExternalIdQueryOptionsBP Opts;
    Opts.AuthType = Obj.AuthType;
    Opts.bLookupByDisplayName = Obj.bLookupByDisplayName;
    return Opts;
}

FExternalIdQueryOptions FExternalIdQueryOptionsBP::ToNative()
{
    return FExternalIdQueryOptions(this->AuthType, this->bLookupByDisplayName);
}

UUserOnlineAccountRef *UUserOnlineAccountRef::FromUserOnlineAccount(const TSharedPtr<FUserOnlineAccount> &User)
{
    auto Ref = NewObject<UUserOnlineAccountRef>();
    Ref->AssignFromUserOnlineAccount(User);
    return Ref;
}

void UUserOnlineAccountRef::AssignFromUserOnlineAccount(const TSharedPtr<FUserOnlineAccount> &InUser)
{
    this->AssignFromUser(InUser);
    this->UserOnlineAccount = InUser;
}

FString UUserOnlineAccountRef::GetAccessToken()
{
    if (!this->UserOnlineAccount.IsValid())
    {
        return TEXT("");
    }

    return this->UserOnlineAccount->GetAccessToken();
}

FString UUserOnlineAccountRef::GetAuthAttribute(FString Key, bool &Found)
{
    if (!this->UserOnlineAccount.IsValid())
    {
        Found = false;
        return TEXT("");
    }

    FString OutValue;
    if (this->UserOnlineAccount->GetAuthAttribute(Key, OutValue))
    {
        Found = true;
        return OutValue;
    }

    Found = false;
    return TEXT("");
}

bool UUserOnlineAccountRef::SetUserAttribute(FString Key, FString Value)
{
    if (!this->UserOnlineAccount.IsValid())
    {
        return false;
    }

    return this->UserOnlineAccount->SetUserAttribute(Key, Value);
}