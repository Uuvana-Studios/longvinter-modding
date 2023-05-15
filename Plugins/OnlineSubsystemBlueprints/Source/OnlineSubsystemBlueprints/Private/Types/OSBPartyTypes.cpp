// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBPartyTypes.h"

#include "Helpers/UniqueNetIdConversion.h"

UPartyId *UPartyId::FromNative(const FOnlinePartyId &InObj)
{
    auto Ref = NewObject<UPartyId>();
    Ref->PartyId = TSharedPtr<const FOnlinePartyId>(InObj.AsShared());
    return Ref;
}

UPartyId *UPartyId::FromNative(const TSharedPtr<const FOnlinePartyId> &InObj)
{
    auto Ref = NewObject<UPartyId>();
    Ref->PartyId = InObj;
    return Ref;
}

UParty *UParty::FromNative(FOnlinePartyConstPtr InObj)
{
    auto Ref = NewObject<UParty>();
    Ref->Party = InObj;
    return Ref;
}

UPartyId *UParty::GetPartyId()
{
    if (auto PinnedParty = this->Party.Pin())
    {
        if (PinnedParty.IsValid())
        {
            return UPartyId::FromNative(PinnedParty->PartyId);
        }
    }

    return nullptr;
}

int64 UParty::GetPartyTypeId()
{
    if (auto PinnedParty = this->Party.Pin())
    {
        if (PinnedParty.IsValid())
        {
            return PinnedParty->PartyTypeId.GetValue();
        }
    }

    return 0;
}

FUniqueNetIdRepl UParty::GetLeaderId()
{
    if (auto PinnedParty = this->Party.Pin())
    {
        if (PinnedParty.IsValid())
        {
            if (PinnedParty->LeaderId.IsValid())
            {
                return ConvertDangerousUniqueNetIdToRepl(*PinnedParty->LeaderId);
            }
        }
    }

    return FUniqueNetIdRepl();
}

void UBlueprintPartyMember::BeginDestroy()
{
    if (this->PartyMember.IsValid())
    {
        this->PartyMember->OnMemberAttributeChanged().Remove(this->AttributeHandle);
        this->PartyMember->OnMemberConnectionStatusChanged().Remove(this->ConnectionStatusHandle);
        this->PartyMember = nullptr;
    }

    Super::BeginDestroy();
}

UBlueprintPartyMember *UBlueprintPartyMember::FromNative(FOnlinePartyMemberConstPtr InObj)
{
    auto Ref = NewObject<UBlueprintPartyMember>();
    Ref->PartyMember = InObj;

    Ref->AttributeHandle =
        Ref->PartyMember->OnMemberAttributeChanged().AddUObject(Ref, &UBlueprintPartyMember::OnMemberAttributeChanged);
    Ref->ConnectionStatusHandle = Ref->PartyMember->OnMemberConnectionStatusChanged().AddUObject(
        Ref,
        &UBlueprintPartyMember::OnMemberConnectionStatusChanged);

    return Ref;
}

void UBlueprintPartyMember::OnMemberAttributeChanged(
    const FUniqueNetId &ChangedUserId,
    const FString &Attribute,
    const FString &NewValue,
    const FString &OldValue)
{
    this->OnAttributeChanged.Broadcast(ConvertDangerousUniqueNetIdToRepl(ChangedUserId), Attribute, NewValue, OldValue);
}

void UBlueprintPartyMember::OnMemberConnectionStatusChanged(
    const FUniqueNetId &ChangedUserId,
    const EMemberConnectionStatus NewMemberConnectionStatus,
    const EMemberConnectionStatus PreviousMemberConnectionStatus)
{
    this->OnConnectionStatusChanged.Broadcast(
        ConvertDangerousUniqueNetIdToRepl(ChangedUserId),
        (EMemberConnectionStatus_)NewMemberConnectionStatus,
        (EMemberConnectionStatus_)PreviousMemberConnectionStatus);
}

FUniqueNetIdRepl UBlueprintPartyMember::GetUserId()
{
    if (this->IsValid())
    {
        return ConvertDangerousUniqueNetIdToRepl(this->PartyMember->GetUserId().Get());
    }

    return FUniqueNetIdRepl();
}

FString UBlueprintPartyMember::GetRealName()
{
    if (this->IsValid())
    {
        return this->PartyMember->GetRealName();
    }

    return TEXT("");
}

FString UBlueprintPartyMember::GetDisplayName(const FString &Platform)
{
    if (this->IsValid())
    {
        return this->PartyMember->GetDisplayName(Platform);
    }

    return TEXT("");
}

bool UBlueprintPartyMember::GetUserAttribute(const FString &AttrName, FString &OutAttrValue)
{
    if (this->IsValid())
    {
        return this->PartyMember->GetUserAttribute(AttrName, OutAttrValue);
    }

    return false;
}

void UReadablePartyData::AssignFromConstPartyData(const FOnlinePartyData &InObj)
{
    this->ConstPartyData = InObj.AsShared();
}

UReadablePartyData *UReadablePartyData::FromNative(const FOnlinePartyData &InObj)
{
    auto Ref = NewObject<UReadablePartyData>();
    Ref->AssignFromConstPartyData(InObj);
    return Ref;
}

UReadablePartyData *UReadablePartyData::FromNative(TSharedPtr<const FOnlinePartyData> InObj)
{
    if (!InObj.IsValid())
    {
        return nullptr;
    }

    return UReadablePartyData::FromNative(*InObj);
}

void UReadablePartyData::GetAttribute(const FString &AttrName, bool &OutFound, FVariantDataBP &OutAttrValue) const
{
    FVariantData OutValue;
    if (this->ConstPartyData->GetAttribute(AttrName, OutValue))
    {
        OutFound = true;
        OutAttrValue = FVariantDataBP::FromNative(OutValue);
        return;
    }

    OutFound = false;
    OutAttrValue = FVariantDataBP();
}

UMutablePartyData *UMutablePartyData::FromNative(FOnlinePartyData &InObj)
{
    auto Ref = NewObject<UMutablePartyData>();
    Ref->AssignFromConstPartyData(InObj);
    Ref->MutablePartyData = InObj.AsShared();
    return Ref;
}

UMutablePartyData *UMutablePartyData::FromNative(TSharedRef<FOnlinePartyData> InObj)
{
    auto Ref = NewObject<UMutablePartyData>();
    Ref->AssignFromConstPartyData(*InObj);
    Ref->MutablePartyData = InObj;
    return Ref;
}

UMutablePartyData *UMutablePartyData::FromNative(TSharedPtr<FOnlinePartyData> InObj)
{
    if (!InObj.IsValid())
    {
        return nullptr;
    }

    return UMutablePartyData::FromNative(*InObj);
}

void UMutablePartyData::SetAttribute(const FString &AttrName, FVariantDataBP AttrValue)
{
    this->MutablePartyData->SetAttribute(AttrName, AttrValue.ToNative());
}

void UMutablePartyData::RemoveAttribute(const FString &AttrName)
{
    this->MutablePartyData->RemoveAttribute(AttrName);
}

UOnlinePartyJoinInfo *UOnlinePartyJoinInfo::FromNative(const IOnlinePartyJoinInfo &InObj)
{
    auto Ref = NewObject<UOnlinePartyJoinInfo>();
    Ref->OnlinePartyJoinInfo = TSharedRef<const IOnlinePartyJoinInfo>(InObj.AsShared());
    return Ref;
}

FOnlinePartyConfiguration FOnlinePartyConfiguration::FromNative(const FPartyConfiguration &Config)
{
    FOnlinePartyConfiguration R;
    R.JoinRequestAction = (EJoinRequestAction_)Config.JoinRequestAction;
    R.PresencePermissions = (EPartySystemPermissions)Config.PresencePermissions;
    R.InvitePermissions = (EPartySystemPermissions)Config.InvitePermissions;
    R.ChatEnabled = Config.bChatEnabled;
    R.ShouldRemoveOnDisconnection = Config.bShouldRemoveOnDisconnection;
    R.IsAcceptingMembers = Config.bIsAcceptingMembers;
    R.NotAcceptingMembersReason = Config.NotAcceptingMembersReason;
    R.MaxMembers = Config.MaxMembers;
    R.Nickname = Config.Nickname;
    R.Description = Config.Description;
    R.Password = Config.Password;
    return R;
}

TSharedPtr<FPartyConfiguration> FOnlinePartyConfiguration::ToNative()
{
    auto R = MakeShared<FPartyConfiguration>();
    R->JoinRequestAction = (EJoinRequestAction)this->JoinRequestAction;
    R->PresencePermissions = (PartySystemPermissions::EPermissionType)this->PresencePermissions;
    R->InvitePermissions = (PartySystemPermissions::EPermissionType)this->InvitePermissions;
    R->bChatEnabled = this->ChatEnabled;
    R->bShouldRemoveOnDisconnection = this->ShouldRemoveOnDisconnection;
    R->bIsAcceptingMembers = this->IsAcceptingMembers;
    R->NotAcceptingMembersReason = this->NotAcceptingMembersReason;
    R->MaxMembers = this->MaxMembers;
    R->Nickname = this->Nickname;
    R->Description = this->Description;
    R->Password = this->Password;
    return R;
}

UPartyId *UOnlinePartyJoinInfo::GetPartyId()
{
    if (!this->IsValid())
    {
        return nullptr;
    }

    return UPartyId::FromNative(this->OnlinePartyJoinInfo->GetPartyId());
}

FString UOnlinePartyJoinInfo::GetSourceDisplayName()
{
    if (!this->IsValid())
    {
        return TEXT("");
    }

    return this->OnlinePartyJoinInfo->GetSourceDisplayName();
}

FUniqueNetIdRepl UOnlinePartyJoinInfo::GetSourceUserId()
{
    if (!this->IsValid())
    {
        return FUniqueNetIdRepl();
    }

    return ConvertDangerousUniqueNetIdToRepl(this->OnlinePartyJoinInfo->GetSourceUserId().Get());
}

FString UOnlinePartyJoinInfo::ToDebugString()
{
    if (!this->IsValid())
    {
        return TEXT("");
    }

#if defined(UE_4_25_OR_LATER)
    return this->OnlinePartyJoinInfo->ToDebugString();
#else
    return TEXT("");
#endif
}