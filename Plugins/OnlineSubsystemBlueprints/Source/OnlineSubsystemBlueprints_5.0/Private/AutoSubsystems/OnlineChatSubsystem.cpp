// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineChatSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> UOnlineChatSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetChatInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineChatSubsystem::IsHandleValid(const TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineChatSubsystem::UOnlineChatSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineChatSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnChatRoomCreated = Handle->AddOnChatRoomCreatedDelegate_Handle(
            FOnChatRoomCreated::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, bool Param3, const FString& Param4) {
                this->OnChatRoomCreated.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
            })
        );
        this->DelegateHandle_OnChatRoomConfigured = Handle->AddOnChatRoomConfiguredDelegate_Handle(
            FOnChatRoomConfigured::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, bool Param3, const FString& Param4) {
                this->OnChatRoomConfigured.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
            })
        );
        this->DelegateHandle_OnChatRoomJoinPublic = Handle->AddOnChatRoomJoinPublicDelegate_Handle(
            FOnChatRoomJoinPublic::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, bool Param3, const FString& Param4) {
                this->OnChatRoomJoinPublic.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
            })
        );
        this->DelegateHandle_OnChatRoomJoinPrivate = Handle->AddOnChatRoomJoinPrivateDelegate_Handle(
            FOnChatRoomJoinPrivate::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, bool Param3, const FString& Param4) {
                this->OnChatRoomJoinPrivate.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
            })
        );
        this->DelegateHandle_OnChatRoomExit = Handle->AddOnChatRoomExitDelegate_Handle(
            FOnChatRoomExit::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, bool Param3, const FString& Param4) {
                this->OnChatRoomExit.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, Param3, Param4);
            })
        );
        this->DelegateHandle_OnChatRoomMemberJoin = Handle->AddOnChatRoomMemberJoinDelegate_Handle(
            FOnChatRoomMemberJoin::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, const FUniqueNetId& Param3) {
                this->OnChatRoomMemberJoin.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, ConvertDangerousUniqueNetIdToRepl(Param3));
            })
        );
        this->DelegateHandle_OnChatRoomMemberExit = Handle->AddOnChatRoomMemberExitDelegate_Handle(
            FOnChatRoomMemberExit::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, const FUniqueNetId& Param3) {
                this->OnChatRoomMemberExit.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, ConvertDangerousUniqueNetIdToRepl(Param3));
            })
        );
        this->DelegateHandle_OnChatRoomMemberUpdate = Handle->AddOnChatRoomMemberUpdateDelegate_Handle(
            FOnChatRoomMemberUpdate::FDelegate::CreateWeakLambda(this, [this](const FUniqueNetId& Param1, const FChatRoomId& Param2, const FUniqueNetId& Param3) {
                this->OnChatRoomMemberUpdate.Broadcast(ConvertDangerousUniqueNetIdToRepl(Param1), Param2, ConvertDangerousUniqueNetIdToRepl(Param3));
            })
        );
    }
}

void UOnlineChatSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnChatRoomCreatedDelegate_Handle(this->DelegateHandle_OnChatRoomCreated);
        Handle->ClearOnChatRoomConfiguredDelegate_Handle(this->DelegateHandle_OnChatRoomConfigured);
        Handle->ClearOnChatRoomJoinPublicDelegate_Handle(this->DelegateHandle_OnChatRoomJoinPublic);
        Handle->ClearOnChatRoomJoinPrivateDelegate_Handle(this->DelegateHandle_OnChatRoomJoinPrivate);
        Handle->ClearOnChatRoomExitDelegate_Handle(this->DelegateHandle_OnChatRoomExit);
        Handle->ClearOnChatRoomMemberJoinDelegate_Handle(this->DelegateHandle_OnChatRoomMemberJoin);
        Handle->ClearOnChatRoomMemberExitDelegate_Handle(this->DelegateHandle_OnChatRoomMemberExit);
        Handle->ClearOnChatRoomMemberUpdateDelegate_Handle(this->DelegateHandle_OnChatRoomMemberUpdate);
        
    }

    Super::BeginDestroy();
}

bool UOnlineChatSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

bool UOnlineChatSubsystem::ExitRoom(FUniqueNetIdRepl UserId, FString RoomId)
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


    auto __Result = Handle->ExitRoom(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId);

    return __Result;
}

bool UOnlineChatSubsystem::SendRoomChat(FUniqueNetIdRepl UserId, FString RoomId, FString MsgBody)
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


    auto __Result = Handle->SendRoomChat(UserId.GetUniqueNetId().ToSharedRef().Get(), RoomId, MsgBody);

    return __Result;
}

bool UOnlineChatSubsystem::SendPrivateChat(FUniqueNetIdRepl UserId, FUniqueNetIdRepl RecipientId, FString MsgBody)
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
    if (!(RecipientId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->SendPrivateChat(UserId.GetUniqueNetId().ToSharedRef().Get(), RecipientId.GetUniqueNetId().ToSharedRef().Get(), MsgBody);

    return __Result;
}

bool UOnlineChatSubsystem::IsChatAllowed(FUniqueNetIdRepl UserId, FUniqueNetIdRepl RecipientId)
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
    if (!(RecipientId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsChatAllowed(UserId.GetUniqueNetId().ToSharedRef().Get(), RecipientId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

void UOnlineChatSubsystem::GetJoinedRooms(FUniqueNetIdRepl UserId, TArray<FString>& OutRooms)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }
    if (!(UserId.IsValid()))
    {
        return ;
    }

    TArray<FChatRoomId> __StoreTemp__OutRooms;

    Handle->GetJoinedRooms(UserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__OutRooms);
    OutRooms = ConvertArrayElements<FChatRoomId, FString>(__StoreTemp__OutRooms, [](const FChatRoomId& Val) { return Val; });
}

void UOnlineChatSubsystem::DumpChatState()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DumpChatState();
}

