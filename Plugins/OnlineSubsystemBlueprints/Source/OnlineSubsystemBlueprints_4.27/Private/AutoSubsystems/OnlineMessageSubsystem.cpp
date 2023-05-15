// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineMessageSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineMessage, ESPMode::ThreadSafe> UOnlineMessageSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineMessage, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetMessageInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineMessageSubsystem::IsHandleValid(const TSharedPtr<class IOnlineMessage, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineMessageSubsystem::UOnlineMessageSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineMessageSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineMessage, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            this->DelegateHandle_OnEnumerateMessagesComplete.Add(Handle->AddOnEnumerateMessagesCompleteDelegate_Handle(
                i,
                FOnEnumerateMessagesComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool Param1, const FString& Param2) {
                    this->OnEnumerateMessagesComplete.Broadcast(i, Param1, Param2);
                })
            ));
            this->DelegateHandle_OnSendMessageComplete.Add(Handle->AddOnSendMessageCompleteDelegate_Handle(
                i,
                FOnSendMessageComplete::FDelegate::CreateWeakLambda(this, [this, i](int32 _UnusedLocalUserNum, bool Param1, const FString& Param2) {
                    this->OnSendMessageComplete.Broadcast(i, Param1, Param2);
                })
            ));
        }
    }
}

void UOnlineMessageSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineMessage, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
            Handle->ClearOnEnumerateMessagesCompleteDelegate_Handle(i, this->DelegateHandle_OnEnumerateMessagesComplete[i]);
            Handle->ClearOnSendMessageCompleteDelegate_Handle(i, this->DelegateHandle_OnSendMessageComplete[i]);
        }
        
    }

    Super::BeginDestroy();
}

bool UOnlineMessageSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

bool UOnlineMessageSubsystem::EnumerateMessages(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->EnumerateMessages(LocalUserNum);

    return __Result;
}

bool UOnlineMessageSubsystem::ClearMessageHeaders(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ClearMessageHeaders(LocalUserNum);

    return __Result;
}

bool UOnlineMessageSubsystem::ClearMessages(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ClearMessages(LocalUserNum);

    return __Result;
}

