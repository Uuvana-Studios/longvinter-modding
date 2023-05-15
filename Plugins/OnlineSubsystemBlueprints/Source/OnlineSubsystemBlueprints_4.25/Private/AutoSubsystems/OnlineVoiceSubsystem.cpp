// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineVoiceSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> UOnlineVoiceSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetVoiceInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineVoiceSubsystem::IsHandleValid(const TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineVoiceSubsystem::UOnlineVoiceSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineVoiceSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnPlayerTalkingStateChanged = Handle->AddOnPlayerTalkingStateChangedDelegate_Handle(
            FOnPlayerTalkingStateChanged::FDelegate::CreateWeakLambda(this, [this](TSharedRef<const FUniqueNetId> Param1, bool Param2) {
                this->OnPlayerTalkingStateChanged.Broadcast(FUniqueNetIdRepl(Param1), Param2);
            })
        );
    }
}

void UOnlineVoiceSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineVoice, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnPlayerTalkingStateChangedDelegate_Handle(this->DelegateHandle_OnPlayerTalkingStateChanged);
        
    }

    Super::BeginDestroy();
}

bool UOnlineVoiceSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

void UOnlineVoiceSubsystem::ProcessMuteChangeNotification()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->ProcessMuteChangeNotification();
}

void UOnlineVoiceSubsystem::StartNetworkedVoice(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->StartNetworkedVoice((uint8)(LocalUserNum));
}

void UOnlineVoiceSubsystem::StopNetworkedVoice(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->StopNetworkedVoice((uint8)(LocalUserNum));
}

bool UOnlineVoiceSubsystem::RegisterLocalTalker(int64 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->RegisterLocalTalker((uint32)(LocalUserNum));

    return __Result;
}

void UOnlineVoiceSubsystem::RegisterLocalTalkers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->RegisterLocalTalkers();
}

bool UOnlineVoiceSubsystem::UnregisterLocalTalker(int64 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->UnregisterLocalTalker((uint32)(LocalUserNum));

    return __Result;
}

void UOnlineVoiceSubsystem::UnregisterLocalTalkers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->UnregisterLocalTalkers();
}

bool UOnlineVoiceSubsystem::RegisterRemoteTalker(FUniqueNetIdRepl UniqueId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UniqueId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->RegisterRemoteTalker(UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineVoiceSubsystem::UnregisterRemoteTalker(FUniqueNetIdRepl UniqueId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UniqueId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->UnregisterRemoteTalker(UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

void UOnlineVoiceSubsystem::RemoveAllRemoteTalkers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->RemoveAllRemoteTalkers();
}

bool UOnlineVoiceSubsystem::IsHeadsetPresent(int64 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->IsHeadsetPresent((uint32)(LocalUserNum));

    return __Result;
}

bool UOnlineVoiceSubsystem::IsLocalPlayerTalking(int64 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->IsLocalPlayerTalking((uint32)(LocalUserNum));

    return __Result;
}

bool UOnlineVoiceSubsystem::IsRemotePlayerTalking(FUniqueNetIdRepl UniqueId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UniqueId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsRemotePlayerTalking(UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineVoiceSubsystem::IsMuted(int64 LocalUserNum, FUniqueNetIdRepl UniqueId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(UniqueId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->IsMuted((uint32)(LocalUserNum), UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineVoiceSubsystem::MuteRemoteTalker(int32 LocalUserNum, FUniqueNetIdRepl PlayerId, bool bIsSystemWide)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(PlayerId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->MuteRemoteTalker((uint8)(LocalUserNum), PlayerId.GetUniqueNetId().ToSharedRef().Get(), bIsSystemWide);

    return __Result;
}

bool UOnlineVoiceSubsystem::UnmuteRemoteTalker(int32 LocalUserNum, FUniqueNetIdRepl PlayerId, bool bIsSystemWide)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }
    if (!(PlayerId.IsValid()))
    {
        return false;
    }


    auto __Result = Handle->UnmuteRemoteTalker((uint8)(LocalUserNum), PlayerId.GetUniqueNetId().ToSharedRef().Get(), bIsSystemWide);

    return __Result;
}

int32 UOnlineVoiceSubsystem::GetNumLocalTalkers()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }


    auto __Result = Handle->GetNumLocalTalkers();

    return __Result;
}

void UOnlineVoiceSubsystem::ClearVoicePackets()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->ClearVoicePackets();
}

FString UOnlineVoiceSubsystem::GetVoiceDebugState()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return TEXT("");
    }


    auto __Result = Handle->GetVoiceDebugState();

    return __Result;
}

float UOnlineVoiceSubsystem::GetAmplitudeOfRemoteTalker(FUniqueNetIdRepl PlayerId)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return 0;
    }
    if (!(PlayerId.IsValid()))
    {
        return 0;
    }


    auto __Result = Handle->GetAmplitudeOfRemoteTalker(PlayerId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineVoiceSubsystem::PatchRemoteTalkerOutputToEndpoint(FString InDeviceName, bool bMuteInGameOutput)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->PatchRemoteTalkerOutputToEndpoint(InDeviceName, bMuteInGameOutput);

    return __Result;
}

bool UOnlineVoiceSubsystem::PatchLocalTalkerOutputToEndpoint(FString InDeviceName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->PatchLocalTalkerOutputToEndpoint(InDeviceName);

    return __Result;
}

void UOnlineVoiceSubsystem::DisconnectAllEndpoints()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DisconnectAllEndpoints();
}

