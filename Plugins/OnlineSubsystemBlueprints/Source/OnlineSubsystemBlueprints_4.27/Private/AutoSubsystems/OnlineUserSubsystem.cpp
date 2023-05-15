// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineUserSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> UOnlineUserSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetUserInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineUserSubsystem::IsHandleValid(const TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineUserSubsystem::UOnlineUserSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineUserSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            this->DelegateHandle_OnQueryUserInfoComplete.Add(Handle->AddOnQueryUserInfoCompleteDelegate_Handle(
                i,
                FOnQueryUserInfoComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorStr) {
                    this->OnQueryUserInfoComplete.Broadcast(i, bWasSuccessful, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(UserIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), ErrorStr);
                })
            ));
        }
    }
}

void UOnlineUserSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineUser, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            Handle->ClearOnQueryUserInfoCompleteDelegate_Handle(i, this->DelegateHandle_OnQueryUserInfoComplete[i]);
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineUserSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineUserSubsystemQueryUserInfo* UOnlineUserSubsystemQueryUserInfo::QueryUserInfo(UOnlineUserSubsystem* Subsystem ,int32 LocalUserNum, TArray<FUniqueNetIdRepl> UserIds)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserSubsystemQueryUserInfo* Node = NewObject<UOnlineUserSubsystemQueryUserInfo>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnQueryUserInfoComplete|%d"), LocalUserNum);
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__UserIds = UserIds;
    return Node;
}

void UOnlineUserSubsystemQueryUserInfo::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
        return;
    }
    if (!(ValidateArrayElements<FUniqueNetIdRepl>(this->__Store__UserIds, [](const FUniqueNetIdRepl& Val) { return Val.IsValid(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserIds' was not valid"));
        this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnQueryUserInfoComplete = Handle->AddOnQueryUserInfoCompleteDelegate_Handle(this->__Store__LocalUserNum, 
        FOnQueryUserInfoCompleteDelegate::CreateUObject(this, &UOnlineUserSubsystemQueryUserInfo::HandleCallback_OnQueryUserInfoComplete));
    
    if (!Handle->QueryUserInfo(this->__Store__LocalUserNum, ConvertArrayElements<FUniqueNetIdRepl, FUniqueNetIdRef>(this->__Store__UserIds, [](const FUniqueNetIdRepl& Val) { return Val.GetUniqueNetId().ToSharedRef(); })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'QueryUserInfo' did not start successfully"));
        Handle->ClearOnQueryUserInfoCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnQueryUserInfoComplete);
        this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
        return;
    }
}

void UOnlineUserSubsystemQueryUserInfo::HandleCallback_OnQueryUserInfoComplete(int32 LocalUserNumCb, bool bWasSuccessful, const TArray<FUniqueNetIdRef>& UserIds, const FString& ErrorStr)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
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
        this->OnCallFailed.Broadcast(false, TArray<FUniqueNetIdRepl>(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnQueryUserInfoCompleteDelegate_Handle(this->__Store__LocalUserNum, this->DelegateHandle_OnQueryUserInfoComplete);

    this->OnQueryUserInfoComplete.Broadcast(bWasSuccessful, ConvertArrayElements<FUniqueNetIdRef, FUniqueNetIdRepl>(UserIds, [](const FUniqueNetIdRef& Val) { return FUniqueNetIdRepl(Val); }), ErrorStr);
}

bool UOnlineUserSubsystem::GetAllUserInfo(int32 LocalUserNum, TArray<UOnlineUserRef*>& OutUsers)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    TArray<TSharedRef<class FOnlineUser>> __StoreTemp__OutUsers;

    auto __Result = Handle->GetAllUserInfo(LocalUserNum, __StoreTemp__OutUsers);
    OutUsers = ConvertArrayElements<TSharedRef<class FOnlineUser>, UOnlineUserRef*>(__StoreTemp__OutUsers, [](const TSharedRef<class FOnlineUser>& Val) { return UOnlineUserRef::FromUser(Val); });

    return __Result;
}

UOnlineUserRef* UOnlineUserSubsystem::GetUserInfo(int32 LocalUserNum, FUniqueNetIdRepl UserId)
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


    auto __Result = UOnlineUserRef::FromUser(Handle->GetUserInfo(LocalUserNum, UserId.GetUniqueNetId().ToSharedRef().Get()));

    return __Result;
}

UOnlineUserSubsystemQueryUserIdMapping* UOnlineUserSubsystemQueryUserIdMapping::QueryUserIdMapping(UOnlineUserSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString DisplayNameOrEmail)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserSubsystemQueryUserIdMapping* Node = NewObject<UOnlineUserSubsystemQueryUserIdMapping>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__DisplayNameOrEmail = DisplayNameOrEmail;
    return Node;
}

void UOnlineUserSubsystemQueryUserIdMapping::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnQueryUserMappingComplete = IOnlineUser::FOnQueryUserMappingComplete::CreateUObject(this, &UOnlineUserSubsystemQueryUserIdMapping::HandleCallback_OnQueryUserMappingComplete);
    
    if (!Handle->QueryUserIdMapping(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__DisplayNameOrEmail, __DelegateHandle_OnQueryUserMappingComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'QueryUserIdMapping' did not start successfully"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
        return;
    }
}

void UOnlineUserSubsystemQueryUserIdMapping::HandleCallback_OnQueryUserMappingComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& DisplayNameOrEmail, const FUniqueNetId& FoundUserId, const FString& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnQueryUserMappingComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), DisplayNameOrEmail, ConvertDangerousUniqueNetIdToRepl(FoundUserId), Error);
}

UOnlineUserSubsystemQueryExternalIdMappings* UOnlineUserSubsystemQueryExternalIdMappings::QueryExternalIdMappings(UOnlineUserSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FExternalIdQueryOptionsBP QueryOptions, TArray<FString> ExternalIds)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserSubsystemQueryExternalIdMappings* Node = NewObject<UOnlineUserSubsystemQueryExternalIdMappings>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__UserId = UserId;
    Node->__Store__QueryOptions = QueryOptions;
    Node->__Store__ExternalIds = ExternalIds;
    return Node;
}

void UOnlineUserSubsystemQueryExternalIdMappings::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__ExternalIds, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'ExternalIds' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnQueryExternalIdMappingsComplete = IOnlineUser::FOnQueryExternalIdMappingsComplete::CreateUObject(this, &UOnlineUserSubsystemQueryExternalIdMappings::HandleCallback_OnQueryExternalIdMappingsComplete);
    
    if (!Handle->QueryExternalIdMappings(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__QueryOptions.ToNative(), ConvertArrayElements<FString, FString>(this->__Store__ExternalIds, [](const FString& Val) { return Val; }), __DelegateHandle_OnQueryExternalIdMappingsComplete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'QueryExternalIdMappings' did not start successfully"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }
}

void UOnlineUserSubsystemQueryExternalIdMappings::HandleCallback_OnQueryExternalIdMappingsComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FExternalIdQueryOptions& QueryOptions, const TArray<FString>& ExternalIds, const FString& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), FExternalIdQueryOptionsBP(), TArray<FString>(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnQueryExternalIdMappingsComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FExternalIdQueryOptionsBP::FromNative(QueryOptions), ConvertArrayElements<FString, FString>(ExternalIds, [](const FString& Val) { return Val; }), Error);
}

void UOnlineUserSubsystem::GetExternalIdMappings(FExternalIdQueryOptionsBP QueryOptions, TArray<FString> ExternalIds, TArray<FUniqueNetIdRepl>& OutIds)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }
    if (!(ValidateArrayElements<FString>(ExternalIds, [](const FString& Val) { return true; })))
    {
        return ;
    }

    TArray<FUniqueNetIdPtr> __StoreTemp__OutIds;

    Handle->GetExternalIdMappings(QueryOptions.ToNative(), ConvertArrayElements<FString, FString>(ExternalIds, [](const FString& Val) { return Val; }), __StoreTemp__OutIds);
    OutIds = ConvertArrayElements<FUniqueNetIdPtr, FUniqueNetIdRepl>(__StoreTemp__OutIds, [](const FUniqueNetIdPtr& Val) { return FUniqueNetIdRepl(Val); });
}

FUniqueNetIdRepl UOnlineUserSubsystem::GetExternalIdMapping(FExternalIdQueryOptionsBP QueryOptions, FString ExternalId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return FUniqueNetIdRepl();
    }


    auto __Result = FUniqueNetIdRepl(Handle->GetExternalIdMapping(QueryOptions.ToNative(), ExternalId));

    return __Result;
}

