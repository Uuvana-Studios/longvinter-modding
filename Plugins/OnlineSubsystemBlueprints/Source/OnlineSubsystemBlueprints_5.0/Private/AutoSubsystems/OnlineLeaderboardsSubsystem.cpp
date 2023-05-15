// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineLeaderboardsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> UOnlineLeaderboardsSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetLeaderboardsInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineLeaderboardsSubsystem::IsHandleValid(const TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineLeaderboardsSubsystem::UOnlineLeaderboardsSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineLeaderboardsSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnLeaderboardReadComplete = Handle->AddOnLeaderboardReadCompleteDelegate_Handle(
            FOnLeaderboardReadComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful) {
                this->OnLeaderboardReadComplete.Broadcast(bWasSuccessful);
            })
        );
        this->DelegateHandle_OnLeaderboardFlushComplete = Handle->AddOnLeaderboardFlushCompleteDelegate_Handle(
            FOnLeaderboardFlushComplete::FDelegate::CreateWeakLambda(this, [this](const FName& SessionName, bool bWasSuccessful) {
                this->OnLeaderboardFlushComplete.Broadcast(SessionName, bWasSuccessful);
            })
        );
    }
}

void UOnlineLeaderboardsSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineLeaderboards, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);
        Handle->ClearOnLeaderboardFlushCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardFlushComplete);
        
    }

    Super::BeginDestroy();
}

bool UOnlineLeaderboardsSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineLeaderboardsSubsystemReadLeaderboards* UOnlineLeaderboardsSubsystemReadLeaderboards::ReadLeaderboards(UOnlineLeaderboardsSubsystem* Subsystem ,TArray<FUniqueNetIdRepl> Players, UOnlineLeaderboardRead* ReadObject)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLeaderboardsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLeaderboardsSubsystemReadLeaderboards* Node = NewObject<UOnlineLeaderboardsSubsystemReadLeaderboards>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__Players = Players;
    Node->__Store__ReadObject = ReadObject;
    return Node;
}

void UOnlineLeaderboardsSubsystemReadLeaderboards::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Players' was not valid"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!((IsValid(this->__Store__ReadObject))))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ReadObject' was not valid"));
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

    this->DelegateHandle_OnLeaderboardReadComplete = Handle->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UOnlineLeaderboardsSubsystemReadLeaderboards::HandleCallback_OnLeaderboardReadComplete));
    
    if (!Handle->ReadLeaderboards(ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__Players, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), this->__Store__ReadObject->GetLeaderboard()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadLeaderboards' did not start successfully"));
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineLeaderboardsSubsystemReadLeaderboards::HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    EOnlineAsyncTaskState::Type CurrentTaskState = this->__Store__ReadObject->GetLeaderboard()->ReadState;
    if (CurrentTaskState == EOnlineAsyncTaskState::NotStarted ||
        CurrentTaskState == EOnlineAsyncTaskState::InProgress)
    {
        // This task is still in-progress, so this is not the completion event we're interested in.
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
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);

    this->OnLeaderboardReadComplete.Broadcast(bWasSuccessful);
}

UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends* UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends::ReadLeaderboardsForFriends(UOnlineLeaderboardsSubsystem* Subsystem ,int32 LocalUserNum, UOnlineLeaderboardRead* ReadObject)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLeaderboardsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends* Node = NewObject<UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__ReadObject = ReadObject;
    return Node;
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!((IsValid(this->__Store__ReadObject))))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ReadObject' was not valid"));
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

    this->DelegateHandle_OnLeaderboardReadComplete = Handle->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends::HandleCallback_OnLeaderboardReadComplete));
    
    if (!Handle->ReadLeaderboardsForFriends(this->__Store__LocalUserNum, this->__Store__ReadObject->GetLeaderboard()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadLeaderboardsForFriends' did not start successfully"));
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsForFriends::HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    EOnlineAsyncTaskState::Type CurrentTaskState = this->__Store__ReadObject->GetLeaderboard()->ReadState;
    if (CurrentTaskState == EOnlineAsyncTaskState::NotStarted ||
        CurrentTaskState == EOnlineAsyncTaskState::InProgress)
    {
        // This task is still in-progress, so this is not the completion event we're interested in.
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
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);

    this->OnLeaderboardReadComplete.Broadcast(bWasSuccessful);
}

UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank* UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank::ReadLeaderboardsAroundRank(UOnlineLeaderboardsSubsystem* Subsystem ,int32 Rank, int64 Range, UOnlineLeaderboardRead* ReadObject)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLeaderboardsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank* Node = NewObject<UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__Rank = Rank;
    Node->__Store__Range = Range;
    Node->__Store__ReadObject = ReadObject;
    return Node;
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!((IsValid(this->__Store__ReadObject))))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ReadObject' was not valid"));
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

    this->DelegateHandle_OnLeaderboardReadComplete = Handle->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank::HandleCallback_OnLeaderboardReadComplete));
    
    if (!Handle->ReadLeaderboardsAroundRank(this->__Store__Rank, (uint32)(this->__Store__Range), this->__Store__ReadObject->GetLeaderboard()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadLeaderboardsAroundRank' did not start successfully"));
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsAroundRank::HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    EOnlineAsyncTaskState::Type CurrentTaskState = this->__Store__ReadObject->GetLeaderboard()->ReadState;
    if (CurrentTaskState == EOnlineAsyncTaskState::NotStarted ||
        CurrentTaskState == EOnlineAsyncTaskState::InProgress)
    {
        // This task is still in-progress, so this is not the completion event we're interested in.
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
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);

    this->OnLeaderboardReadComplete.Broadcast(bWasSuccessful);
}

UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser* UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser::ReadLeaderboardsAroundUser(UOnlineLeaderboardsSubsystem* Subsystem ,FUniqueNetIdRepl Player, int64 Range, UOnlineLeaderboardRead* ReadObject)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineLeaderboardsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser* Node = NewObject<UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__Player = Player;
    Node->__Store__Range = Range;
    Node->__Store__ReadObject = ReadObject;
    return Node;
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!(this->__Store__Player.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Player' was not valid"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!((IsValid(this->__Store__ReadObject))))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ReadObject' was not valid"));
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

    this->DelegateHandle_OnLeaderboardReadComplete = Handle->AddOnLeaderboardReadCompleteDelegate_Handle(
        FOnLeaderboardReadCompleteDelegate::CreateUObject(this, &UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser::HandleCallback_OnLeaderboardReadComplete));
    
    if (!Handle->ReadLeaderboardsAroundUser(this->__Store__Player.GetUniqueNetId().ToSharedRef(), (uint32)(this->__Store__Range), this->__Store__ReadObject->GetLeaderboard()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadLeaderboardsAroundUser' did not start successfully"));
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineLeaderboardsSubsystemReadLeaderboardsAroundUser::HandleCallback_OnLeaderboardReadComplete(bool bWasSuccessful)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    EOnlineAsyncTaskState::Type CurrentTaskState = this->__Store__ReadObject->GetLeaderboard()->ReadState;
    if (CurrentTaskState == EOnlineAsyncTaskState::NotStarted ||
        CurrentTaskState == EOnlineAsyncTaskState::InProgress)
    {
        // This task is still in-progress, so this is not the completion event we're interested in.
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
        Handle->ClearOnLeaderboardReadCompleteDelegate_Handle(this->DelegateHandle_OnLeaderboardReadComplete);

    this->OnLeaderboardReadComplete.Broadcast(bWasSuccessful);
}

void UOnlineLeaderboardsSubsystem::FreeStats(UOnlineLeaderboardRead* ReadObject)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }
    if (!((IsValid(ReadObject))))
    {
        return ;
    }


    Handle->FreeStats(*ReadObject->GetLeaderboard());
}

bool UOnlineLeaderboardsSubsystem::WriteLeaderboards(FName SessionName, FUniqueNetIdRepl Player, UOnlineLeaderboardWrite* WriteObject)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(Player.IsValid()))
    {
        return false;
    }
    if (!((IsValid(WriteObject))))
    {
        return false;
    }


    auto __Result = Handle->WriteLeaderboards(SessionName, Player.GetUniqueNetId().ToSharedRef().Get(), *WriteObject->ToNative());

    return __Result;
}

bool UOnlineLeaderboardsSubsystem::FlushLeaderboards(FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->FlushLeaderboards(SessionName);

    return __Result;
}

