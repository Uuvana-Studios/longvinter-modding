// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlinePresenceSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> UOnlinePresenceSubsystem::GetHandle()
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

    TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetPresenceInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlinePresenceSubsystem::IsHandleValid(const TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlinePresenceSubsystem::UOnlinePresenceSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlinePresenceSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnPresenceReceived = Handle->AddOnPresenceReceivedDelegate_Handle(
            FOnPresenceReceived::FDelegate::CreateWeakLambda(this, [this](const class FUniqueNetId& UserId, const TSharedRef<FOnlineUserPresence>& Presence) {
                this->OnPresenceReceived.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), FOnlineUserPresenceData::FromNative(Presence));
            })
        );
        this->DelegateHandle_OnPresenceArrayUpdated = Handle->AddOnPresenceArrayUpdatedDelegate_Handle(
            FOnPresenceArrayUpdated::FDelegate::CreateWeakLambda(this, [this](const class FUniqueNetId& UserId, const TArray<TSharedRef<FOnlineUserPresence>>& NewPresenceArray) {
                this->OnPresenceArrayUpdated.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), ConvertArrayElements<TSharedRef<FOnlineUserPresence>, FOnlineUserPresenceData>(NewPresenceArray, [](const TSharedRef<FOnlineUserPresence>& Val) { return FOnlineUserPresenceData::FromNative(Val); }));
            })
        );
    }
}

void UOnlinePresenceSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlinePresence, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnPresenceReceivedDelegate_Handle(this->DelegateHandle_OnPresenceReceived);
        Handle->ClearOnPresenceArrayUpdatedDelegate_Handle(this->DelegateHandle_OnPresenceArrayUpdated);
        
    }

    Super::BeginDestroy();
}

bool UOnlinePresenceSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlinePresenceSubsystemSetPresence* UOnlinePresenceSubsystemSetPresence::SetPresence(UOnlinePresenceSubsystem* Subsystem ,FUniqueNetIdRepl User, FOnlineUserPresenceStatusData Status)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePresenceSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePresenceSubsystemSetPresence* Node = NewObject<UOnlinePresenceSubsystemSetPresence>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__User = User;
    Node->__Store__Status = Status;
    return Node;
}

void UOnlinePresenceSubsystemSetPresence::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }
    if (!(this->__Store__User.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'User' was not valid"));
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

    auto __DelegateHandle_OnPresenceTaskComplete = IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(this, &UOnlinePresenceSubsystemSetPresence::HandleCallback_OnPresenceTaskComplete);

    Handle->SetPresence(this->__Store__User.GetUniqueNetId().ToSharedRef().Get(), this->__Store__Status.ToNative(), __DelegateHandle_OnPresenceTaskComplete);
}

void UOnlinePresenceSubsystemSetPresence::HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful)
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

    this->OnPresenceTaskComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful);
}

UOnlinePresenceSubsystemQueryPresence* UOnlinePresenceSubsystemQueryPresence::QueryPresence(UOnlinePresenceSubsystem* Subsystem ,FUniqueNetIdRepl User)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlinePresenceSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlinePresenceSubsystemQueryPresence* Node = NewObject<UOnlinePresenceSubsystemQueryPresence>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__User = User;
    return Node;
}

void UOnlinePresenceSubsystemQueryPresence::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), false);
        return;
    }
    if (!(this->__Store__User.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'User' was not valid"));
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

    auto __DelegateHandle_OnPresenceTaskComplete = IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateUObject(this, &UOnlinePresenceSubsystemQueryPresence::HandleCallback_OnPresenceTaskComplete);

    Handle->QueryPresence(this->__Store__User.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnPresenceTaskComplete);
}

void UOnlinePresenceSubsystemQueryPresence::HandleCallback_OnPresenceTaskComplete(const class FUniqueNetId& UserId, const bool bWasSuccessful)
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

    this->OnPresenceTaskComplete.Broadcast(ConvertDangerousUniqueNetIdToRepl(UserId), bWasSuccessful);
}

EOnlineCachedResult_ UOnlinePresenceSubsystem::GetCachedPresence(FUniqueNetIdRepl User, FOnlineUserPresenceData& OutPresence)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return EOnlineCachedResult_::NotFound;
    }
    if (!(User.IsValid()))
    {
        return EOnlineCachedResult_::NotFound;
    }

    TSharedPtr<FOnlineUserPresence> __StoreTemp__OutPresence;

    auto __Result = (EOnlineCachedResult_)(Handle->GetCachedPresence(User.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutPresence));
    OutPresence = FOnlineUserPresenceData::FromNative(__StoreTemp__OutPresence);

    return __Result;
}

EOnlineCachedResult_ UOnlinePresenceSubsystem::GetCachedPresenceForApp(FUniqueNetIdRepl LocalUserId, FUniqueNetIdRepl User, FString AppId, FOnlineUserPresenceData& OutPresence)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return EOnlineCachedResult_::NotFound;
    }
    if (!(LocalUserId.IsValid()))
    {
        return EOnlineCachedResult_::NotFound;
    }
    if (!(User.IsValid()))
    {
        return EOnlineCachedResult_::NotFound;
    }

    TSharedPtr<FOnlineUserPresence> __StoreTemp__OutPresence;

    auto __Result = (EOnlineCachedResult_)(Handle->GetCachedPresenceForApp(LocalUserId.GetUniqueNetId().ToSharedRef().Get(), User.GetUniqueNetId().ToSharedRef().Get(), AppId, __StoreTemp__OutPresence));
    OutPresence = FOnlineUserPresenceData::FromNative(__StoreTemp__OutPresence);

    return __Result;
}

