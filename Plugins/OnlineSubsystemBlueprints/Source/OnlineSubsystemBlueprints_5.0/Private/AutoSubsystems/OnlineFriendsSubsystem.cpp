// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineFriendsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe> UOnlineFriendsSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetFriendsInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineFriendsSubsystem::IsHandleValid(const TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineFriendsSubsystem::UOnlineFriendsSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineFriendsSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            this->DelegateHandle_OnFriendsChange.Add(Handle->AddOnFriendsChangeDelegate_Handle(
                i,
                FOnFriendsChange::FDelegate::CreateWeakLambda(this, [this, i]() {
                    this->OnFriendsChange.Broadcast(i);
                })
            ));
            this->DelegateHandle_OnOutgoingInviteSent.Add(Handle->AddOnOutgoingInviteSentDelegate_Handle(
                i,
                FOnOutgoingInviteSent::FDelegate::CreateWeakLambda(this, [this, i]() {
                    this->OnOutgoingInviteSent.Broadcast(i);
                })
            ));
            this->DelegateHandle_OnRejectInviteComplete.Add(Handle->AddOnRejectInviteCompleteDelegate_Handle(
                i,
                FOnRejectInviteComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr) {
                    this->OnRejectInviteComplete.Broadcast(i, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
                })
            ));
            this->DelegateHandle_OnDeleteFriendComplete.Add(Handle->AddOnDeleteFriendCompleteDelegate_Handle(
                i,
                FOnDeleteFriendComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr) {
                    this->OnDeleteFriendComplete.Broadcast(i, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
                })
            ));
            this->DelegateHandle_OnBlockedPlayerComplete.Add(Handle->AddOnBlockedPlayerCompleteDelegate_Handle(
                i,
                FOnBlockedPlayerComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr) {
                    this->OnBlockedPlayerComplete.Broadcast(i, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UniqueId), ListName, ErrorStr);
                })
            ));
            this->DelegateHandle_OnUnblockedPlayerComplete.Add(Handle->AddOnUnblockedPlayerCompleteDelegate_Handle(
                i,
                FOnUnblockedPlayerComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr) {
                    this->OnUnblockedPlayerComplete.Broadcast(i, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UniqueId), ListName, ErrorStr);
                })
            ));
            this->DelegateHandle_OnBlockListChange.Add(Handle->AddOnBlockListChangeDelegate_Handle(
                i,
                FOnBlockListChange::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, const FString& ListName) {
                    this->OnBlockListChange.Broadcast(i, ListName);
                })
            ));
        }
        this->DelegateHandle_OnInviteReceived = Handle->AddOnInviteReceivedDelegate_Handle(
            FOnInviteReceived::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FriendId) {
                this->OnInviteReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FriendId));
            })
        );
        this->DelegateHandle_OnInviteAccepted = Handle->AddOnInviteAcceptedDelegate_Handle(
            FOnInviteAccepted::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FriendId) {
                this->OnInviteAccepted.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FriendId));
            })
        );
        this->DelegateHandle_OnInviteRejected = Handle->AddOnInviteRejectedDelegate_Handle(
            FOnInviteRejected::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FriendId) {
                this->OnInviteRejected.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FriendId));
            })
        );
        this->DelegateHandle_OnInviteAborted = Handle->AddOnInviteAbortedDelegate_Handle(
            FOnInviteAborted::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FriendId) {
                this->OnInviteAborted.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FriendId));
            })
        );
        this->DelegateHandle_OnFriendRemoved = Handle->AddOnFriendRemovedDelegate_Handle(
            FOnFriendRemoved::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FUniqueNetId& FriendId) {
                this->OnFriendRemoved.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertDangerousUniqueNetIdToRepl(FriendId));
            })
        );
        this->DelegateHandle_OnQueryRecentPlayersComplete = Handle->AddOnQueryRecentPlayersCompleteDelegate_Handle(
            FOnQueryRecentPlayersComplete::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FString& Namespace, bool bWasSuccessful, const FString& Error) {
                this->OnQueryRecentPlayersComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), Namespace, bWasSuccessful, Error);
            })
        );
        this->DelegateHandle_OnQueryBlockedPlayersComplete = Handle->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
            FOnQueryBlockedPlayersComplete::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error) {
                this->OnQueryBlockedPlayersComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful, Error);
            })
        );
        this->DelegateHandle_OnRecentPlayersAdded = Handle->AddOnRecentPlayersAddedDelegate_Handle(
            FOnRecentPlayersAdded::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const TArray<TSharedRef<FOnlineRecentPlayer>>& AddedPlayers) {
                this->OnRecentPlayersAdded.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertArrayElements<TSharedRef<FOnlineRecentPlayer>, UOnlineRecentPlayerRef*>(AddedPlayers, [](const TSharedRef<FOnlineRecentPlayer>& Val) { return UOnlineRecentPlayerRef::FromRecentPlayer(Val); }));
            })
        );
        this->DelegateHandle_OnFriendSettingsUpdated = Handle->AddOnFriendSettingsUpdatedDelegate_Handle(
            FOnFriendSettingsUpdated::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, bool bWasSuccessful, bool bWasUpdate, const FFriendSettings& Settings, const FString& ErrorStr) {
                this->OnFriendSettingsUpdated.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful, bWasUpdate, FFriendSettingsData::FromNative(Settings), ErrorStr);
            })
        );
    }
}

void UOnlineFriendsSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineFriends, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            Handle->ClearOnFriendsChangeDelegate_Handle(i, this->DelegateHandle_OnFriendsChange[i]);
            Handle->ClearOnOutgoingInviteSentDelegate_Handle(i, this->DelegateHandle_OnOutgoingInviteSent[i]);
            Handle->ClearOnRejectInviteCompleteDelegate_Handle(i, this->DelegateHandle_OnRejectInviteComplete[i]);
            Handle->ClearOnDeleteFriendCompleteDelegate_Handle(i, this->DelegateHandle_OnDeleteFriendComplete[i]);
            Handle->ClearOnBlockedPlayerCompleteDelegate_Handle(i, this->DelegateHandle_OnBlockedPlayerComplete[i]);
            Handle->ClearOnUnblockedPlayerCompleteDelegate_Handle(i, this->DelegateHandle_OnUnblockedPlayerComplete[i]);
            Handle->ClearOnBlockListChangeDelegate_Handle(i, this->DelegateHandle_OnBlockListChange[i]);
        }
        Handle->ClearOnInviteReceivedDelegate_Handle(this->DelegateHandle_OnInviteReceived);
        Handle->ClearOnInviteAcceptedDelegate_Handle(this->DelegateHandle_OnInviteAccepted);
        Handle->ClearOnInviteRejectedDelegate_Handle(this->DelegateHandle_OnInviteRejected);
        Handle->ClearOnInviteAbortedDelegate_Handle(this->DelegateHandle_OnInviteAborted);
        Handle->ClearOnFriendRemovedDelegate_Handle(this->DelegateHandle_OnFriendRemoved);
        Handle->ClearOnQueryRecentPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryRecentPlayersComplete);
        Handle->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryBlockedPlayersComplete);
        Handle->ClearOnRecentPlayersAddedDelegate_Handle(this->DelegateHandle_OnRecentPlayersAdded);
        Handle->ClearOnFriendSettingsUpdatedDelegate_Handle(this->DelegateHandle_OnFriendSettingsUpdated);
        
    }

    Super::BeginDestroy();
}

bool UOnlineFriendsSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineFriendsSubsystemReadFriendsList* UOnlineFriendsSubsystemReadFriendsList::ReadFriendsList(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemReadFriendsList* Node = NewObject<UOnlineFriendsSubsystemReadFriendsList>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemReadFriendsList::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnReadFriendsListComplete = FOnReadFriendsListComplete::CreateUObject(this, &UOnlineFriendsSubsystemReadFriendsList::HandleCallback_OnReadFriendsListComplete);
    
    if (!Handle->ReadFriendsList(this->__Store__LocalUserNum, this->__Store__ListName, __DelegateHandle_OnReadFriendsListComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadFriendsList' did not start successfully"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemReadFriendsList::HandleCallback_OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnReadFriendsListComplete.Broadcast(LocalUserNum, bWasSuccessful, ListName, ErrorStr);
}

UOnlineFriendsSubsystemDeleteFriendsList* UOnlineFriendsSubsystemDeleteFriendsList::DeleteFriendsList(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemDeleteFriendsList* Node = NewObject<UOnlineFriendsSubsystemDeleteFriendsList>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemDeleteFriendsList::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnDeleteFriendsListComplete = FOnDeleteFriendsListComplete::CreateUObject(this, &UOnlineFriendsSubsystemDeleteFriendsList::HandleCallback_OnDeleteFriendsListComplete);
    
    if (!Handle->DeleteFriendsList(this->__Store__LocalUserNum, this->__Store__ListName, __DelegateHandle_OnDeleteFriendsListComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DeleteFriendsList' did not start successfully"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemDeleteFriendsList::HandleCallback_OnDeleteFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnDeleteFriendsListComplete.Broadcast(LocalUserNum, bWasSuccessful, ListName, ErrorStr);
}

UOnlineFriendsSubsystemSendInvite* UOnlineFriendsSubsystemSendInvite::SendInvite(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemSendInvite* Node = NewObject<UOnlineFriendsSubsystemSendInvite>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemSendInvite::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSendInviteComplete = FOnSendInviteComplete::CreateUObject(this, &UOnlineFriendsSubsystemSendInvite::HandleCallback_OnSendInviteComplete);
    
    if (!Handle->SendInvite(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName, __DelegateHandle_OnSendInviteComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'SendInvite' did not start successfully"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemSendInvite::HandleCallback_OnSendInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSendInviteComplete.Broadcast(LocalUserNum, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
}

UOnlineFriendsSubsystemAcceptInvite* UOnlineFriendsSubsystemAcceptInvite::AcceptInvite(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemAcceptInvite* Node = NewObject<UOnlineFriendsSubsystemAcceptInvite>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemAcceptInvite::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnAcceptInviteComplete = FOnAcceptInviteComplete::CreateUObject(this, &UOnlineFriendsSubsystemAcceptInvite::HandleCallback_OnAcceptInviteComplete);
    
    if (!Handle->AcceptInvite(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName, __DelegateHandle_OnAcceptInviteComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'AcceptInvite' did not start successfully"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemAcceptInvite::HandleCallback_OnAcceptInviteComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnAcceptInviteComplete.Broadcast(LocalUserNum, bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
}

UOnlineFriendsSubsystemRejectInvite* UOnlineFriendsSubsystemRejectInvite::RejectInvite(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemRejectInvite* Node = NewObject<UOnlineFriendsSubsystemRejectInvite>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnRejectInviteComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemRejectInvite::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnRejectInviteComplete = Handle->AddOnRejectInviteCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnRejectInviteCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemRejectInvite::HandleCallback_OnRejectInviteComplete));
    
    if (!Handle->RejectInvite(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'RejectInvite' did not start successfully"));
        Handle->ClearOnRejectInviteCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnRejectInviteComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemRejectInvite::HandleCallback_OnRejectInviteComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
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
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnRejectInviteCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnRejectInviteComplete);

    this->OnRejectInviteComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
}

UOnlineFriendsSubsystemSetFriendAlias* UOnlineFriendsSubsystemSetFriendAlias::SetFriendAlias(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName, FString Alias)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemSetFriendAlias* Node = NewObject<UOnlineFriendsSubsystemSetFriendAlias>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    Node->__Store__Alias = Alias;
    return Node;
}

void UOnlineFriendsSubsystemSetFriendAlias::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSetFriendAliasComplete = FOnSetFriendAliasComplete::CreateUObject(this, &UOnlineFriendsSubsystemSetFriendAlias::HandleCallback_OnSetFriendAliasComplete);

    Handle->SetFriendAlias(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName, this->__Store__Alias, __DelegateHandle_OnSetFriendAliasComplete);
}

void UOnlineFriendsSubsystemSetFriendAlias::HandleCallback_OnSetFriendAliasComplete(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnlineError& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSetFriendAliasComplete.Broadcast(LocalUserNum, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, FOnlineErrorInfo::FromNative(Error));
}

UOnlineFriendsSubsystemDeleteFriendAlias* UOnlineFriendsSubsystemDeleteFriendAlias::DeleteFriendAlias(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemDeleteFriendAlias* Node = NewObject<UOnlineFriendsSubsystemDeleteFriendAlias>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemDeleteFriendAlias::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnDeleteFriendAliasComplete = FOnDeleteFriendAliasComplete::CreateUObject(this, &UOnlineFriendsSubsystemDeleteFriendAlias::HandleCallback_OnDeleteFriendAliasComplete);

    Handle->DeleteFriendAlias(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName, __DelegateHandle_OnDeleteFriendAliasComplete);
}

void UOnlineFriendsSubsystemDeleteFriendAlias::HandleCallback_OnDeleteFriendAliasComplete(int32 LocalUserNum, const FUniqueNetId& FriendId, const FString& ListName, const FOnlineError& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FUniqueNetIdRepl(), TEXT(""), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnDeleteFriendAliasComplete.Broadcast(LocalUserNum, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, FOnlineErrorInfo::FromNative(Error));
}

UOnlineFriendsSubsystemDeleteFriend* UOnlineFriendsSubsystemDeleteFriend::DeleteFriend(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemDeleteFriend* Node = NewObject<UOnlineFriendsSubsystemDeleteFriend>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnDeleteFriendComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FriendId = FriendId;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemDeleteFriend::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__FriendId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'FriendId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnDeleteFriendComplete = Handle->AddOnDeleteFriendCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnDeleteFriendCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemDeleteFriend::HandleCallback_OnDeleteFriendComplete));
    
    if (!Handle->DeleteFriend(this->__Store__LocalUserNum, this->__Store__FriendId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ListName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DeleteFriend' did not start successfully"));
        Handle->ClearOnDeleteFriendCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnDeleteFriendComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemDeleteFriend::HandleCallback_OnDeleteFriendComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& FriendId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (LocalUserNumCb != this->__Store__LocalUserNum ||
        FUniqueNetIdRepl(FriendId) != __Store__FriendId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnDeleteFriendCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnDeleteFriendComplete);

    this->OnDeleteFriendComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(FriendId), ListName, ErrorStr);
}

bool UOnlineFriendsSubsystem::GetFriendsList(int32 LocalUserNum, FString ListName, TArray<UOnlineFriendRef*>& OutFriends)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    TArray<TSharedRef<FOnlineFriend>> __StoreTemp__OutFriends;

    auto __Result = Handle->GetFriendsList(LocalUserNum, ListName, __StoreTemp__OutFriends);
    OutFriends = ConvertArrayElements<TSharedRef<FOnlineFriend>, UOnlineFriendRef*>(__StoreTemp__OutFriends, [](const TSharedRef<FOnlineFriend>& Val) { return UOnlineFriendRef::FromFriend(Val); });

    return __Result;
}

UOnlineFriendRef* UOnlineFriendsSubsystem::GetFriend(int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(FriendId.IsValid()))
    {
        return nullptr;
    }


    auto __Result = UOnlineFriendRef::FromFriend(Handle->GetFriend(LocalUserNum, FriendId.GetUniqueNetId().ToSharedRef().Get(), ListName));

    return __Result;
}

bool UOnlineFriendsSubsystem::IsFriend(int32 LocalUserNum, FUniqueNetIdRepl FriendId, FString ListName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(FriendId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsFriend(LocalUserNum, FriendId.GetUniqueNetId().ToSharedRef().Get(), ListName);

    return __Result;
}

UOnlineFriendsSubsystemAddRecentPlayers* UOnlineFriendsSubsystemAddRecentPlayers::AddRecentPlayers(UOnlineFriendsSubsystem* Subsystem ,FUniqueNetIdRepl UserId, TArray<FReportPlayedWithUserInfo> InRecentPlayers, FString ListName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemAddRecentPlayers* Node = NewObject<UOnlineFriendsSubsystemAddRecentPlayers>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__InRecentPlayers = InRecentPlayers;
    Node->__Store__ListName = ListName;
    return Node;
}

void UOnlineFriendsSubsystemAddRecentPlayers::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FReportPlayedWithUserInfo>(this->__Store__InRecentPlayers, [](const FReportPlayedWithUserInfo& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'InRecentPlayers' was not valid"));
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

    auto __DelegateHandle_OnAddRecentPlayersComplete = FOnAddRecentPlayersComplete::CreateUObject(this, &UOnlineFriendsSubsystemAddRecentPlayers::HandleCallback_OnAddRecentPlayersComplete);

    Handle->AddRecentPlayers(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FReportPlayedWithUserInfo, FReportPlayedWithUser>(this->__Store__InRecentPlayers, [](const FReportPlayedWithUserInfo& Val) { return Val.ToNative(); }), this->__Store__ListName, __DelegateHandle_OnAddRecentPlayersComplete);
}

void UOnlineFriendsSubsystemAddRecentPlayers::HandleCallback_OnAddRecentPlayersComplete(const FUniqueNetId& UserId, const FOnlineError& Error)
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

    this->OnAddRecentPlayersComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), FOnlineErrorInfo::FromNative(Error));
}

UOnlineFriendsSubsystemQueryRecentPlayers* UOnlineFriendsSubsystemQueryRecentPlayers::QueryRecentPlayers(UOnlineFriendsSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString Namespace)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemQueryRecentPlayers* Node = NewObject<UOnlineFriendsSubsystemQueryRecentPlayers>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnQueryRecentPlayersComplete|%s"), *UserId.ToString());
    Node->__Store__UserId = UserId;
    Node->__Store__Namespace = Namespace;
    return Node;
}

void UOnlineFriendsSubsystemQueryRecentPlayers::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnQueryRecentPlayersComplete = Handle->AddOnQueryRecentPlayersCompleteDelegate_Handle(
        FOnQueryRecentPlayersCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemQueryRecentPlayers::HandleCallback_OnQueryRecentPlayersComplete));
    
    if (!Handle->QueryRecentPlayers(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__Namespace))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'QueryRecentPlayers' did not start successfully"));
        Handle->ClearOnQueryRecentPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryRecentPlayersComplete);
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemQueryRecentPlayers::HandleCallback_OnQueryRecentPlayersComplete(const FUniqueNetId& UserId, const FString& Namespace, bool bWasSuccessful, const FString& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
        return;
    }
    if (FUniqueNetIdRepl(UserId) != __Store__UserId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), TEXT(""), false, TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnQueryRecentPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryRecentPlayersComplete);

    this->OnQueryRecentPlayersComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), Namespace, bWasSuccessful, Error);
}

bool UOnlineFriendsSubsystem::GetRecentPlayers(FUniqueNetIdRepl UserId, FString Namespace, TArray<UOnlineRecentPlayerRef*>& OutRecentPlayers)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid()))
    {
        return false;
    }

    TArray<TSharedRef<FOnlineRecentPlayer>> __StoreTemp__OutRecentPlayers;

    auto __Result = Handle->GetRecentPlayers(UserId.GetUniqueNetId().ToSharedRef().Get(), Namespace, __StoreTemp__OutRecentPlayers);
    OutRecentPlayers = ConvertArrayElements<TSharedRef<FOnlineRecentPlayer>, UOnlineRecentPlayerRef*>(__StoreTemp__OutRecentPlayers, [](const TSharedRef<FOnlineRecentPlayer>& Val) { return UOnlineRecentPlayerRef::FromRecentPlayer(Val); });

    return __Result;
}

void UOnlineFriendsSubsystem::DumpRecentPlayers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DumpRecentPlayers();
}

UOnlineFriendsSubsystemBlockPlayer* UOnlineFriendsSubsystemBlockPlayer::BlockPlayer(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl PlayerId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemBlockPlayer* Node = NewObject<UOnlineFriendsSubsystemBlockPlayer>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnBlockedPlayerComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__PlayerId = PlayerId;
    return Node;
}

void UOnlineFriendsSubsystemBlockPlayer::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__PlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PlayerId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnBlockedPlayerComplete = Handle->AddOnBlockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnBlockedPlayerCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemBlockPlayer::HandleCallback_OnBlockedPlayerComplete));
    
    if (!Handle->BlockPlayer(this->__Store__LocalUserNum, this->__Store__PlayerId.GetUniqueNetId().ToSharedRef().Get()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'BlockPlayer' did not start successfully"));
        Handle->ClearOnBlockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnBlockedPlayerComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemBlockPlayer::HandleCallback_OnBlockedPlayerComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (LocalUserNumCb != this->__Store__LocalUserNum ||
        FUniqueNetIdRepl(UniqueId) != __Store__PlayerId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnBlockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnBlockedPlayerComplete);

    this->OnBlockedPlayerComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UniqueId), ListName, ErrorStr);
}

UOnlineFriendsSubsystemUnblockPlayer* UOnlineFriendsSubsystemUnblockPlayer::UnblockPlayer(UOnlineFriendsSubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl PlayerId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemUnblockPlayer* Node = NewObject<UOnlineFriendsSubsystemUnblockPlayer>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnUnblockedPlayerComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__PlayerId = PlayerId;
    return Node;
}

void UOnlineFriendsSubsystemUnblockPlayer::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (!(this->__Store__PlayerId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'PlayerId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnUnblockedPlayerComplete = Handle->AddOnUnblockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnUnblockedPlayerCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemUnblockPlayer::HandleCallback_OnUnblockedPlayerComplete));
    
    if (!Handle->UnblockPlayer(this->__Store__LocalUserNum, this->__Store__PlayerId.GetUniqueNetId().ToSharedRef().Get()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UnblockPlayer' did not start successfully"));
        Handle->ClearOnUnblockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnUnblockedPlayerComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemUnblockPlayer::HandleCallback_OnUnblockedPlayerComplete(int32 LocalUserNumCb, bool bWasSuccessful, const FUniqueNetId& UniqueId, const FString& ListName, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (LocalUserNumCb != this->__Store__LocalUserNum ||
        FUniqueNetIdRepl(UniqueId) != __Store__PlayerId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnUnblockedPlayerCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnUnblockedPlayerComplete);

    this->OnUnblockedPlayerComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UniqueId), ListName, ErrorStr);
}

UOnlineFriendsSubsystemQueryBlockedPlayers* UOnlineFriendsSubsystemQueryBlockedPlayers::QueryBlockedPlayers(UOnlineFriendsSubsystem* Subsystem ,FUniqueNetIdRepl UserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemQueryBlockedPlayers* Node = NewObject<UOnlineFriendsSubsystemQueryBlockedPlayers>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnQueryBlockedPlayersComplete|%s"), *UserId.ToString());
    Node->__Store__UserId = UserId;
    return Node;
}

void UOnlineFriendsSubsystemQueryBlockedPlayers::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnQueryBlockedPlayersComplete = Handle->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
        FOnQueryBlockedPlayersCompleteDelegate::CreateUObject(this, &UOnlineFriendsSubsystemQueryBlockedPlayers::HandleCallback_OnQueryBlockedPlayersComplete));
    
    if (!Handle->QueryBlockedPlayers(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'QueryBlockedPlayers' did not start successfully"));
        Handle->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryBlockedPlayersComplete);
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemQueryBlockedPlayers::HandleCallback_OnQueryBlockedPlayersComplete(const FUniqueNetId& UserId, bool bWasSuccessful, const FString& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    if (FUniqueNetIdRepl(UserId) != __Store__UserId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(this->DelegateHandle_OnQueryBlockedPlayersComplete);

    this->OnQueryBlockedPlayersComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful, Error);
}

bool UOnlineFriendsSubsystem::GetBlockedPlayers(FUniqueNetIdRepl UserId, TArray<UOnlineUserRef*>& OutBlockedPlayers)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid()))
    {
        return false;
    }

    TArray<TSharedRef<FOnlineBlockedPlayer>> __StoreTemp__OutBlockedPlayers;

    auto __Result = Handle->GetBlockedPlayers(UserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutBlockedPlayers);
    OutBlockedPlayers = ConvertArrayElements<TSharedRef<FOnlineBlockedPlayer>, UOnlineUserRef*>(__StoreTemp__OutBlockedPlayers, [](const TSharedRef<FOnlineBlockedPlayer>& Val) { return UOnlineUserRef::FromBlockedPlayer(Val); });

    return __Result;
}

void UOnlineFriendsSubsystem::DumpBlockedPlayers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DumpBlockedPlayers();
}

UOnlineFriendsSubsystemQueryFriendSettings* UOnlineFriendsSubsystemQueryFriendSettings::QueryFriendSettings(UOnlineFriendsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemQueryFriendSettings* Node = NewObject<UOnlineFriendsSubsystemQueryFriendSettings>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    return Node;
}

void UOnlineFriendsSubsystemQueryFriendSettings::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, false, FFriendSettingsData(), TEXT(""));
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, false, FFriendSettingsData(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, false, FFriendSettingsData(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSettingsOperationComplete = FOnSettingsOperationComplete::CreateUObject(this, &UOnlineFriendsSubsystemQueryFriendSettings::HandleCallback_OnSettingsOperationComplete);

    Handle->QueryFriendSettings(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnSettingsOperationComplete);
}

void UOnlineFriendsSubsystemQueryFriendSettings::HandleCallback_OnSettingsOperationComplete(const FUniqueNetId& UserId, bool bWasSuccessful, bool bWasUpdate, const FFriendSettings& Settings, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, false, FFriendSettingsData(), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, false, FFriendSettingsData(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSettingsOperationComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful, bWasUpdate, FFriendSettingsData::FromNative(Settings), ErrorStr);
}

bool UOnlineFriendsSubsystem::GetFriendSettings(FUniqueNetIdRepl UserId, TMap<FString, FOnlineFriendSettingsSourceDataConfig>& OutSettings)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UserId.IsValid()))
    {
        return false;
    }

    TMap<FString, TSharedRef<FOnlineFriendSettingsSourceData>> __StoreTemp__OutSettings;

    auto __Result = Handle->GetFriendSettings(UserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutSettings);
    OutSettings = ConvertMapElements<FString, FString, TSharedRef<FOnlineFriendSettingsSourceData>, FOnlineFriendSettingsSourceDataConfig>(__StoreTemp__OutSettings, [](const FString& Val) { return Val; }, [](const TSharedRef<FOnlineFriendSettingsSourceData>& Val) { return FOnlineFriendSettingsSourceDataConfig::FromNative(Val); });

    return __Result;
}

UOnlineFriendsSubsystemSetFriendSettings* UOnlineFriendsSubsystemSetFriendSettings::SetFriendSettings(UOnlineFriendsSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString Source, bool bNeverShowAgain)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineFriendsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineFriendsSubsystemSetFriendSettings* Node = NewObject<UOnlineFriendsSubsystemSetFriendSettings>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__Source = Source;
    Node->__Store__bNeverShowAgain = bNeverShowAgain;
    return Node;
}

void UOnlineFriendsSubsystemSetFriendSettings::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnSetFriendSettingsComplete = FOnSetFriendSettingsComplete::CreateUObject(this, &UOnlineFriendsSubsystemSetFriendSettings::HandleCallback_OnSetFriendSettingsComplete);
    
    if (!Handle->SetFriendSettings(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__Source, this->__Store__bNeverShowAgain, __DelegateHandle_OnSetFriendSettingsComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'SetFriendSettings' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
}

void UOnlineFriendsSubsystemSetFriendSettings::HandleCallback_OnSetFriendSettingsComplete(const FUniqueNetId& Param1, bool Param2, const FString& Param3)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false, TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnSetFriendSettingsComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3);
}

