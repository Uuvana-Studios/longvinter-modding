// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineStatsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe> UOnlineStatsSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetStatsInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineStatsSubsystem::IsHandleValid(const TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineStatsSubsystem::UOnlineStatsSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineStatsSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
    }
}

void UOnlineStatsSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineStats, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineStatsSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineStatsSubsystemQueryStats* UOnlineStatsSubsystemQueryStats::QueryStats(UOnlineStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FUniqueNetIdRepl> StatUsers, TArray<FString> StatNames)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineStatsSubsystemQueryStats* Node = NewObject<UOnlineStatsSubsystemQueryStats>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__StatUsers = StatUsers;
    Node->__Store__StatNames = StatNames;
    return Node;
}

void UOnlineStatsSubsystemQueryStats::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__StatUsers, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'StatUsers' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__StatNames, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'StatNames' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnlineStatsQueryUsersStatsComplete = FOnlineStatsQueryUsersStatsComplete::CreateUObject(this, &UOnlineStatsSubsystemQueryStats::HandleCallback_OnlineStatsQueryUsersStatsComplete);

    Handle->QueryStats(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef(), ConvertArrayElements<FUniqueNetIdRepl, TSharedRef<const FUniqueNetId>>(this->__Store__StatUsers, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); }), ConvertArrayElements<FString, FString>(this->__Store__StatNames, [](const FString& Val) { return Val; }), __DelegateHandle_OnlineStatsQueryUsersStatsComplete);
}

void UOnlineStatsSubsystemQueryStats::HandleCallback_OnlineStatsQueryUsersStatsComplete(const FOnlineError& ResultState, const TArray<TSharedRef<const FOnlineStatsUserStats>>& UsersStatsResult)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo(), TArray<FOnlineStatsUserStatsBP>());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnlineStatsQueryUsersStatsComplete.Broadcast(FOnlineErrorInfo::FromNative(ResultState), ConvertArrayElements<TSharedRef<const FOnlineStatsUserStats>, FOnlineStatsUserStatsBP>(UsersStatsResult, [](const TSharedRef<const FOnlineStatsUserStats>& Val) { return FOnlineStatsUserStatsBP::FromNative(Val); }));
}

FOnlineStatsUserStatsBP UOnlineStatsSubsystem::GetStats(FUniqueNetIdRepl StatsUserId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FOnlineStatsUserStatsBP();
    }
    if (!(StatsUserId.IsValid()))
    {
        return FOnlineStatsUserStatsBP();
    }


    auto __Result = FOnlineStatsUserStatsBP::FromNative(Handle->GetStats(StatsUserId.GetUniqueNetId().ToSharedRef()));

    return __Result;
}

UOnlineStatsSubsystemUpdateStats* UOnlineStatsSubsystemUpdateStats::UpdateStats(UOnlineStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FOnlineStatsUserUpdatedStatsBP> UpdatedUserStats)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineStatsSubsystemUpdateStats* Node = NewObject<UOnlineStatsSubsystemUpdateStats>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__UpdatedUserStats = UpdatedUserStats;
    return Node;
}

void UOnlineStatsSubsystemUpdateStats::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo());
        return;
    }
    if (!(this->__Store__LocalUserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'LocalUserId' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FOnlineStatsUserUpdatedStatsBP>(this->__Store__UpdatedUserStats, [](const FOnlineStatsUserUpdatedStatsBP& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UpdatedUserStats' was not valid"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnlineStatsUpdateStatsComplete = FOnlineStatsUpdateStatsComplete::CreateUObject(this, &UOnlineStatsSubsystemUpdateStats::HandleCallback_OnlineStatsUpdateStatsComplete);

    Handle->UpdateStats(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef(), ConvertArrayElements<FOnlineStatsUserUpdatedStatsBP, FOnlineStatsUserUpdatedStats>(this->__Store__UpdatedUserStats, [](const FOnlineStatsUserUpdatedStatsBP& Val) { return Val.ToNative(); }), __DelegateHandle_OnlineStatsUpdateStatsComplete);
}

void UOnlineStatsSubsystemUpdateStats::HandleCallback_OnlineStatsUpdateStatsComplete(const FOnlineError& ResultState)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnlineStatsUpdateStatsComplete.Broadcast(FOnlineErrorInfo::FromNative(ResultState));
}

