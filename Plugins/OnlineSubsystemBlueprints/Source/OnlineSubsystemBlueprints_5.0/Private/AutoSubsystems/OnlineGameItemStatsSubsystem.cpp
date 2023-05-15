// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineGameItemStatsSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> UOnlineGameItemStatsSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetGameItemStatsInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineGameItemStatsSubsystem::IsHandleValid(const TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineGameItemStatsSubsystem::UOnlineGameItemStatsSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineGameItemStatsSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
    }
}

void UOnlineGameItemStatsSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineGameItemStats, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineGameItemStatsSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineGameItemStatsSubsystemItemUsage* UOnlineGameItemStatsSubsystemItemUsage::ItemUsage(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, FString ItemUsedBy, TArray<FString> ItemsUsed)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemUsage* Node = NewObject<UOnlineGameItemStatsSubsystemItemUsage>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ItemUsedBy = ItemUsedBy;
    Node->__Store__ItemsUsed = ItemsUsed;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemUsage::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__ItemsUsed, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ItemsUsed' was not valid"));
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

    auto __DelegateHandle_OnItemUsageComplete = FOnItemUsageComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemUsage::HandleCallback_OnItemUsageComplete);

    Handle->ItemUsage(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ItemUsedBy, ConvertArrayElements<FString, FString>(this->__Store__ItemsUsed, [](const FString& Val) { return Val; }), __DelegateHandle_OnItemUsageComplete);
}

void UOnlineGameItemStatsSubsystemItemUsage::HandleCallback_OnItemUsageComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemUsageComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

UOnlineGameItemStatsSubsystemItemImpact* UOnlineGameItemStatsSubsystemItemImpact::ItemImpact(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FString> TargetActors, FString ImpactInitiatedBy, TArray<FString> ItemsUsed)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemImpact* Node = NewObject<UOnlineGameItemStatsSubsystemItemImpact>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__TargetActors = TargetActors;
    Node->__Store__ImpactInitiatedBy = ImpactInitiatedBy;
    Node->__Store__ItemsUsed = ItemsUsed;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemImpact::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__TargetActors, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'TargetActors' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__ItemsUsed, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ItemsUsed' was not valid"));
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

    auto __DelegateHandle_OnItemImpactComplete = FOnItemImpactComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemImpact::HandleCallback_OnItemImpactComplete);

    Handle->ItemImpact(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FString, FString>(this->__Store__TargetActors, [](const FString& Val) { return Val; }), this->__Store__ImpactInitiatedBy, ConvertArrayElements<FString, FString>(this->__Store__ItemsUsed, [](const FString& Val) { return Val; }), __DelegateHandle_OnItemImpactComplete);
}

void UOnlineGameItemStatsSubsystemItemImpact::HandleCallback_OnItemImpactComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemImpactComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

UOnlineGameItemStatsSubsystemItemMitigation* UOnlineGameItemStatsSubsystemItemMitigation::ItemMitigation(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FString> ItemsUsed, TArray<FString> ImpactItemsMitigated, FString ItemUsedBy)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemMitigation* Node = NewObject<UOnlineGameItemStatsSubsystemItemMitigation>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ItemsUsed = ItemsUsed;
    Node->__Store__ImpactItemsMitigated = ImpactItemsMitigated;
    Node->__Store__ItemUsedBy = ItemUsedBy;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemMitigation::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__ItemsUsed, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ItemsUsed' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__ImpactItemsMitigated, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ImpactItemsMitigated' was not valid"));
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

    auto __DelegateHandle_OnItemMitigationComplete = FOnItemMitigationComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemMitigation::HandleCallback_OnItemMitigationComplete);

    Handle->ItemMitigation(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FString, FString>(this->__Store__ItemsUsed, [](const FString& Val) { return Val; }), ConvertArrayElements<FString, FString>(this->__Store__ImpactItemsMitigated, [](const FString& Val) { return Val; }), this->__Store__ItemUsedBy, __DelegateHandle_OnItemMitigationComplete);
}

void UOnlineGameItemStatsSubsystemItemMitigation::HandleCallback_OnItemMitigationComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemMitigationComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

UOnlineGameItemStatsSubsystemItemAvailabilityChange* UOnlineGameItemStatsSubsystemItemAvailabilityChange::ItemAvailabilityChange(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FString> AvailableItems, TArray<FString> UnavailableItems)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemAvailabilityChange* Node = NewObject<UOnlineGameItemStatsSubsystemItemAvailabilityChange>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__AvailableItems = AvailableItems;
    Node->__Store__UnavailableItems = UnavailableItems;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemAvailabilityChange::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__AvailableItems, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'AvailableItems' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__UnavailableItems, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UnavailableItems' was not valid"));
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

    auto __DelegateHandle_OnItemAvailabilityChangeComplete = FOnItemAvailabilityChangeComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemAvailabilityChange::HandleCallback_OnItemAvailabilityChangeComplete);

    Handle->ItemAvailabilityChange(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FString, FString>(this->__Store__AvailableItems, [](const FString& Val) { return Val; }), ConvertArrayElements<FString, FString>(this->__Store__UnavailableItems, [](const FString& Val) { return Val; }), __DelegateHandle_OnItemAvailabilityChangeComplete);
}

void UOnlineGameItemStatsSubsystemItemAvailabilityChange::HandleCallback_OnItemAvailabilityChangeComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemAvailabilityChangeComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

UOnlineGameItemStatsSubsystemItemInventoryChange* UOnlineGameItemStatsSubsystemItemInventoryChange::ItemInventoryChange(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FString> ItemsToAdd, TArray<FString> ItemsToRemove)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemInventoryChange* Node = NewObject<UOnlineGameItemStatsSubsystemItemInventoryChange>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__ItemsToAdd = ItemsToAdd;
    Node->__Store__ItemsToRemove = ItemsToRemove;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemInventoryChange::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__ItemsToAdd, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ItemsToAdd' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__ItemsToRemove, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ItemsToRemove' was not valid"));
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

    auto __DelegateHandle_OnItemInventoryChangeComplete = FOnItemInventoryChangeComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemInventoryChange::HandleCallback_OnItemInventoryChangeComplete);

    Handle->ItemInventoryChange(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FString, FString>(this->__Store__ItemsToAdd, [](const FString& Val) { return Val; }), ConvertArrayElements<FString, FString>(this->__Store__ItemsToRemove, [](const FString& Val) { return Val; }), __DelegateHandle_OnItemInventoryChangeComplete);
}

void UOnlineGameItemStatsSubsystemItemInventoryChange::HandleCallback_OnItemInventoryChangeComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemInventoryChangeComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

UOnlineGameItemStatsSubsystemItemLoadoutChange* UOnlineGameItemStatsSubsystemItemLoadoutChange::ItemLoadoutChange(UOnlineGameItemStatsSubsystem* Subsystem ,FUniqueNetIdRepl LocalUserId, TArray<FString> EquippedItems, TArray<FString> UnequippedItems)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineGameItemStatsSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineGameItemStatsSubsystemItemLoadoutChange* Node = NewObject<UOnlineGameItemStatsSubsystemItemLoadoutChange>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserId = LocalUserId;
    Node->__Store__EquippedItems = EquippedItems;
    Node->__Store__UnequippedItems = UnequippedItems;
    return Node;
}

void UOnlineGameItemStatsSubsystemItemLoadoutChange::Activate()
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
    if (!(ValidateArrayElements<FString>(this->__Store__EquippedItems, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'EquippedItems' was not valid"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), FOnlineErrorInfo());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__UnequippedItems, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UnequippedItems' was not valid"));
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

    auto __DelegateHandle_OnItemLoadoutChangeComplete = FOnItemLoadoutChangeComplete::CreateUObject(this, &UOnlineGameItemStatsSubsystemItemLoadoutChange::HandleCallback_OnItemLoadoutChangeComplete);

    Handle->ItemLoadoutChange(this->__Store__LocalUserId.GetUniqueNetId().ToSharedRef().Get(), ConvertArrayElements<FString, FString>(this->__Store__EquippedItems, [](const FString& Val) { return Val; }), ConvertArrayElements<FString, FString>(this->__Store__UnequippedItems, [](const FString& Val) { return Val; }), __DelegateHandle_OnItemLoadoutChangeComplete);
}

void UOnlineGameItemStatsSubsystemItemLoadoutChange::HandleCallback_OnItemLoadoutChangeComplete(const FUniqueNetId& LocalUserId, const FOnlineError& Status)
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

    this->OnItemLoadoutChangeComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(LocalUserId), FOnlineErrorInfo::FromNative(Status));
}

