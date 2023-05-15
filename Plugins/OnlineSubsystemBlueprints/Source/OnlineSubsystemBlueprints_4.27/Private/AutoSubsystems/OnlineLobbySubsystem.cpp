// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineLobbySubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe> UOnlineLobbySubsystem::GetHandle()
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

    TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Online::GetLobbyInterface(Subsystem);
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineLobbySubsystem::IsHandleValid(const TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineLobbySubsystem::UOnlineLobbySubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineLobbySubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnLobbyUpdate = Handle->AddOnLobbyUpdateDelegate_Handle(
            FOnLobbyUpdate::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FOnlineLobbyId& LobbyId) {
                this->OnLobbyUpdate.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ULobbyId::FromNative(LobbyId));
            })
        );
        this->DelegateHandle_OnLobbyDelete = Handle->AddOnLobbyDeleteDelegate_Handle(
            FOnLobbyDelete::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FOnlineLobbyId& LobbyId) {
                this->OnLobbyDelete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ULobbyId::FromNative(LobbyId));
            })
        );
        this->DelegateHandle_OnMemberConnect = Handle->AddOnMemberConnectDelegate_Handle(
            FOnMemberConnect::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FOnlineLobbyId& LobbyId, const FUniqueNetId& MemberId) {
                this->OnMemberConnect.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ULobbyId::FromNative(LobbyId), ConvertDangerousUniqueNetIdToRepl(MemberId));
            })
        );
        this->DelegateHandle_OnMemberUpdate = Handle->AddOnMemberUpdateDelegate_Handle(
            FOnMemberUpdate::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FOnlineLobbyId& LobbyId, const FUniqueNetId& MemberId) {
                this->OnMemberUpdate.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ULobbyId::FromNative(LobbyId), ConvertDangerousUniqueNetIdToRepl(MemberId));
            })
        );
        this->DelegateHandle_OnMemberDisconnect = Handle->AddOnMemberDisconnectDelegate_Handle(
            FOnMemberDisconnect::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& UserId, const FOnlineLobbyId& LobbyId, const FUniqueNetId& MemberId, bool bWasKicked) {
                this->OnMemberDisconnect.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ULobbyId::FromNative(LobbyId), ConvertDangerousUniqueNetIdToRepl(MemberId), bWasKicked);
            })
        );
    }
}

void UOnlineLobbySubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnLobbyUpdateDelegate_Handle(this->DelegateHandle_OnLobbyUpdate);
        Handle->ClearOnLobbyDeleteDelegate_Handle(this->DelegateHandle_OnLobbyDelete);
        Handle->ClearOnMemberConnectDelegate_Handle(this->DelegateHandle_OnMemberConnect);
        Handle->ClearOnMemberUpdateDelegate_Handle(this->DelegateHandle_OnMemberUpdate);
        Handle->ClearOnMemberDisconnectDelegate_Handle(this->DelegateHandle_OnMemberDisconnect);
        
    }

    Super::BeginDestroy();
}

bool UOnlineLobbySubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineLobbyTransaction* UOnlineLobbySubsystem::MakeCreateLobbyTransaction(FUniqueNetIdRepl UserId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(UserId.IsValid()))
    {
        return nullptr;
    }


    auto __Result = UOnlineLobbyTransaction::FromNative(Handle->MakeCreateLobbyTransaction(UserId.GetUniqueNetId().ToSharedRef().Get()));

    return __Result;
}

UOnlineLobbyTransaction* UOnlineLobbySubsystem::MakeUpdateLobbyTransaction(FUniqueNetIdRepl UserId, ULobbyId* LobbyId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(UserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return nullptr;
    }


    auto __Result = UOnlineLobbyTransaction::FromNative(Handle->MakeUpdateLobbyTransaction(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative()));

    return __Result;
}

UOnlineLobbyMemberTransaction* UOnlineLobbySubsystem::MakeUpdateLobbyMemberTransaction(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }
    if (!(UserId.IsValid()))
    {
        return nullptr;
    }
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return nullptr;
    }
    if (!(MemberId.IsValid()))
    {
        return nullptr;
    }


    auto __Result = UOnlineLobbyMemberTransaction::FromNative(Handle->MakeUpdateLobbyMemberTransaction(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative(), MemberId.GetUniqueNetId().ToSharedRef().Get()));

    return __Result;
}

UOnlineLobbySubsystemCreateLobby* UOnlineLobbySubsystemCreateLobby::CreateLobby(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, UOnlineLobbyTransaction* Transaction)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemCreateLobby* Node = NewObject<UOnlineLobbySubsystemCreateLobby>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__Transaction = Transaction;
    return Node;
}

void UOnlineLobbySubsystemCreateLobby::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (!(this->__Store__Transaction->IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Transaction' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyCreateOrConnectComplete = FOnLobbyCreateOrConnectComplete::CreateUObject(this, &UOnlineLobbySubsystemCreateLobby::HandleCallback_OnLobbyCreateOrConnectComplete);
    
    if (!Handle->CreateLobby(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), *this->__Store__Transaction->ToNative(), __DelegateHandle_OnLobbyCreateOrConnectComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'CreateLobby' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
}

void UOnlineLobbySubsystemCreateLobby::HandleCallback_OnLobbyCreateOrConnectComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TSharedPtr<class FOnlineLobby>& Lobby)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyCreateOrConnectComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId), ULobby::FromNative(Lobby));
}

UOnlineLobbySubsystemUpdateLobby* UOnlineLobbySubsystemUpdateLobby::UpdateLobby(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, UOnlineLobbyTransaction* Transaction)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemUpdateLobby* Node = NewObject<UOnlineLobbySubsystemUpdateLobby>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    Node->__Store__Transaction = Transaction;
    return Node;
}

void UOnlineLobbySubsystemUpdateLobby::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__Transaction->IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Transaction' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyOperationComplete = FOnLobbyOperationComplete::CreateUObject(this, &UOnlineLobbySubsystemUpdateLobby::HandleCallback_OnLobbyOperationComplete);
    
    if (!Handle->UpdateLobby(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), *this->__Store__Transaction->ToNative(), __DelegateHandle_OnLobbyOperationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UpdateLobby' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineLobbySubsystemUpdateLobby::HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyOperationComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId));
}

UOnlineLobbySubsystemDeleteLobby* UOnlineLobbySubsystemDeleteLobby::DeleteLobby(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemDeleteLobby* Node = NewObject<UOnlineLobbySubsystemDeleteLobby>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    return Node;
}

void UOnlineLobbySubsystemDeleteLobby::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyOperationComplete = FOnLobbyOperationComplete::CreateUObject(this, &UOnlineLobbySubsystemDeleteLobby::HandleCallback_OnLobbyOperationComplete);
    
    if (!Handle->DeleteLobby(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), __DelegateHandle_OnLobbyOperationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DeleteLobby' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineLobbySubsystemDeleteLobby::HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyOperationComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId));
}

UOnlineLobbySubsystemConnectLobby* UOnlineLobbySubsystemConnectLobby::ConnectLobby(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemConnectLobby* Node = NewObject<UOnlineLobbySubsystemConnectLobby>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    return Node;
}

void UOnlineLobbySubsystemConnectLobby::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyCreateOrConnectComplete = FOnLobbyCreateOrConnectComplete::CreateUObject(this, &UOnlineLobbySubsystemConnectLobby::HandleCallback_OnLobbyCreateOrConnectComplete);
    
    if (!Handle->ConnectLobby(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), __DelegateHandle_OnLobbyCreateOrConnectComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ConnectLobby' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
}

void UOnlineLobbySubsystemConnectLobby::HandleCallback_OnLobbyCreateOrConnectComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TSharedPtr<class FOnlineLobby>& Lobby)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), nullptr);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyCreateOrConnectComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId), ULobby::FromNative(Lobby));
}

UOnlineLobbySubsystemDisconnectLobby* UOnlineLobbySubsystemDisconnectLobby::DisconnectLobby(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemDisconnectLobby* Node = NewObject<UOnlineLobbySubsystemDisconnectLobby>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    return Node;
}

void UOnlineLobbySubsystemDisconnectLobby::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyOperationComplete = FOnLobbyOperationComplete::CreateUObject(this, &UOnlineLobbySubsystemDisconnectLobby::HandleCallback_OnLobbyOperationComplete);
    
    if (!Handle->DisconnectLobby(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), __DelegateHandle_OnLobbyOperationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DisconnectLobby' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineLobbySubsystemDisconnectLobby::HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyOperationComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId));
}

UOnlineLobbySubsystemUpdateMemberSelf* UOnlineLobbySubsystemUpdateMemberSelf::UpdateMemberSelf(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, UOnlineLobbyMemberTransaction* Transaction)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemUpdateMemberSelf* Node = NewObject<UOnlineLobbySubsystemUpdateMemberSelf>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    Node->__Store__Transaction = Transaction;
    return Node;
}

void UOnlineLobbySubsystemUpdateMemberSelf::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__Transaction->IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Transaction' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyOperationComplete = FOnLobbyOperationComplete::CreateUObject(this, &UOnlineLobbySubsystemUpdateMemberSelf::HandleCallback_OnLobbyOperationComplete);
    
    if (!Handle->UpdateMemberSelf(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), *this->__Store__Transaction->ToNative(), __DelegateHandle_OnLobbyOperationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'UpdateMemberSelf' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineLobbySubsystemUpdateMemberSelf::HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyOperationComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId));
}

bool UOnlineLobbySubsystem::GetMemberCount(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, int32& OutMemberCount)
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
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return false;
    }

    int32 __StoreTemp__OutMemberCount;

    auto __Result = Handle->GetMemberCount(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative(), __StoreTemp__OutMemberCount);
    OutMemberCount = __StoreTemp__OutMemberCount;

    return __Result;
}

bool UOnlineLobbySubsystem::GetMemberUserId(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, int32 MemberIndex, FUniqueNetIdRepl& OutMemberId)
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
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return false;
    }

    TSharedPtr<const FUniqueNetId> __StoreTemp__OutMemberId;

    auto __Result = Handle->GetMemberUserId(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative(), MemberIndex, __StoreTemp__OutMemberId);
    OutMemberId = FUniqueNetIdRepl(__StoreTemp__OutMemberId);

    return __Result;
}

bool UOnlineLobbySubsystem::GetMemberMetadataValue(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId, FString MetadataKey, FVariantDataBP& OutMetadataValue)
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
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return false;
    }
    if (!(MemberId.IsValid()))
    {
        return false;
    }

    FVariantData __StoreTemp__OutMetadataValue;

    auto __Result = Handle->GetMemberMetadataValue(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative(), MemberId.GetUniqueNetId().ToSharedRef().Get(), MetadataKey, __StoreTemp__OutMetadataValue);
    OutMetadataValue = FVariantDataBP::FromNative(__StoreTemp__OutMetadataValue);

    return __Result;
}

UOnlineLobbySubsystemSearch* UOnlineLobbySubsystemSearch::Search(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, FOnlineLobbySearchQueryBP Query)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemSearch* Node = NewObject<UOnlineLobbySubsystemSearch>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__Query = Query;
    return Node;
}

void UOnlineLobbySubsystemSearch::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }
    if (!(true))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Query' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbySearchComplete = FOnLobbySearchComplete::CreateUObject(this, &UOnlineLobbySubsystemSearch::HandleCallback_OnLobbySearchComplete);
    
    if (!Handle->Search(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__Query.ToNative(), __DelegateHandle_OnLobbySearchComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'Search' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }
}

void UOnlineLobbySubsystemSearch::HandleCallback_OnLobbySearchComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TArray<TSharedRef<const FOnlineLobbyId>>& Lobbies)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl(), TArray<ULobbyId*>());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbySearchComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId), ConvertArrayElements<TSharedRef<const FOnlineLobbyId>, ULobbyId*>(Lobbies, [](const TSharedRef<const FOnlineLobbyId>& Val) { return ULobbyId::FromNative(Val); }));
}

bool UOnlineLobbySubsystem::GetLobbyMetadataValue(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FString MetadataKey, FVariantDataBP& OutMetadataValue)
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
    if (!((IsValid(LobbyId) && LobbyId->IsValid())))
    {
        return false;
    }

    FVariantData __StoreTemp__OutMetadataValue;

    auto __Result = Handle->GetLobbyMetadataValue(UserId.GetUniqueNetId().ToSharedRef().Get(), LobbyId->ToNative(), MetadataKey, __StoreTemp__OutMetadataValue);
    OutMetadataValue = FVariantDataBP::FromNative(__StoreTemp__OutMetadataValue);

    return __Result;
}

ULobbyId* UOnlineLobbySubsystem::ParseSerializedLobbyId(FString InLobbyId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return nullptr;
    }


    auto __Result = ULobbyId::FromNative(Handle->ParseSerializedLobbyId(InLobbyId));

    return __Result;
}

UOnlineLobbySubsystemKickMember* UOnlineLobbySubsystemKickMember::KickMember(UOnlineLobbySubsystem* Subsystem ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLobbySubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLobbySubsystemKickMember* Node = NewObject<UOnlineLobbySubsystemKickMember>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__LobbyId = LobbyId;
    Node->__Store__MemberId = MemberId;
    return Node;
}

void UOnlineLobbySubsystemKickMember::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!((IsValid(this->__Store__LobbyId) && this->__Store__LobbyId->IsValid())))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LobbyId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__MemberId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'MemberId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLobbyOperationComplete = FOnLobbyOperationComplete::CreateUObject(this, &UOnlineLobbySubsystemKickMember::HandleCallback_OnLobbyOperationComplete);
    
    if (!Handle->KickMember(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__LobbyId->ToNative(), this->__Store__MemberId.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnLobbyOperationComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'KickMember' did not start successfully"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
}

void UOnlineLobbySubsystemKickMember::HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLobbyOperationComplete.Broadcast(FOnlineErrorInfo::FromNative(Error), ConvertDangerousUniqueNetIdToRepl(UserId));
}

