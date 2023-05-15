// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlinePartySubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> UOnlinePartySubsystem::GetHandle()
{
    if (this->HasAnyFlags(RF_BeginDestroyed))
    {
        auto BDCheck_GameInstance = this->GetGameInstance();
        check(IsValid(BDCheck_GameInstance));
    
        auto BDCheck_World = BDCheck_GameInstance->GetWorld();
        auto BDCheck_Pinned = this->HandleWk.Pin();

        if ((BDCheck_Pinned == nullptr || !BDCheck_Pinned.IsValid()) && !IsValid(BDCheck_World))
        {
            // The world and associated online subsystem interfaces have already been cleaned up.
            // The only caller of GetHandle() in this context is event deregistration, which doesn't
            // need to be done if the online subsystem itself is already gone.
            return nullptr;
        }
    }

    auto GameInstance = this->GetGameInstance();
    check(IsValid(GameInstance));

    auto World = GameInstance->GetWorld();
    if (World == nullptr)
    {
        // The world has already been freed; this GetHandle call might be happening in a callback
        // (where it is already handled as a "CallFailed" if GetHandle returns a nullptr).
        return nullptr;
    }

    auto CurrentWorldContext = GEngine->GetWorldContextFromWorld(World);
    check(CurrentWorldContext != nullptr);
    check(CurrentWorldContext->ContextHandle.IsEqual(this->WorldContext->ContextHandle));

    TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
    if (Pinned == nullptr || !Pinned.IsValid())
    {
        // Even though Online::GetSubsystem takes a UWorld* as it's argument, it only ever uses it
        // to get the world context, so that it can provide a different online subsystem per
        // play-in-editor context.
        //
        // There's one UGameInstance* per play-in-editor context, which also means game instance
        // subsystems get created per play-in-editor context. In contrast, the UWorld* gets reused
        // between contexts, which means that if we inherited from UWorldSubsystem, our lifetime
        // would span contexts and in turn make accessing the correct online subsystem instance
        // much harder.
        //
        // So we get the UWorld* of the UGameInstance*, which will be the same UWorld that's shared
        // between all other instances, but we'll have the right FWorldContext
        auto Subsystem = Online::GetSubsystem(World);
        check(Subsystem != nullptr);
        
        // Finish setup, store the reference to the interface.
        auto __Handle__ = Subsystem->GetPartyInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlinePartySubsystem::IsHandleValid(const TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlinePartySubsystem::UOnlinePartySubsystem()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        return;
    }

    auto GameInstance = this->GetGameInstance();
    check(IsValid(GameInstance));

    auto World = GameInstance->GetWorld();
    check(IsValid(World));
    
    // Store the WorldContext so we can assert that it never changes later during our
    // operation (required for semantic correctness).
    this->WorldContext = GEngine->GetWorldContextFromWorld(World);
    check(WorldContext != nullptr);
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlinePartySubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnPartyJoined = Handle->AddOnPartyJoinedDelegate_Handle(
            FOnPartyJoined::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) {
                this->OnPartyJoined.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId));
            })
        );
        this->DelegateHandle_OnPartyExited = Handle->AddOnPartyExitedDelegate_Handle(
            FOnPartyExited::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId) {
                this->OnPartyExited.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId));
            })
        );
        this->DelegateHandle_OnPartyStateChanged = Handle->AddOnPartyStateChangedDelegate_Handle(
            FOnPartyStateChanged::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, EPartyState State, EPartyState PreviousState) {
                this->OnPartyStateChanged.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (EPartyState_)(State), (EPartyState_)(PreviousState));
            })
        );
        this->DelegateHandle_OnPartyJIP = Handle->AddOnPartyJIPDelegate_Handle(
            FOnPartyJIP::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, bool Success) {
                this->OnPartyJIP.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), Success);
            })
        );
        this->DelegateHandle_OnPartyPromotionLockoutChanged = Handle->AddOnPartyPromotionLockoutChangedDelegate_Handle(
            FOnPartyPromotionLockoutChanged::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const bool bLockoutState) {
                this->OnPartyPromotionLockoutChanged.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), bLockoutState);
            })
        );
        this->DelegateHandle_OnPartyConfigChanged = Handle->AddOnPartyConfigChangedDelegate_Handle(
            FOnPartyConfigChanged::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FPartyConfiguration& PartyConfig) {
                this->OnPartyConfigChanged.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), FOnlinePartyConfiguration::FromNative(PartyConfig));
            })
        );
        this->DelegateHandle_OnPartyDataReceived = Handle->AddOnPartyDataReceivedDelegate_Handle(
            FOnPartyDataReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FName& Namespace, const FOnlinePartyData& PartyData) {
                this->OnPartyDataReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), Namespace, UReadablePartyData::FromNative(PartyData));
            })
        );
        this->DelegateHandle_OnPartyMemberPromoted = Handle->AddOnPartyMemberPromotedDelegate_Handle(
            FOnPartyMemberPromoted::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& NewLeaderId) {
                this->OnPartyMemberPromoted.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(NewLeaderId));
            })
        );
        this->DelegateHandle_OnPartyMemberExited = Handle->AddOnPartyMemberExitedDelegate_Handle(
            FOnPartyMemberExited::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const EMemberExitedReason Reason) {
                this->OnPartyMemberExited.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(MemberId), (EMemberExitedReason_)(Reason));
            })
        );
        this->DelegateHandle_OnPartyMemberJoined = Handle->AddOnPartyMemberJoinedDelegate_Handle(
            FOnPartyMemberJoined::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId) {
                this->OnPartyMemberJoined.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(MemberId));
            })
        );
        this->DelegateHandle_OnPartyMemberDataReceived = Handle->AddOnPartyMemberDataReceivedDelegate_Handle(
            FOnPartyMemberDataReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const FName& Namespace, const FOnlinePartyData& PartyData) {
                this->OnPartyMemberDataReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(MemberId), Namespace, UReadablePartyData::FromNative(PartyData));
            })
        );
        this->DelegateHandle_OnPartyInvitesChanged = Handle->AddOnPartyInvitesChangedDelegate_Handle(
            FOnPartyInvitesChanged::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId) {
                this->OnPartyInvitesChanged.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId));
            })
        );
        this->DelegateHandle_OnPartyInviteRequestReceived = Handle->AddOnPartyInviteRequestReceivedDelegate_Handle(
            FOnPartyInviteRequestReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& SenderId, const FUniqueNetId& RequestForId) {
                this->OnPartyInviteRequestReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(SenderId), ConvertDangerousUniqueNetIdToRepl(RequestForId));
            })
        );
        this->DelegateHandle_OnPartyInviteReceived = Handle->AddOnPartyInviteReceivedDelegate_Handle(
            FOnPartyInviteReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& SenderId) {
                this->OnPartyInviteReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(SenderId));
            })
        );
        this->DelegateHandle_OnPartyInviteReceivedEx = Handle->AddOnPartyInviteReceivedExDelegate_Handle(
            FOnPartyInviteReceivedEx::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const IOnlinePartyJoinInfo& Invitation) {
                this->OnPartyInviteReceivedEx.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UOnlinePartyJoinInfo::FromNative(Invitation));
            })
        );
        this->DelegateHandle_OnPartyJIPRequestReceived = Handle->AddOnPartyJIPRequestReceivedDelegate_Handle(
            FOnPartyJIPRequestReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& SenderId) {
                this->OnPartyJIPRequestReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(SenderId));
            })
        );
        this->DelegateHandle_OnFillPartyJoinRequestData = Handle->AddOnFillPartyJoinRequestDataDelegate_Handle(
            FOnFillPartyJoinRequestData::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, FOnlinePartyData& PartyData) {
                this->OnFillPartyJoinRequestData.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), UMutablePartyData::FromNative(PartyData));
            })
        );
    }
}

void UOnlinePartySubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlinePartySystem, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnPartyJoinedDelegate_Handle(this->DelegateHandle_OnPartyJoined);
        Handle->ClearOnPartyExitedDelegate_Handle(this->DelegateHandle_OnPartyExited);
        Handle->ClearOnPartyStateChangedDelegate_Handle(this->DelegateHandle_OnPartyStateChanged);
        Handle->ClearOnPartyJIPDelegate_Handle(this->DelegateHandle_OnPartyJIP);
        Handle->ClearOnPartyPromotionLockoutChangedDelegate_Handle(this->DelegateHandle_OnPartyPromotionLockoutChanged);
        Handle->ClearOnPartyConfigChangedDelegate_Handle(this->DelegateHandle_OnPartyConfigChanged);
        Handle->ClearOnPartyDataReceivedDelegate_Handle(this->DelegateHandle_OnPartyDataReceived);
        Handle->ClearOnPartyMemberPromotedDelegate_Handle(this->DelegateHandle_OnPartyMemberPromoted);
        Handle->ClearOnPartyMemberExitedDelegate_Handle(this->DelegateHandle_OnPartyMemberExited);
        Handle->ClearOnPartyMemberJoinedDelegate_Handle(this->DelegateHandle_OnPartyMemberJoined);
        Handle->ClearOnPartyMemberDataReceivedDelegate_Handle(this->DelegateHandle_OnPartyMemberDataReceived);
        Handle->ClearOnPartyInvitesChangedDelegate_Handle(this->DelegateHandle_OnPartyInvitesChanged);
        Handle->ClearOnPartyInviteRequestReceivedDelegate_Handle(this->DelegateHandle_OnPartyInviteRequestReceived);
        Handle->ClearOnPartyInviteReceivedDelegate_Handle(this->DelegateHandle_OnPartyInviteReceived);
        Handle->ClearOnPartyInviteReceivedExDelegate_Handle(this->DelegateHandle_OnPartyInviteReceivedEx);
        Handle->ClearOnPartyJIPRequestReceivedDelegate_Handle(this->DelegateHandle_OnPartyJIPRequestReceived);
        Handle->ClearOnFillPartyJoinRequestDataDelegate_Handle(this->DelegateHandle_OnFillPartyJoinRequestData);
        
    }

    Super::BeginDestroy();
}

bool UOnlinePartySubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlinePartySubsystemRestoreParties* UOnlinePartySubsystemRestoreParties::RestoreParties(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemRestoreParties* Node = NewObject<UOnlinePartySubsystemRestoreParties>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    return Node;
}

void UOnlinePartySubsystemRestoreParties::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnRestorePartiesComplete = FOnRestorePartiesComplete::CreateUObject(this, &UOnlinePartySubsystemRestoreParties::HandleCallback_OnRestorePartiesComplete);

    Handle->RestoreParties(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnRestorePartiesComplete);
}

void UOnlinePartySubsystemRestoreParties::HandleCallback_OnRestorePartiesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnRestorePartiesComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Result));
}

UOnlinePartySubsystemRestoreInvites* UOnlinePartySubsystemRestoreInvites::RestoreInvites(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemRestoreInvites* Node = NewObject<UOnlinePartySubsystemRestoreInvites>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    return Node;
}

void UOnlinePartySubsystemRestoreInvites::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnRestoreInvitesComplete = FOnRestoreInvitesComplete::CreateUObject(this, &UOnlinePartySubsystemRestoreInvites::HandleCallback_OnRestoreInvitesComplete);

    Handle->RestoreInvites(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnRestoreInvitesComplete);
}

void UOnlinePartySubsystemRestoreInvites::HandleCallback_OnRestoreInvitesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnRestoreInvitesComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Result));
}

UOnlinePartySubsystemCleanupParties* UOnlinePartySubsystemCleanupParties::CleanupParties(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemCleanupParties* Node = NewObject<UOnlinePartySubsystemCleanupParties>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    return Node;
}

void UOnlinePartySubsystemCleanupParties::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnCleanupPartiesComplete = FOnCleanupPartiesComplete::CreateUObject(this, &UOnlinePartySubsystemCleanupParties::HandleCallback_OnCleanupPartiesComplete);

    Handle->CleanupParties(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnCleanupPartiesComplete);
}

void UOnlinePartySubsystemCleanupParties::HandleCallback_OnCleanupPartiesComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnCleanupPartiesComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Result));
}

UOnlinePartySubsystemCreateParty* UOnlinePartySubsystemCreateParty::CreateParty(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, int64 PartyTypeId, FOnlinePartyConfiguration PartyConfig)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemCreateParty* Node = NewObject<UOnlinePartySubsystemCreateParty>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyTypeId = PartyTypeId;
    Node->__Store__PartyConfig = PartyConfig;
    return Node;
}

void UOnlinePartySubsystemCreateParty::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnCreatePartyComplete = FOnCreatePartyComplete::CreateUObject(this, &UOnlinePartySubsystemCreateParty::HandleCallback_OnCreatePartyComplete);
    
    if (!Handle->CreateParty(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), FOnlinePartyTypeId(this->__Store__PartyTypeId), this->__Store__PartyConfig.ToNative().ToSharedRef().Get(), __DelegateHandle_OnCreatePartyComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CreateParty' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemCreateParty::HandleCallback_OnCreatePartyComplete(const FUniqueNetId& LocalUserId, const TSharedPtr<const FOnlinePartyId>& PartyId, const ECreatePartyCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ECreatePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnCreatePartyComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (ECreatePartyCompletionResult_)(Result));
}

UOnlinePartySubsystemUpdateParty* UOnlinePartySubsystemUpdateParty::UpdateParty(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FOnlinePartyConfiguration PartyConfig, bool bShouldRegenerateReservationKey)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemUpdateParty* Node = NewObject<UOnlinePartySubsystemUpdateParty>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__PartyConfig = PartyConfig;
    Node->__Store__bShouldRegenerateReservationKey = bShouldRegenerateReservationKey;
    return Node;
}

void UOnlinePartySubsystemUpdateParty::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnUpdatePartyComplete = FOnUpdatePartyComplete::CreateUObject(this, &UOnlinePartySubsystemUpdateParty::HandleCallback_OnUpdatePartyComplete);
    
    if (!Handle->UpdateParty(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), this->__Store__PartyConfig.ToNative().ToSharedRef().Get(), this->__Store__bShouldRegenerateReservationKey, __DelegateHandle_OnUpdatePartyComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UpdateParty' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemUpdateParty::HandleCallback_OnUpdatePartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EUpdateConfigCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EUpdateConfigCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnUpdatePartyComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (EUpdateConfigCompletionResult_)(Result));
}

UOnlinePartySubsystemJoinParty* UOnlinePartySubsystemJoinParty::JoinParty(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UOnlinePartyJoinInfo* OnlinePartyJoinInfo)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemJoinParty* Node = NewObject<UOnlinePartySubsystemJoinParty>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__OnlinePartyJoinInfo = OnlinePartyJoinInfo;
    return Node;
}

void UOnlinePartySubsystemJoinParty::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!((IsValid(this->__Store__OnlinePartyJoinInfo) && this->__Store__OnlinePartyJoinInfo->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'OnlinePartyJoinInfo' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnJoinPartyComplete = FOnJoinPartyComplete::CreateUObject(this, &UOnlinePartySubsystemJoinParty::HandleCallback_OnJoinPartyComplete);
    
    if (!Handle->JoinParty(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__OnlinePartyJoinInfo->ToNative(), __DelegateHandle_OnJoinPartyComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'JoinParty' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
}

void UOnlinePartySubsystemJoinParty::HandleCallback_OnJoinPartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnJoinPartyComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (EJoinPartyCompletionResult_)(Result), NotApprovedReason);
}

bool UOnlinePartySubsystem::JIPFromWithinParty(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl PartyLeaderId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!(PartyLeaderId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->JIPFromWithinParty(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), PartyLeaderId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

UOnlinePartySubsystemQueryPartyJoinability* UOnlinePartySubsystemQueryPartyJoinability::QueryPartyJoinability(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UOnlinePartyJoinInfo* OnlinePartyJoinInfo)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemQueryPartyJoinability* Node = NewObject<UOnlinePartySubsystemQueryPartyJoinability>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__OnlinePartyJoinInfo = OnlinePartyJoinInfo;
    return Node;
}

void UOnlinePartySubsystemQueryPartyJoinability::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!((IsValid(this->__Store__OnlinePartyJoinInfo) && this->__Store__OnlinePartyJoinInfo->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'OnlinePartyJoinInfo' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnQueryPartyJoinabilityComplete = FOnQueryPartyJoinabilityComplete::CreateUObject(this, &UOnlinePartySubsystemQueryPartyJoinability::HandleCallback_OnQueryPartyJoinabilityComplete);

    Handle->QueryPartyJoinability(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__OnlinePartyJoinInfo->ToNative(), __DelegateHandle_OnQueryPartyJoinabilityComplete);
}

void UOnlinePartySubsystemQueryPartyJoinability::HandleCallback_OnQueryPartyJoinabilityComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnQueryPartyJoinabilityComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (EJoinPartyCompletionResult_)(Result), NotApprovedReason);
}

UOnlinePartySubsystemRejoinParty* UOnlinePartySubsystemRejoinParty::RejoinParty(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, int64 PartyTypeId, TArray<FUniqueNetIdRepl> FormerMembers)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemRejoinParty* Node = NewObject<UOnlinePartySubsystemRejoinParty>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__PartyTypeId = PartyTypeId;
    Node->__Store__FormerMembers = FormerMembers;
    return Node;
}

void UOnlinePartySubsystemRejoinParty::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__FormerMembers, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FormerMembers' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnJoinPartyComplete = FOnJoinPartyComplete::CreateUObject(this, &UOnlinePartySubsystemRejoinParty::HandleCallback_OnJoinPartyComplete);
    
    if (!Handle->RejoinParty(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), FOnlinePartyTypeId(this->__Store__PartyTypeId), ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__FormerMembers, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), __DelegateHandle_OnJoinPartyComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'RejoinParty' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
}

void UOnlinePartySubsystemRejoinParty::HandleCallback_OnJoinPartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const EJoinPartyCompletionResult Result, const int32 NotApprovedReason)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, EJoinPartyCompletionResult_::UnknownInternalFailure, 0);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnJoinPartyComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (EJoinPartyCompletionResult_)(Result), NotApprovedReason);
}

UOnlinePartySubsystemLeaveParty* UOnlinePartySubsystemLeaveParty::LeaveParty(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, bool bSynchronizeLeave)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemLeaveParty* Node = NewObject<UOnlinePartySubsystemLeaveParty>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__bSynchronizeLeave = bSynchronizeLeave;
    return Node;
}

void UOnlinePartySubsystemLeaveParty::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLeavePartyComplete = FOnLeavePartyComplete::CreateUObject(this, &UOnlinePartySubsystemLeaveParty::HandleCallback_OnLeavePartyComplete);
    
    if (!Handle->LeaveParty(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), this->__Store__bSynchronizeLeave, __DelegateHandle_OnLeavePartyComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'LeaveParty' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemLeaveParty::HandleCallback_OnLeavePartyComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const ELeavePartyCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, ELeavePartyCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLeavePartyComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), (ELeavePartyCompletionResult_)(Result));
}

bool UOnlinePartySubsystem::ApproveJoinRequest(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bIsApproved, int32 DeniedResultCode)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!(RecipientId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->ApproveJoinRequest(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), RecipientId.GetUniqueNetId().ToSharedRef().Get(), bIsApproved, DeniedResultCode);

    return __Result;
}

bool UOnlinePartySubsystem::ApproveJIPRequest(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bIsApproved, int32 DeniedResultCode)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!(RecipientId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->ApproveJIPRequest(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), RecipientId.GetUniqueNetId().ToSharedRef().Get(), bIsApproved, DeniedResultCode);

    return __Result;
}

void UOnlinePartySubsystem::RespondToQueryJoinability(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl RecipientId, bool bCanJoin, int32 DeniedResultCode, UReadablePartyData* PartyData)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }
    if (!(LocalUserId.IsValid()))
    {
        return ;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return ;
    }
    if (!(RecipientId.IsValid()))
    {
        return ;
    }
    if (!((IsValid(PartyData) && PartyData->IsValid())))
    {
        return ;
    }


    Handle->RespondToQueryJoinability(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), RecipientId.GetUniqueNetId().ToSharedRef().Get(), bCanJoin, DeniedResultCode, PartyData->ToNativeConst());
}

UOnlinePartySubsystemSendInvitation* UOnlinePartySubsystemSendInvitation::SendInvitation(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl Recipient)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemSendInvitation* Node = NewObject<UOnlinePartySubsystemSendInvitation>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__Recipient = Recipient;
    return Node;
}

void UOnlinePartySubsystemSendInvitation::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__Recipient.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Recipient' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSendPartyInvitationComplete = FOnSendPartyInvitationComplete::CreateUObject(this, &UOnlinePartySubsystemSendInvitation::HandleCallback_OnSendPartyInvitationComplete);
    
    if (!Handle->SendInvitation(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), FPartyInvitationRecipient(this->__Store__Recipient.GetUniqueNetId().ToSharedRef()), __DelegateHandle_OnSendPartyInvitationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'SendInvitation' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemSendInvitation::HandleCallback_OnSendPartyInvitationComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& RecipientId, const ESendPartyInvitationCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), ESendPartyInvitationCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSendPartyInvitationComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(RecipientId), (ESendPartyInvitationCompletionResult_)(Result));
}

bool UOnlinePartySubsystem::RejectInvitation(FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl SenderId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!(SenderId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->RejectInvitation(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), SenderId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

UOnlinePartySubsystemKickMember* UOnlinePartySubsystemKickMember::KickMember(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl TargetMemberId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemKickMember* Node = NewObject<UOnlinePartySubsystemKickMember>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__TargetMemberId = TargetMemberId;
    return Node;
}

void UOnlinePartySubsystemKickMember::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__TargetMemberId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetMemberId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnKickPartyMemberComplete = FOnKickPartyMemberComplete::CreateUObject(this, &UOnlinePartySubsystemKickMember::HandleCallback_OnKickPartyMemberComplete);
    
    if (!Handle->KickMember(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), this->__Store__TargetMemberId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnKickPartyMemberComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'KickMember' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemKickMember::HandleCallback_OnKickPartyMemberComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const EKickMemberCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EKickMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnKickPartyMemberComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(MemberId), (EKickMemberCompletionResult_)(Result));
}

UOnlinePartySubsystemPromoteMember* UOnlinePartySubsystemPromoteMember::PromoteMember(UOnlinePartySubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl TargetMemberId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePartySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePartySubsystemPromoteMember* Node = NewObject<UOnlinePartySubsystemPromoteMember>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__PartyId = PartyId;
    Node->__Store__TargetMemberId = TargetMemberId;
    return Node;
}

void UOnlinePartySubsystemPromoteMember::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!((IsValid(this->__Store__PartyId) && this->__Store__PartyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PartyId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (!(this->__Store__TargetMemberId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetMemberId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnPromotePartyMemberComplete = FOnPromotePartyMemberComplete::CreateUObject(this, &UOnlinePartySubsystemPromoteMember::HandleCallback_OnPromotePartyMemberComplete);
    
    if (!Handle->PromoteMember(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__PartyId->ToNative(), this->__Store__TargetMemberId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnPromotePartyMemberComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'PromoteMember' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
}

void UOnlinePartySubsystemPromoteMember::HandleCallback_OnPromotePartyMemberComplete(const FUniqueNetId& LocalUserId, const FOnlinePartyId& PartyId, const FUniqueNetId& MemberId, const EPromoteMemberCompletionResult Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), nullptr, FUniqueNetIdRepl(), EPromoteMemberCompletionResult_::UnknownInternalFailure);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnPromotePartyMemberComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), UPartyId::FromNative(PartyId), ConvertDangerousUniqueNetIdToRepl(MemberId), (EPromoteMemberCompletionResult_)(Result));
}

bool UOnlinePartySubsystem::UpdatePartyData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace, UReadablePartyData* PartyData)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!((IsValid(PartyData) && PartyData->IsValid())))
    {
        return false;
    }


    auto __Result = Handle->UpdatePartyData(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), Namespace, PartyData->ToNativeConst().Get());

    return __Result;
}

bool UOnlinePartySubsystem::UpdatePartyMemberData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace, UReadablePartyData* PartyMemberData)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!((IsValid(PartyMemberData) && PartyMemberData->IsValid())))
    {
        return false;
    }


    auto __Result = Handle->UpdatePartyMemberData(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), Namespace, PartyMemberData->ToNativeConst().Get());

    return __Result;
}

bool UOnlinePartySubsystem::IsMemberLeader(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }
    if (!(MemberId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsMemberLeader(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), MemberId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

int64 UOnlinePartySubsystem::GetPartyMemberCount(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return int64();
    }
    if (!(LocalUserId.IsValid()))
    {
        return int64();
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return int64();
    }


    auto __Result = (int64)(Handle->GetPartyMemberCount(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative()));

    return __Result;
}

UParty* UOnlinePartySubsystem::GetParty(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(LocalUserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return nullptr;
    }


    auto __Result = UParty::FromNative(Handle->GetParty(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative()));

    return __Result;
}

UBlueprintPartyMember* UOnlinePartySubsystem::GetPartyMember(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(LocalUserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return nullptr;
    }
    if (!(MemberId.IsValid()))
    {
        return nullptr;
    }


    auto __Result = UBlueprintPartyMember::FromNative(Handle->GetPartyMember(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), MemberId.GetUniqueNetId().ToSharedRef().Get()));

    return __Result;
}

UReadablePartyData* UOnlinePartySubsystem::GetPartyData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FName Namespace)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(LocalUserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return nullptr;
    }


    auto __Result = UReadablePartyData::FromNative(Handle->GetPartyData(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), Namespace));

    return __Result;
}

UReadablePartyData* UOnlinePartySubsystem::GetPartyMemberData(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, FUniqueNetIdRepl MemberId, FName Namespace)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(LocalUserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return nullptr;
    }
    if (!(MemberId.IsValid()))
    {
        return nullptr;
    }


    auto __Result = UReadablePartyData::FromNative(Handle->GetPartyMemberData(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), MemberId.GetUniqueNetId().ToSharedRef().Get(), Namespace));

    return __Result;
}

bool UOnlinePartySubsystem::GetJoinedParties(FUniqueNetIdRepl LocalUserId, TArray<UPartyId*>& OutPartyIdArray)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }

    TArray<TSharedRef<const FOnlinePartyId>> __StoreTemp__OutPartyIdArray;

    auto __Result = Handle->GetJoinedParties(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutPartyIdArray);
    OutPartyIdArray = ConvertArrayElements<TSharedRef<const FOnlinePartyId>, UPartyId*>(__StoreTemp__OutPartyIdArray, [](const TSharedRef<const FOnlinePartyId>& Val) { return UPartyId::FromNative(Val); });

    return __Result;
}

bool UOnlinePartySubsystem::GetPartyMembers(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, TArray<UBlueprintPartyMember*>& OutPartyMembersArray)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }

    TArray<FOnlinePartyMemberConstRef> __StoreTemp__OutPartyMembersArray;

    auto __Result = Handle->GetPartyMembers(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), __StoreTemp__OutPartyMembersArray);
    OutPartyMembersArray = ConvertArrayElements<FOnlinePartyMemberConstRef, UBlueprintPartyMember*>(__StoreTemp__OutPartyMembersArray, [](const FOnlinePartyMemberConstRef& Val) { return UBlueprintPartyMember::FromNative(Val); });

    return __Result;
}

bool UOnlinePartySubsystem::GetPendingInvites(FUniqueNetIdRepl LocalUserId, TArray<UOnlinePartyJoinInfo*>& OutPendingInvitesArray)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }

    TArray<IOnlinePartyJoinInfoConstRef> __StoreTemp__OutPendingInvitesArray;

    auto __Result = Handle->GetPendingInvites(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutPendingInvitesArray);
    OutPendingInvitesArray = ConvertArrayElements<IOnlinePartyJoinInfoConstRef, UOnlinePartyJoinInfo*>(__StoreTemp__OutPendingInvitesArray, [](const IOnlinePartyJoinInfoConstRef& Val) { return UOnlinePartyJoinInfo::FromNative(Val.Get()); });

    return __Result;
}

bool UOnlinePartySubsystem::GetPendingInvitedUsers(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId, TArray<FUniqueNetIdRepl>& OutPendingInvitedUserArray)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(LocalUserId.IsValid()))
    {
        return false;
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return false;
    }

    TArray<FUniqueNetIdRef> __StoreTemp__OutPendingInvitedUserArray;

    auto __Result = Handle->GetPendingInvitedUsers(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative(), __StoreTemp__OutPendingInvitedUserArray);
    OutPendingInvitedUserArray = ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(__StoreTemp__OutPendingInvitedUserArray, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); });

    return __Result;
}

FString UOnlinePartySubsystem::MakeJoinInfoJson(FUniqueNetIdRepl LocalUserId, UPartyId* PartyId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }
    if (!(LocalUserId.IsValid()))
    {
        return TEXT("");
    }
    if (!((IsValid(PartyId) && PartyId->IsValid())))
    {
        return TEXT("");
    }


    auto __Result = Handle->MakeJoinInfoJson(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), PartyId->ToNative());

    return __Result;
}

FString UOnlinePartySubsystem::MakeTokenFromJoinInfo(UOnlinePartyJoinInfo* JoinInfo)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }
    if (!((IsValid(JoinInfo) && JoinInfo->IsValid())))
    {
        return TEXT("");
    }


    auto __Result = Handle->MakeTokenFromJoinInfo(JoinInfo->ToNative());

    return __Result;
}

void UOnlinePartySubsystem::DumpPartyState()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DumpPartyState();
}

