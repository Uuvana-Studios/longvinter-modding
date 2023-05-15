// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineMessageSanitizerSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> UOnlineMessageSanitizerSubsystem::GetHandle()
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

    TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = GetMessageSanitizerSafe(Subsystem);
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineMessageSanitizerSubsystem::IsHandleValid(const TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineMessageSanitizerSubsystem::UOnlineMessageSanitizerSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineMessageSanitizerSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
    }
}

void UOnlineMessageSanitizerSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineMessageSanitizerSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineMessageSanitizerSubsystemSanitizeDisplayName* UOnlineMessageSanitizerSubsystemSanitizeDisplayName::SanitizeDisplayName(UOnlineMessageSanitizerSubsystem* Subsystem ,FString DisplayName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineMessageSanitizerSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineMessageSanitizerSubsystemSanitizeDisplayName* Node = NewObject<UOnlineMessageSanitizerSubsystemSanitizeDisplayName>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__DisplayName = DisplayName;
    return Node;
}

void UOnlineMessageSanitizerSubsystemSanitizeDisplayName::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnMessageProcessed = FOnMessageProcessed::CreateUObject(this, &UOnlineMessageSanitizerSubsystemSanitizeDisplayName::HandleCallback_OnMessageProcessed);

    Handle->SanitizeDisplayName(this->__Store__DisplayName, __DelegateHandle_OnMessageProcessed);
}

void UOnlineMessageSanitizerSubsystemSanitizeDisplayName::HandleCallback_OnMessageProcessed(bool bSuccess, const FString& SanitizedMessage)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnMessageProcessed.Broadcast(bSuccess, SanitizedMessage);
}

UOnlineMessageSanitizerSubsystemSanitizeDisplayNames* UOnlineMessageSanitizerSubsystemSanitizeDisplayNames::SanitizeDisplayNames(UOnlineMessageSanitizerSubsystem* Subsystem ,TArray<FString> DisplayNames)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineMessageSanitizerSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineMessageSanitizerSubsystemSanitizeDisplayNames* Node = NewObject<UOnlineMessageSanitizerSubsystemSanitizeDisplayNames>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__DisplayNames = DisplayNames;
    return Node;
}

void UOnlineMessageSanitizerSubsystemSanitizeDisplayNames::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, TArray<FString>());
        return;
    }
    if (!(ValidateArrayElements<FString>(this->__Store__DisplayNames, [](const FString& Val) { return true; })))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'DisplayNames' was not valid"));
        this->OnCallFailed.Broadcast(false, TArray<FString>());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, TArray<FString>());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnMessageArrayProcessed = FOnMessageArrayProcessed::CreateUObject(this, &UOnlineMessageSanitizerSubsystemSanitizeDisplayNames::HandleCallback_OnMessageArrayProcessed);

    Handle->SanitizeDisplayNames(ConvertArrayElements<FString, FString>(this->__Store__DisplayNames, [](const FString& Val) { return Val; }), __DelegateHandle_OnMessageArrayProcessed);
}

void UOnlineMessageSanitizerSubsystemSanitizeDisplayNames::HandleCallback_OnMessageArrayProcessed(bool bSuccess, const TArray<FString>& SanitizedMessages)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TArray<FString>());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TArray<FString>());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnMessageArrayProcessed.Broadcast(bSuccess, ConvertArrayElements<FString, FString>(SanitizedMessages, [](const FString& Val) { return Val; }));
}

UOnlineMessageSanitizerSubsystemQueryBlockedUser* UOnlineMessageSanitizerSubsystemQueryBlockedUser::QueryBlockedUser(UOnlineMessageSanitizerSubsystem* Subsystem ,int32 LocalUserNum, FString FromUserId, FString FromPlatform)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineMessageSanitizerSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineMessageSanitizerSubsystemQueryBlockedUser* Node = NewObject<UOnlineMessageSanitizerSubsystemQueryBlockedUser>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__FromUserId = FromUserId;
    Node->__Store__FromPlatform = FromPlatform;
    return Node;
}

void UOnlineMessageSanitizerSubsystemQueryBlockedUser::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FBlockedQueryResultInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FBlockedQueryResultInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnQueryUserBlockedResponse = FOnQueryUserBlockedResponse::CreateUObject(this, &UOnlineMessageSanitizerSubsystemQueryBlockedUser::HandleCallback_OnQueryUserBlockedResponse);

    Handle->QueryBlockedUser(this->__Store__LocalUserNum, this->__Store__FromUserId, this->__Store__FromPlatform, __DelegateHandle_OnQueryUserBlockedResponse);
}

void UOnlineMessageSanitizerSubsystemQueryBlockedUser::HandleCallback_OnQueryUserBlockedResponse(const FBlockedQueryResult& QueryResult)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FBlockedQueryResultInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FBlockedQueryResultInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnQueryUserBlockedResponse.Broadcast(FBlockedQueryResultInfo::FromNative(QueryResult));
}

void UOnlineMessageSanitizerSubsystem::ResetBlockedUserCache()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->ResetBlockedUserCache();
}

