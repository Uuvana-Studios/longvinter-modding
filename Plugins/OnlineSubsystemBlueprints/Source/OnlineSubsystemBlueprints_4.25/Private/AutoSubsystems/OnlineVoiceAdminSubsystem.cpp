// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineVoiceAdminSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> UOnlineVoiceAdminSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Online::GetVoiceAdminInterface(Subsystem);
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineVoiceAdminSubsystem::IsHandleValid(const TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineVoiceAdminSubsystem::UOnlineVoiceAdminSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineVoiceAdminSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
    }
}

void UOnlineVoiceAdminSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineVoiceAdminSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineVoiceAdminSubsystemCreateChannelCredentials* UOnlineVoiceAdminSubsystemCreateChannelCredentials::CreateChannelCredentials(UOnlineVoiceAdminSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, FString ChannelName, TArray<FUniqueNetIdRepl> TargetUserIds)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineVoiceAdminSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineVoiceAdminSubsystemCreateChannelCredentials* Node = NewObject<UOnlineVoiceAdminSubsystemCreateChannelCredentials>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ChannelName = ChannelName;
    Node->__Store__TargetUserIds = TargetUserIds;
    return Node;
}

void UOnlineVoiceAdminSubsystemCreateChannelCredentials::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__TargetUserIds, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetUserIds' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnVoiceAdminCreateChannelCredentialsComplete = FOnVoiceAdminCreateChannelCredentialsComplete::CreateUObject(this, &UOnlineVoiceAdminSubsystemCreateChannelCredentials::HandleCallback_OnVoiceAdminCreateChannelCredentialsComplete);
    
    if (!Handle->CreateChannelCredentials(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ChannelName, ConvertArrayElements<FUniqueNetIdRepl, TSharedRef<const FUniqueNetId>>(this->__Store__TargetUserIds, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), __DelegateHandle_OnVoiceAdminCreateChannelCredentialsComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CreateChannelCredentials' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }
}

void UOnlineVoiceAdminSubsystemCreateChannelCredentials::HandleCallback_OnVoiceAdminCreateChannelCredentialsComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const TArray<FVoiceAdminChannelCredentials>& Credentials)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<FVoiceAdminChannelCredentialsBP>());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnVoiceAdminCreateChannelCredentialsComplete.Broadcast(FOnlineErrorInfo::FromNative(Result), ConvertDangerousUniqueNetIdToRepl(LocalUserId), ConvertArrayElements<FVoiceAdminChannelCredentials, FVoiceAdminChannelCredentialsBP>(Credentials, [](const FVoiceAdminChannelCredentials& Val) { return FVoiceAdminChannelCredentialsBP::FromNative(Val); }));
}

UOnlineVoiceAdminSubsystemKickParticipant* UOnlineVoiceAdminSubsystemKickParticipant::KickParticipant(UOnlineVoiceAdminSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, FString ChannelName, FUniqueNetIdRepl TargetUserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineVoiceAdminSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineVoiceAdminSubsystemKickParticipant* Node = NewObject<UOnlineVoiceAdminSubsystemKickParticipant>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ChannelName = ChannelName;
    Node->__Store__TargetUserId = TargetUserId;
    return Node;
}

void UOnlineVoiceAdminSubsystemKickParticipant::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__TargetUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnVoiceAdminKickParticipantComplete = FOnVoiceAdminKickParticipantComplete::CreateUObject(this, &UOnlineVoiceAdminSubsystemKickParticipant::HandleCallback_OnVoiceAdminKickParticipantComplete);
    
    if (!Handle->KickParticipant(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ChannelName, this->__Store__TargetUserId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnVoiceAdminKickParticipantComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'KickParticipant' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineVoiceAdminSubsystemKickParticipant::HandleCallback_OnVoiceAdminKickParticipantComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnVoiceAdminKickParticipantComplete.Broadcast(FOnlineErrorInfo::FromNative(Result), ConvertDangerousUniqueNetIdToRepl(LocalUserId), ConvertDangerousUniqueNetIdToRepl(TargetUserId));
}

UOnlineVoiceAdminSubsystemSetParticipantHardMute* UOnlineVoiceAdminSubsystemSetParticipantHardMute::SetParticipantHardMute(UOnlineVoiceAdminSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, FString ChannelName, FUniqueNetIdRepl TargetUserId, bool bMuted)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineVoiceAdminSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineVoiceAdminSubsystemSetParticipantHardMute* Node = NewObject<UOnlineVoiceAdminSubsystemSetParticipantHardMute>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ChannelName = ChannelName;
    Node->__Store__TargetUserId = TargetUserId;
    Node->__Store__bMuted = bMuted;
    return Node;
}

void UOnlineVoiceAdminSubsystemSetParticipantHardMute::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }
    if (!(this->__Store__TargetUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnVoiceAdminSetParticipantHardMuteComplete = FOnVoiceAdminSetParticipantHardMuteComplete::CreateUObject(this, &UOnlineVoiceAdminSubsystemSetParticipantHardMute::HandleCallback_OnVoiceAdminSetParticipantHardMuteComplete);
    
    if (!Handle->SetParticipantHardMute(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ChannelName, this->__Store__TargetUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__bMuted, __DelegateHandle_OnVoiceAdminSetParticipantHardMuteComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'SetParticipantHardMute' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }
}

void UOnlineVoiceAdminSubsystemSetParticipantHardMute::HandleCallback_OnVoiceAdminSetParticipantHardMuteComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId, bool bIsNowMuted)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), FUniqueNetIdRepl(), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnVoiceAdminSetParticipantHardMuteComplete.Broadcast(FOnlineErrorInfo::FromNative(Result), ConvertDangerousUniqueNetIdToRepl(LocalUserId), ConvertDangerousUniqueNetIdToRepl(TargetUserId), bIsNowMuted);
}

