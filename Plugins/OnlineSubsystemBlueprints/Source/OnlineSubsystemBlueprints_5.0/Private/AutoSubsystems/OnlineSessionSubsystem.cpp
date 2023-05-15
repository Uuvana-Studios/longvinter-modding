// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineSessionSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> UOnlineSessionSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetSessionInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineSessionSubsystem::IsHandleValid(const TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineSessionSubsystem::UOnlineSessionSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineSessionSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            this->DelegateHandle_OnFindFriendSessionComplete.Add(Handle->AddOnFindFriendSessionCompleteDelegate_Handle(
                i,
                FOnFindFriendSessionComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResult) {
                    this->OnFindFriendSessionComplete.Broadcast(i, bWasSuccessful, ConvertArrayElements<FOnlineSessionSearchResult, FOnlineSessionSearchResultBP>(FriendSearchResult, [](const FOnlineSessionSearchResult& Val) { return FOnlineSessionSearchResultBP::FromNative(Val); }));
                })
            ));
        }
        this->DelegateHandle_OnCreateSessionComplete = Handle->AddOnCreateSessionCompleteDelegate_Handle(
            FOnCreateSessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnCreateSessionComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnStartSessionComplete = Handle->AddOnStartSessionCompleteDelegate_Handle(
            FOnStartSessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnStartSessionComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnUpdateSessionComplete = Handle->AddOnUpdateSessionCompleteDelegate_Handle(
            FOnUpdateSessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnUpdateSessionComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnEndSessionComplete = Handle->AddOnEndSessionCompleteDelegate_Handle(
            FOnEndSessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnEndSessionComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnDestroySessionComplete = Handle->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnDestroySessionComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnMatchmakingComplete = Handle->AddOnMatchmakingCompleteDelegate_Handle(
            FOnMatchmakingComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnMatchmakingComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnCancelMatchmakingComplete = Handle->AddOnCancelMatchmakingCompleteDelegate_Handle(
            FOnCancelMatchmakingComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, bool bWasSuccessful) {
                this->OnCancelMatchmakingComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
        this->DelegateHandle_OnFindSessionsComplete = Handle->AddOnFindSessionsCompleteDelegate_Handle(
            FOnFindSessionsComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful) {
                this->OnFindSessionsComplete.Broadcast(bWasSuccessful);
            })
        );
        this->DelegateHandle_OnCancelFindSessionsComplete = Handle->AddOnCancelFindSessionsCompleteDelegate_Handle(
            FOnCancelFindSessionsComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful) {
                this->OnCancelFindSessionsComplete.Broadcast(bWasSuccessful);
            })
        );
        this->DelegateHandle_OnPingSearchResultsComplete = Handle->AddOnPingSearchResultsCompleteDelegate_Handle(
            FOnPingSearchResultsComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful) {
                this->OnPingSearchResultsComplete.Broadcast(bWasSuccessful);
            })
        );
        this->DelegateHandle_OnJoinSessionComplete = Handle->AddOnJoinSessionCompleteDelegate_Handle(
            FOnJoinSessionComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, EOnJoinSessionCompleteResult::Type Result) {
                this->OnJoinSessionComplete.Broadcast(SessionName, (EOnJoinSessionCompleteResult_)(Result));
            })
        );
        this->DelegateHandle_OnSessionParticipantsChange = Handle->AddOnSessionParticipantsChangeDelegate_Handle(
            FOnSessionParticipantsChange::FDelegate::CreateWeakLambda(this, [this](FName Param1, const FUniqueNetId& Param2, bool Param3) {
                this->OnSessionParticipantsChange.Broadcast(Param1, ConvertDangerousUniqueNetIdToRepl(Param2), Param3);
            })
        );
        this->DelegateHandle_OnQosDataRequested = Handle->AddOnQosDataRequestedDelegate_Handle(
            FOnQosDataRequested::FDelegate::CreateWeakLambda(this, [this](FName Param1) {
                this->OnQosDataRequested.Broadcast(Param1);
            })
        );
        this->DelegateHandle_OnSessionCustomDataChanged = Handle->AddOnSessionCustomDataChangedDelegate_Handle(
            FOnSessionCustomDataChanged::FDelegate::CreateWeakLambda(this, [this](FName Param1, const FOnlineSessionSettings& Param2) {
                this->OnSessionCustomDataChanged.Broadcast(Param1, FOnlineSessionSettingsBP::FromNative(Param2));
            })
        );
        this->DelegateHandle_OnSessionSettingsUpdated = Handle->AddOnSessionSettingsUpdatedDelegate_Handle(
            FOnSessionSettingsUpdated::FDelegate::CreateWeakLambda(this, [this](FName Param1, const FOnlineSessionSettings& Param2) {
                this->OnSessionSettingsUpdated.Broadcast(Param1, FOnlineSessionSettingsBP::FromNative(Param2));
            })
        );
        this->DelegateHandle_OnSessionParticipantSettingsUpdated = Handle->AddOnSessionParticipantSettingsUpdatedDelegate_Handle(
            FOnSessionParticipantSettingsUpdated::FDelegate::CreateWeakLambda(this, [this](FName Param1, const FUniqueNetId& Param2, const FOnlineSessionSettings& Param3) {
                this->OnSessionParticipantSettingsUpdated.Broadcast(Param1, ConvertDangerousUniqueNetIdToRepl(Param2), FOnlineSessionSettingsBP::FromNative(Param3));
            })
        );
        this->DelegateHandle_OnSessionParticipantRemoved = Handle->AddOnSessionParticipantRemovedDelegate_Handle(
            FOnSessionParticipantRemoved::FDelegate::CreateWeakLambda(this, [this](FName Param1, const FUniqueNetId& Param2) {
                this->OnSessionParticipantRemoved.Broadcast(Param1, ConvertDangerousUniqueNetIdToRepl(Param2));
            })
        );
        this->DelegateHandle_OnSessionUserInviteAccepted = Handle->AddOnSessionUserInviteAcceptedDelegate_Handle(
            FOnSessionUserInviteAccepted::FDelegate::CreateWeakLambda(this, [this](const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult) {
                this->OnSessionUserInviteAccepted.Broadcast(bWasSuccessful, ControllerId, FUniqueNetIdRepl(UserId), FOnlineSessionSearchResultBP::FromNative(InviteResult));
            })
        );
        this->DelegateHandle_OnSessionInviteReceived = Handle->AddOnSessionInviteReceivedDelegate_Handle(
            FOnSessionInviteReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FromId, const FString& AppId, const FOnlineSessionSearchResult& InviteResult) {
                this->OnSessionInviteReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FromId), AppId, FOnlineSessionSearchResultBP::FromNative(InviteResult));
            })
        );
        this->DelegateHandle_OnRegisterPlayersComplete = Handle->AddOnRegisterPlayersCompleteDelegate_Handle(
            FOnRegisterPlayersComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, const TArray<FUniqueNetIdRef>& PlayerIds, bool bWasSuccessful) {
                this->OnRegisterPlayersComplete.Broadcast(SessionName, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(PlayerIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), bWasSuccessful);
            })
        );
        this->DelegateHandle_OnUnregisterPlayersComplete = Handle->AddOnUnregisterPlayersCompleteDelegate_Handle(
            FOnUnregisterPlayersComplete::FDelegate::CreateWeakLambda(this, [this](FName SessionName, const TArray<FUniqueNetIdRef>& PlayerIds, bool bWasSuccessful) {
                this->OnUnregisterPlayersComplete.Broadcast(SessionName, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(PlayerIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), bWasSuccessful);
            })
        );
        this->DelegateHandle_OnSessionFailure = Handle->AddOnSessionFailureDelegate_Handle(
            FOnSessionFailure::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& PlayerId, ESessionFailure::Type FailureType) {
                this->OnSessionFailure.Broadcast(ConvertDangerousUniqueNetIdToRepl(PlayerId), (ESessionFailure_)(FailureType));
            })
        );
    }
}

void UOnlineSessionSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            Handle->ClearOnFindFriendSessionCompleteDelegate_Handle(i, this->DelegateHandle_OnFindFriendSessionComplete[i]);
        }
        Handle->ClearOnCreateSessionCompleteDelegate_Handle(this->DelegateHandle_OnCreateSessionComplete);
        Handle->ClearOnStartSessionCompleteDelegate_Handle(this->DelegateHandle_OnStartSessionComplete);
        Handle->ClearOnUpdateSessionCompleteDelegate_Handle(this->DelegateHandle_OnUpdateSessionComplete);
        Handle->ClearOnEndSessionCompleteDelegate_Handle(this->DelegateHandle_OnEndSessionComplete);
        Handle->ClearOnDestroySessionCompleteDelegate_Handle(this->DelegateHandle_OnDestroySessionComplete);
        Handle->ClearOnMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnMatchmakingComplete);
        Handle->ClearOnCancelMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnCancelMatchmakingComplete);
        Handle->ClearOnFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnFindSessionsComplete);
        Handle->ClearOnCancelFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnCancelFindSessionsComplete);
        Handle->ClearOnPingSearchResultsCompleteDelegate_Handle(this->DelegateHandle_OnPingSearchResultsComplete);
        Handle->ClearOnJoinSessionCompleteDelegate_Handle(this->DelegateHandle_OnJoinSessionComplete);
        Handle->ClearOnSessionParticipantsChangeDelegate_Handle(this->DelegateHandle_OnSessionParticipantsChange);
        Handle->ClearOnQosDataRequestedDelegate_Handle(this->DelegateHandle_OnQosDataRequested);
        Handle->ClearOnSessionCustomDataChangedDelegate_Handle(this->DelegateHandle_OnSessionCustomDataChanged);
        Handle->ClearOnSessionSettingsUpdatedDelegate_Handle(this->DelegateHandle_OnSessionSettingsUpdated);
        Handle->ClearOnSessionParticipantSettingsUpdatedDelegate_Handle(this->DelegateHandle_OnSessionParticipantSettingsUpdated);
        Handle->ClearOnSessionParticipantRemovedDelegate_Handle(this->DelegateHandle_OnSessionParticipantRemoved);
        Handle->ClearOnSessionUserInviteAcceptedDelegate_Handle(this->DelegateHandle_OnSessionUserInviteAccepted);
        Handle->ClearOnSessionInviteReceivedDelegate_Handle(this->DelegateHandle_OnSessionInviteReceived);
        Handle->ClearOnRegisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnRegisterPlayersComplete);
        Handle->ClearOnUnregisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnUnregisterPlayersComplete);
        Handle->ClearOnSessionFailureDelegate_Handle(this->DelegateHandle_OnSessionFailure);
        
    }

    Super::BeginDestroy();
}

bool UOnlineSessionSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

FUniqueNetIdRepl UOnlineSessionSubsystem::CreateSessionIdFromString(FString SessionIdStr)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FUniqueNetIdRepl();
    }


    auto __Result = FUniqueNetIdRepl(Handle->CreateSessionIdFromString(SessionIdStr));

    return __Result;
}

UNamedOnlineSession* UOnlineSessionSubsystem::GetNamedSession(FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }


    auto __Result = UNamedOnlineSession::FromNative(Handle->GetNamedSession(SessionName));

    return __Result;
}

void UOnlineSessionSubsystem::RemoveNamedSession(FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->RemoveNamedSession(SessionName);
}

bool UOnlineSessionSubsystem::HasPresenceSession()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->HasPresenceSession();

    return __Result;
}

EOnlineSessionState_ UOnlineSessionSubsystem::GetSessionState(FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return EOnlineSessionState_::NoSession;
    }


    auto __Result = (EOnlineSessionState_)(Handle->GetSessionState(SessionName));

    return __Result;
}

UOnlineSessionSubsystemCreateSession* UOnlineSessionSubsystemCreateSession::CreateSession(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl HostingPlayerId, FName SessionName, FOnlineSessionSettingsBP NewSessionSettings)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemCreateSession* Node = NewObject<UOnlineSessionSubsystemCreateSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnCreateSessionComplete|%s"), *SessionName.ToString());
    Node->__Store__HostingPlayerId = HostingPlayerId;
    Node->__Store__SessionName = SessionName;
    Node->__Store__NewSessionSettings = NewSessionSettings;
    return Node;
}

void UOnlineSessionSubsystemCreateSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!(this->__Store__HostingPlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'HostingPlayerId' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnCreateSessionComplete = Handle->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemCreateSession::HandleCallback_OnCreateSessionComplete));
    
    if (!Handle->CreateSession(this->__Store__HostingPlayerId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionName, this->__Store__NewSessionSettings.ToNative()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CreateSession' did not start successfully"));
        Handle->ClearOnCreateSessionCompleteDelegate_Handle(this->DelegateHandle_OnCreateSessionComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemCreateSession::HandleCallback_OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnCreateSessionCompleteDelegate_Handle(this->DelegateHandle_OnCreateSessionComplete);

    this->OnCreateSessionComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemStartSession* UOnlineSessionSubsystemStartSession::StartSession(UOnlineSessionSubsystem* Subsystem ,FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemStartSession* Node = NewObject<UOnlineSessionSubsystemStartSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnStartSessionComplete|%s"), *SessionName.ToString());
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemStartSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnStartSessionComplete = Handle->AddOnStartSessionCompleteDelegate_Handle(
        FOnStartSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemStartSession::HandleCallback_OnStartSessionComplete));
    
    if (!Handle->StartSession(this->__Store__SessionName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'StartSession' did not start successfully"));
        Handle->ClearOnStartSessionCompleteDelegate_Handle(this->DelegateHandle_OnStartSessionComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemStartSession::HandleCallback_OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnStartSessionCompleteDelegate_Handle(this->DelegateHandle_OnStartSessionComplete);

    this->OnStartSessionComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemUpdateSession* UOnlineSessionSubsystemUpdateSession::UpdateSession(UOnlineSessionSubsystem* Subsystem ,FName SessionName, FOnlineSessionSettingsBP UpdatedSessionSettings, bool bShouldRefreshOnlineData)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemUpdateSession* Node = NewObject<UOnlineSessionSubsystemUpdateSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnUpdateSessionComplete|%s"), *SessionName.ToString());
    Node->__Store__SessionName = SessionName;
    Node->__Store__UpdatedSessionSettings = UpdatedSessionSettings;
    Node->__Store__bShouldRefreshOnlineData = bShouldRefreshOnlineData;
    return Node;
}

void UOnlineSessionSubsystemUpdateSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnUpdateSessionComplete = Handle->AddOnUpdateSessionCompleteDelegate_Handle(
        FOnUpdateSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemUpdateSession::HandleCallback_OnUpdateSessionComplete));
    
    if (!Handle->UpdateSession(this->__Store__SessionName, this->__Store__UpdatedSessionSettings.ToNativeRef(), this->__Store__bShouldRefreshOnlineData))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UpdateSession' did not start successfully"));
        Handle->ClearOnUpdateSessionCompleteDelegate_Handle(this->DelegateHandle_OnUpdateSessionComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemUpdateSession::HandleCallback_OnUpdateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnUpdateSessionCompleteDelegate_Handle(this->DelegateHandle_OnUpdateSessionComplete);

    this->OnUpdateSessionComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemEndSession* UOnlineSessionSubsystemEndSession::EndSession(UOnlineSessionSubsystem* Subsystem ,FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemEndSession* Node = NewObject<UOnlineSessionSubsystemEndSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnEndSessionComplete|%s"), *SessionName.ToString());
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemEndSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnEndSessionComplete = Handle->AddOnEndSessionCompleteDelegate_Handle(
        FOnEndSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemEndSession::HandleCallback_OnEndSessionComplete));
    
    if (!Handle->EndSession(this->__Store__SessionName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'EndSession' did not start successfully"));
        Handle->ClearOnEndSessionCompleteDelegate_Handle(this->DelegateHandle_OnEndSessionComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemEndSession::HandleCallback_OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnEndSessionCompleteDelegate_Handle(this->DelegateHandle_OnEndSessionComplete);

    this->OnEndSessionComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemDestroySession* UOnlineSessionSubsystemDestroySession::DestroySession(UOnlineSessionSubsystem* Subsystem ,FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemDestroySession* Node = NewObject<UOnlineSessionSubsystemDestroySession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemDestroySession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnDestroySessionComplete = FOnDestroySessionComplete::FDelegate::CreateUObject(this, &UOnlineSessionSubsystemDestroySession::HandleCallback_OnDestroySessionComplete);
    
    if (!Handle->DestroySession(this->__Store__SessionName, __DelegateHandle_OnDestroySessionComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DestroySession' did not start successfully"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemDestroySession::HandleCallback_OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnDestroySessionComplete.Broadcast(SessionName, bWasSuccessful);
}

bool UOnlineSessionSubsystem::IsPlayerInSession(FName SessionName, FUniqueNetIdRepl UniqueId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UniqueId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsPlayerInSession(SessionName, UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

UOnlineSessionSubsystemStartMatchmaking* UOnlineSessionSubsystemStartMatchmaking::StartMatchmaking(UOnlineSessionSubsystem* Subsystem ,TArray<FUniqueNetIdRepl> LocalPlayers, FName SessionName, FOnlineSessionSettingsBP NewSessionSettings, UOnlineSessionSearch* SearchSettings)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemStartMatchmaking* Node = NewObject<UOnlineSessionSubsystemStartMatchmaking>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnMatchmakingComplete|%s"), *SessionName.ToString());
    Node->__Store__LocalPlayers = LocalPlayers;
    Node->__Store__SessionName = SessionName;
    Node->__Store__NewSessionSettings = NewSessionSettings;
    Node->__Store__SearchSettings = SearchSettings;
    return Node;
}

void UOnlineSessionSubsystemStartMatchmaking::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__LocalPlayers, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalPlayers' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnMatchmakingComplete = Handle->AddOnMatchmakingCompleteDelegate_Handle(
        FOnMatchmakingCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemStartMatchmaking::HandleCallback_OnMatchmakingComplete));
    
    if (!Handle->StartMatchmaking(ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__LocalPlayers, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), this->__Store__SessionName, this->__Store__NewSessionSettings.ToNative(), this->__Store__SearchSettings->ToNativeRef()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'StartMatchmaking' did not start successfully"));
        Handle->ClearOnMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnMatchmakingComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemStartMatchmaking::HandleCallback_OnMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnMatchmakingComplete);

    this->OnMatchmakingComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemCancelMatchmaking* UOnlineSessionSubsystemCancelMatchmaking::CancelMatchmaking(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl SearchingPlayerId, FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemCancelMatchmaking* Node = NewObject<UOnlineSessionSubsystemCancelMatchmaking>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnCancelMatchmakingComplete|%s"), *SessionName.ToString());
    Node->__Store__SearchingPlayerId = SearchingPlayerId;
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemCancelMatchmaking::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!(this->__Store__SearchingPlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'SearchingPlayerId' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnCancelMatchmakingComplete = Handle->AddOnCancelMatchmakingCompleteDelegate_Handle(
        FOnCancelMatchmakingCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemCancelMatchmaking::HandleCallback_OnCancelMatchmakingComplete));
    
    if (!Handle->CancelMatchmaking(this->__Store__SearchingPlayerId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CancelMatchmaking' did not start successfully"));
        Handle->ClearOnCancelMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnCancelMatchmakingComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
}

void UOnlineSessionSubsystemCancelMatchmaking::HandleCallback_OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnCancelMatchmakingCompleteDelegate_Handle(this->DelegateHandle_OnCancelMatchmakingComplete);

    this->OnCancelMatchmakingComplete.Broadcast(SessionName, bWasSuccessful);
}

UOnlineSessionSubsystemFindSessions* UOnlineSessionSubsystemFindSessions::FindSessions(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl SearchingPlayerId, UOnlineSessionSearch* SearchSettings)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemFindSessions* Node = NewObject<UOnlineSessionSubsystemFindSessions>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("OnFindSessionsComplete");
    Node->__Store__SearchingPlayerId = SearchingPlayerId;
    Node->__Store__SearchSettings = SearchSettings;
    return Node;
}

void UOnlineSessionSubsystemFindSessions::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!(this->__Store__SearchingPlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'SearchingPlayerId' was not valid"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!((IsValid(this->__Store__SearchSettings) && this->__Store__SearchSettings->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'SearchSettings' was not valid"));
        this->OnCallFailed.Broadcast(false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnFindSessionsComplete = Handle->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemFindSessions::HandleCallback_OnFindSessionsComplete));
    
    if (!Handle->FindSessions(this->__Store__SearchingPlayerId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SearchSettings->ToNative()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'FindSessions' did not start successfully"));
        Handle->ClearOnFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnFindSessionsComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineSessionSubsystemFindSessions::HandleCallback_OnFindSessionsComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnFindSessionsComplete);

    this->OnFindSessionsComplete.Broadcast(bWasSuccessful);
}

UOnlineSessionSubsystemFindSessionById* UOnlineSessionSubsystemFindSessionById::FindSessionById(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl SearchingUserId, FUniqueNetIdRepl SessionId, FUniqueNetIdRepl FriendId, FString UserData)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemFindSessionById* Node = NewObject<UOnlineSessionSubsystemFindSessionById>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__SearchingUserId = SearchingUserId;
    Node->__Store__SessionId = SessionId;
    Node->__Store__FriendId = FriendId;
    Node->__Store__UserData = UserData;
    return Node;
}

void UOnlineSessionSubsystemFindSessionById::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
    if (!(this->__Store__SearchingUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'SearchingUserId' was not valid"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
    if (!(this->__Store__SessionId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'SessionId' was not valid"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSingleSessionResultComplete = FOnSingleSessionResultComplete::FDelegate::CreateUObject(this, &UOnlineSessionSubsystemFindSessionById::HandleCallback_OnSingleSessionResultComplete);
    
    if (!Handle->FindSessionById(this->__Store__SearchingUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__UserData, __DelegateHandle_OnSingleSessionResultComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'FindSessionById' did not start successfully"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
}

void UOnlineSessionSubsystemFindSessionById::HandleCallback_OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FOnlineSessionSearchResultBP());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSingleSessionResultComplete.Broadcast(LocalUserNum, bWasSuccessful, FOnlineSessionSearchResultBP::FromNative(SearchResult));
}

UOnlineSessionSubsystemCancelFindSessions* UOnlineSessionSubsystemCancelFindSessions::CancelFindSessions(UOnlineSessionSubsystem* Subsystem )
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemCancelFindSessions* Node = NewObject<UOnlineSessionSubsystemCancelFindSessions>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("OnCancelFindSessionsComplete");
    return Node;
}

void UOnlineSessionSubsystemCancelFindSessions::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnCancelFindSessionsComplete = Handle->AddOnCancelFindSessionsCompleteDelegate_Handle(
        FOnCancelFindSessionsCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemCancelFindSessions::HandleCallback_OnCancelFindSessionsComplete));
    
    if (!Handle->CancelFindSessions())
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CancelFindSessions' did not start successfully"));
        Handle->ClearOnCancelFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnCancelFindSessionsComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineSessionSubsystemCancelFindSessions::HandleCallback_OnCancelFindSessionsComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnCancelFindSessionsCompleteDelegate_Handle(this->DelegateHandle_OnCancelFindSessionsComplete);

    this->OnCancelFindSessionsComplete.Broadcast(bWasSuccessful);
}

UOnlineSessionSubsystemPingSearchResults* UOnlineSessionSubsystemPingSearchResults::PingSearchResults(UOnlineSessionSubsystem* Subsystem ,FOnlineSessionSearchResultBP SearchResult)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemPingSearchResults* Node = NewObject<UOnlineSessionSubsystemPingSearchResults>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("OnPingSearchResultsComplete");
    Node->__Store__SearchResult = SearchResult;
    return Node;
}

void UOnlineSessionSubsystemPingSearchResults::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnPingSearchResultsComplete = Handle->AddOnPingSearchResultsCompleteDelegate_Handle(
        FOnPingSearchResultsCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemPingSearchResults::HandleCallback_OnPingSearchResultsComplete));
    
    if (!Handle->PingSearchResults(this->__Store__SearchResult.ToNative()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'PingSearchResults' did not start successfully"));
        Handle->ClearOnPingSearchResultsCompleteDelegate_Handle(this->DelegateHandle_OnPingSearchResultsComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineSessionSubsystemPingSearchResults::HandleCallback_OnPingSearchResultsComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnPingSearchResultsCompleteDelegate_Handle(this->DelegateHandle_OnPingSearchResultsComplete);

    this->OnPingSearchResultsComplete.Broadcast(bWasSuccessful);
}

UOnlineSessionSubsystemJoinSession* UOnlineSessionSubsystemJoinSession::JoinSession(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, FName SessionName, FOnlineSessionSearchResultBP DesiredSession)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemJoinSession* Node = NewObject<UOnlineSessionSubsystemJoinSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("OnJoinSessionComplete");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__SessionName = SessionName;
    Node->__Store__DesiredSession = DesiredSession;
    return Node;
}

void UOnlineSessionSubsystemJoinSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnJoinSessionComplete = Handle->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemJoinSession::HandleCallback_OnJoinSessionComplete));
    
    if (!Handle->JoinSession(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionName, this->__Store__DesiredSession.ToNative()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'JoinSession' did not start successfully"));
        Handle->ClearOnJoinSessionCompleteDelegate_Handle(this->DelegateHandle_OnJoinSessionComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
}

void UOnlineSessionSubsystemJoinSession::HandleCallback_OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnJoinSessionCompleteDelegate_Handle(this->DelegateHandle_OnJoinSessionComplete);

    this->OnJoinSessionComplete.Broadcast(SessionName, (EOnJoinSessionCompleteResult_)(Result));
}

UOnlineSessionSubsystemFindFriendSession* UOnlineSessionSubsystemFindFriendSession::FindFriendSession(UOnlineSessionSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl Friend)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemFindFriendSession* Node = NewObject<UOnlineSessionSubsystemFindFriendSession>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnFindFriendSessionComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__Friend = Friend;
    return Node;
}

void UOnlineSessionSubsystemFindFriendSession::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
        return;
    }
    if (!(this->__Store__Friend.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Friend' was not valid"));
        this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnFindFriendSessionComplete = Handle->AddOnFindFriendSessionCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnFindFriendSessionCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemFindFriendSession::HandleCallback_OnFindFriendSessionComplete));
    
    if (!Handle->FindFriendSession(this->__Store__LocalUserNum, this->__Store__Friend.GetUniqueNetId().ToSharedRef().Get()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'FindFriendSession' did not start successfully"));
        Handle->ClearOnFindFriendSessionCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnFindFriendSessionComplete);
        this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
        return;
    }
}

void UOnlineSessionSubsystemFindFriendSession::HandleCallback_OnFindFriendSessionComplete(int32 LocalUserNumCb, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& FriendSearchResult)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
        return;
    }
    if (LocalUserNumCb != this->__Store__LocalUserNum)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TArray<FOnlineSessionSearchResultBP>());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnFindFriendSessionCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnFindFriendSessionComplete);

    this->OnFindFriendSessionComplete.Broadcast(bWasSuccessful, ConvertArrayElements<FOnlineSessionSearchResult, FOnlineSessionSearchResultBP>(FriendSearchResult, [](const FOnlineSessionSearchResult& Val) { return FOnlineSessionSearchResultBP::FromNative(Val); }));
}

bool UOnlineSessionSubsystem::SendSessionInviteToFriend(FUniqueNetIdRepl LocalUserId, FName SessionName, FUniqueNetIdRepl Friend)
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
    if (!(Friend.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->SendSessionInviteToFriend(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), SessionName, Friend.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineSessionSubsystem::SendSessionInviteToFriends(FUniqueNetIdRepl LocalUserId, FName SessionName, TArray<FUniqueNetIdRepl> Friends)
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
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(Friends, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        return false;
    }


    auto __Result = Handle->SendSessionInviteToFriends(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), SessionName, ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(Friends, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }));

    return __Result;
}

UOnlineSessionSettings* UOnlineSessionSubsystem::GetSessionSettings(FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }


    auto __Result = UOnlineSessionSettings::FromNative(Handle->GetSessionSettings(SessionName));

    return __Result;
}

UOnlineSessionSubsystemRegisterPlayers* UOnlineSessionSubsystemRegisterPlayers::RegisterPlayers(UOnlineSessionSubsystem* Subsystem ,FName SessionName, TArray<FUniqueNetIdRepl> Players, bool bWasInvited)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemRegisterPlayers* Node = NewObject<UOnlineSessionSubsystemRegisterPlayers>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnRegisterPlayersComplete|%s"), *SessionName.ToString());
    Node->__Store__SessionName = SessionName;
    Node->__Store__Players = Players;
    Node->__Store__bWasInvited = bWasInvited;
    return Node;
}

void UOnlineSessionSubsystemRegisterPlayers::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Players' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnRegisterPlayersComplete = Handle->AddOnRegisterPlayersCompleteDelegate_Handle(
        FOnRegisterPlayersCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemRegisterPlayers::HandleCallback_OnRegisterPlayersComplete));
    
    if (!Handle->RegisterPlayers(this->__Store__SessionName, ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), this->__Store__bWasInvited))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'RegisterPlayers' did not start successfully"));
        Handle->ClearOnRegisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnRegisterPlayersComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
}

void UOnlineSessionSubsystemRegisterPlayers::HandleCallback_OnRegisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerIds, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnRegisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnRegisterPlayersComplete);

    this->OnRegisterPlayersComplete.Broadcast(SessionName, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(PlayerIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), bWasSuccessful);
}

UOnlineSessionSubsystemUnregisterPlayers* UOnlineSessionSubsystemUnregisterPlayers::UnregisterPlayers(UOnlineSessionSubsystem* Subsystem ,FName SessionName, TArray<FUniqueNetIdRepl> Players)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemUnregisterPlayers* Node = NewObject<UOnlineSessionSubsystemUnregisterPlayers>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnUnregisterPlayersComplete|%s"), *SessionName.ToString());
    Node->__Store__SessionName = SessionName;
    Node->__Store__Players = Players;
    return Node;
}

void UOnlineSessionSubsystemUnregisterPlayers::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Players' was not valid"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnUnregisterPlayersComplete = Handle->AddOnUnregisterPlayersCompleteDelegate_Handle(
        FOnUnregisterPlayersCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemUnregisterPlayers::HandleCallback_OnUnregisterPlayersComplete));
    
    if (!Handle->UnregisterPlayers(this->__Store__SessionName, ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UnregisterPlayers' did not start successfully"));
        Handle->ClearOnUnregisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnUnregisterPlayersComplete);
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
}

void UOnlineSessionSubsystemUnregisterPlayers::HandleCallback_OnUnregisterPlayersComplete(FName SessionName, const TArray<FUniqueNetIdRef>& PlayerIds, bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (!SessionName.IsEqual(this->__Store__SessionName))
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FName(TEXT("")), TArray<FUniqueNetIdRepl>(), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnUnregisterPlayersCompleteDelegate_Handle(this->DelegateHandle_OnUnregisterPlayersComplete);

    this->OnUnregisterPlayersComplete.Broadcast(SessionName, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(PlayerIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), bWasSuccessful);
}

UOnlineSessionSubsystemRegisterLocalPlayer* UOnlineSessionSubsystemRegisterLocalPlayer::RegisterLocalPlayer(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl PlayerId, FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemRegisterLocalPlayer* Node = NewObject<UOnlineSessionSubsystemRegisterLocalPlayer>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__PlayerId = PlayerId;
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemRegisterLocalPlayer::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    if (!(this->__Store__PlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PlayerId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), EOnJoinSessionCompleteResult_::UnknownError);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnRegisterLocalPlayerComplete = FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemRegisterLocalPlayer::HandleCallback_OnRegisterLocalPlayerComplete);

    Handle->RegisterLocalPlayer(this->__Store__PlayerId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionName, __DelegateHandle_OnRegisterLocalPlayerComplete);
}

void UOnlineSessionSubsystemRegisterLocalPlayer::HandleCallback_OnRegisterLocalPlayerComplete(const FUniqueNetId& Param1, EOnJoinSessionCompleteResult::Type Param2)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), EOnJoinSessionCompleteResult_::UnknownError);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnRegisterLocalPlayerComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), (EOnJoinSessionCompleteResult_)(Param2));
}

UOnlineSessionSubsystemUnregisterLocalPlayer* UOnlineSessionSubsystemUnregisterLocalPlayer::UnregisterLocalPlayer(UOnlineSessionSubsystem* Subsystem ,FUniqueNetIdRepl PlayerId, FName SessionName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineSessionSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineSessionSubsystemUnregisterLocalPlayer* Node = NewObject<UOnlineSessionSubsystemUnregisterLocalPlayer>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__PlayerId = PlayerId;
    Node->__Store__SessionName = SessionName;
    return Node;
}

void UOnlineSessionSubsystemUnregisterLocalPlayer::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }
    if (!(this->__Store__PlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PlayerId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnUnregisterLocalPlayerComplete = FOnUnregisterLocalPlayerCompleteDelegate::CreateUObject(this, &UOnlineSessionSubsystemUnregisterLocalPlayer::HandleCallback_OnUnregisterLocalPlayerComplete);

    Handle->UnregisterLocalPlayer(this->__Store__PlayerId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__SessionName, __DelegateHandle_OnUnregisterLocalPlayerComplete);
}

void UOnlineSessionSubsystemUnregisterLocalPlayer::HandleCallback_OnUnregisterLocalPlayerComplete(const FUniqueNetId& Param1, const bool Param2)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnUnregisterLocalPlayerComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2);
}

void UOnlineSessionSubsystem::RemovePlayerFromSession(int32 LocalUserNum, FName SessionName, FUniqueNetIdRepl TargetPlayerId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }
    if (!(TargetPlayerId.IsValid()))
    {
        return ;
    }


    Handle->RemovePlayerFromSession(LocalUserNum, SessionName, TargetPlayerId.GetUniqueNetId().ToSharedRef().Get());
}

int32 UOnlineSessionSubsystem::GetNumSessions()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }


    auto __Result = Handle->GetNumSessions();

    return __Result;
}

void UOnlineSessionSubsystem::DumpSessionState()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DumpSessionState();
}

