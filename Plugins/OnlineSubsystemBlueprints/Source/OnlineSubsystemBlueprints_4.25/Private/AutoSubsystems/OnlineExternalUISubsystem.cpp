// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineExternalUISubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> UOnlineExternalUISubsystem::GetHandle()
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

    TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetExternalUIInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineExternalUISubsystem::IsHandleValid(const TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineExternalUISubsystem::UOnlineExternalUISubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineExternalUISubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnExternalUIChange = Handle->AddOnExternalUIChangeDelegate_Handle(
            FOnExternalUIChange::FDelegate::CreateWeakLambda(this, [this](bool Param1) {
                this->OnExternalUIChange.Broadcast(Param1);
            })
        );
    }
}

void UOnlineExternalUISubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineExternalUI, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnExternalUIChangeDelegate_Handle(this->DelegateHandle_OnExternalUIChange);
        
    }

    Super::BeginDestroy();
}

bool UOnlineExternalUISubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

UOnlineExternalUISubsystemShowLoginUI* UOnlineExternalUISubsystemShowLoginUI::ShowLoginUI(UOnlineExternalUISubsystem* Subsystem ,int ControllerIndex, bool bShowOnlineOnly, bool bShowSkipButton)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowLoginUI* Node = NewObject<UOnlineExternalUISubsystemShowLoginUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__ControllerIndex = ControllerIndex;
    Node->__Store__bShowOnlineOnly = bShowOnlineOnly;
    Node->__Store__bShowSkipButton = bShowSkipButton;
    return Node;
}

void UOnlineExternalUISubsystemShowLoginUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), 0, FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), 0, FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnLoginUIClosed = FOnLoginUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowLoginUI::HandleCallback_OnLoginUIClosed);
    
    if (!Handle->ShowLoginUI(this->__Store__ControllerIndex, this->__Store__bShowOnlineOnly, this->__Store__bShowSkipButton, __DelegateHandle_OnLoginUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowLoginUI' did not start successfully"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), 0, FOnlineErrorInfo());
        return;
    }
}

void UOnlineExternalUISubsystemShowLoginUI::HandleCallback_OnLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex, const FOnlineError& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), 0, FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FUniqueNetIdRepl(), 0, FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnLoginUIClosed.Broadcast(FUniqueNetIdRepl(UniqueId), ControllerIndex, FOnlineErrorInfo::FromNative(Error));
}

UOnlineExternalUISubsystemShowAccountCreationUI* UOnlineExternalUISubsystemShowAccountCreationUI::ShowAccountCreationUI(UOnlineExternalUISubsystem* Subsystem ,int ControllerIndex)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowAccountCreationUI* Node = NewObject<UOnlineExternalUISubsystemShowAccountCreationUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__ControllerIndex = ControllerIndex;
    return Node;
}

void UOnlineExternalUISubsystemShowAccountCreationUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(0, FOnlineAccountCredential(), FOnlineErrorInfo());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(0, FOnlineAccountCredential(), FOnlineErrorInfo());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnAccountCreationUIClosed = FOnAccountCreationUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowAccountCreationUI::HandleCallback_OnAccountCreationUIClosed);
    
    if (!Handle->ShowAccountCreationUI(this->__Store__ControllerIndex, __DelegateHandle_OnAccountCreationUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowAccountCreationUI' did not start successfully"));
        this->OnCallFailed.Broadcast(0, FOnlineAccountCredential(), FOnlineErrorInfo());
        return;
    }
}

void UOnlineExternalUISubsystemShowAccountCreationUI::HandleCallback_OnAccountCreationUIClosed(const int ControllerIndex, const FOnlineAccountCredentials& AccountCredentials, const FOnlineError& Error)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FOnlineAccountCredential(), FOnlineErrorInfo());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(0, FOnlineAccountCredential(), FOnlineErrorInfo());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnAccountCreationUIClosed.Broadcast(ControllerIndex, FOnlineAccountCredential::FromNative(AccountCredentials), FOnlineErrorInfo::FromNative(Error));
}

bool UOnlineExternalUISubsystem::ShowFriendsUI(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ShowFriendsUI(LocalUserNum);

    return __Result;
}

bool UOnlineExternalUISubsystem::ShowInviteUI(int32 LocalUserNum, FName SessionName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ShowInviteUI(LocalUserNum, SessionName);

    return __Result;
}

bool UOnlineExternalUISubsystem::ShowAchievementsUI(int32 LocalUserNum)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ShowAchievementsUI(LocalUserNum);

    return __Result;
}

bool UOnlineExternalUISubsystem::ShowLeaderboardUI(FString LeaderboardName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ShowLeaderboardUI(LeaderboardName);

    return __Result;
}

UOnlineExternalUISubsystemShowWebURL* UOnlineExternalUISubsystemShowWebURL::ShowWebURL(UOnlineExternalUISubsystem* Subsystem ,FString Url, FShowWebUrlParameters ShowParams)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowWebURL* Node = NewObject<UOnlineExternalUISubsystemShowWebURL>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__Url = Url;
    Node->__Store__ShowParams = ShowParams;
    return Node;
}

void UOnlineExternalUISubsystemShowWebURL::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnShowWebUrlClosed = FOnShowWebUrlClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowWebURL::HandleCallback_OnShowWebUrlClosed);
    
    if (!Handle->ShowWebURL(this->__Store__Url, this->__Store__ShowParams.ToNative(), __DelegateHandle_OnShowWebUrlClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowWebURL' did not start successfully"));
        this->OnCallFailed.Broadcast(TEXT(""));
        return;
    }
}

void UOnlineExternalUISubsystemShowWebURL::HandleCallback_OnShowWebUrlClosed(const FString& FinalUrl)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""));
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnShowWebUrlClosed.Broadcast(FinalUrl);
}

bool UOnlineExternalUISubsystem::CloseWebURL()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->CloseWebURL();

    return __Result;
}

UOnlineExternalUISubsystemShowProfileUI* UOnlineExternalUISubsystemShowProfileUI::ShowProfileUI(UOnlineExternalUISubsystem* Subsystem ,FUniqueNetIdRepl Requestor, FUniqueNetIdRepl Requestee)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowProfileUI* Node = NewObject<UOnlineExternalUISubsystemShowProfileUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__Requestor = Requestor;
    Node->__Store__Requestee = Requestee;
    return Node;
}

void UOnlineExternalUISubsystemShowProfileUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast();
        return;
    }
    if (!(this->__Store__Requestor.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Requestor' was not valid"));
        this->OnCallFailed.Broadcast();
        return;
    }
    if (!(this->__Store__Requestee.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Requestee' was not valid"));
        this->OnCallFailed.Broadcast();
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast();
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    auto __DelegateHandle_OnProfileUIClosed = FOnProfileUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowProfileUI::HandleCallback_OnProfileUIClosed);
    
    if (!Handle->ShowProfileUI(this->__Store__Requestor.GetUniqueNetId().ToSharedRef().Get(), this->__Store__Requestee.GetUniqueNetId().ToSharedRef().Get(), __DelegateHandle_OnProfileUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowProfileUI' did not start successfully"));
        this->OnCallFailed.Broadcast();
        return;
    }
}

void UOnlineExternalUISubsystemShowProfileUI::HandleCallback_OnProfileUIClosed()
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast();
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast();
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

    this->OnProfileUIClosed.Broadcast();
}

bool UOnlineExternalUISubsystem::ShowAccountUpgradeUI(FUniqueNetIdRepl UniqueId)
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


    auto __Result = Handle->ShowAccountUpgradeUI(UniqueId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

UOnlineExternalUISubsystemShowStoreUI* UOnlineExternalUISubsystemShowStoreUI::ShowStoreUI(UOnlineExternalUISubsystem* Subsystem ,int32 LocalUserNum, FShowStoreParameters ShowParams)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowStoreUI* Node = NewObject<UOnlineExternalUISubsystemShowStoreUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__ShowParams = ShowParams;
    return Node;
}

void UOnlineExternalUISubsystemShowStoreUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
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

    auto __DelegateHandle_OnShowStoreUIClosed = FOnShowStoreUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowStoreUI::HandleCallback_OnShowStoreUIClosed);
    
    if (!Handle->ShowStoreUI(this->__Store__LocalUserNum, this->__Store__ShowParams.ToNative(), __DelegateHandle_OnShowStoreUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowStoreUI' did not start successfully"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineExternalUISubsystemShowStoreUI::HandleCallback_OnShowStoreUIClosed(bool bPurchased)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
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

    this->OnShowStoreUIClosed.Broadcast(bPurchased);
}

UOnlineExternalUISubsystemShowSendMessageUI* UOnlineExternalUISubsystemShowSendMessageUI::ShowSendMessageUI(UOnlineExternalUISubsystem* Subsystem ,int32 LocalUserNum, FShowSendMessageParameters ShowParams)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowSendMessageUI* Node = NewObject<UOnlineExternalUISubsystemShowSendMessageUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__ShowParams = ShowParams;
    return Node;
}

void UOnlineExternalUISubsystemShowSendMessageUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
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

    auto __DelegateHandle_OnShowSendMessageUIClosed = FOnShowSendMessageUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowSendMessageUI::HandleCallback_OnShowSendMessageUIClosed);
    
    if (!Handle->ShowSendMessageUI(this->__Store__LocalUserNum, this->__Store__ShowParams.ToNative(), __DelegateHandle_OnShowSendMessageUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowSendMessageUI' did not start successfully"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineExternalUISubsystemShowSendMessageUI::HandleCallback_OnShowSendMessageUIClosed(bool bMessageSent)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
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

    this->OnShowSendMessageUIClosed.Broadcast(bMessageSent);
}

UOnlineExternalUISubsystemShowSendMessageToUserUI* UOnlineExternalUISubsystemShowSendMessageToUserUI::ShowSendMessageToUserUI(UOnlineExternalUISubsystem* Subsystem ,int32 LocalUserNum, FUniqueNetIdRepl Recipient, FShowSendMessageParameters ShowParams)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineExternalUISubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineExternalUISubsystemShowSendMessageToUserUI* Node = NewObject<UOnlineExternalUISubsystemShowSendMessageToUserUI>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("");
    Node->__Store__LocalUserNum = LocalUserNum;
    Node->__Store__Recipient = Recipient;
    Node->__Store__ShowParams = ShowParams;
    return Node;
}

void UOnlineExternalUISubsystemShowSendMessageToUserUI::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
    if (!(this->__Store__Recipient.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'Recipient' was not valid"));
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

    auto __DelegateHandle_OnShowSendMessageUIClosed = FOnShowSendMessageUIClosedDelegate::CreateUObject(this, &UOnlineExternalUISubsystemShowSendMessageToUserUI::HandleCallback_OnShowSendMessageUIClosed);
    
    if (!Handle->ShowSendMessageToUserUI(this->__Store__LocalUserNum, this->__Store__Recipient.GetUniqueNetId().ToSharedRef().Get(), this->__Store__ShowParams.ToNative(), __DelegateHandle_OnShowSendMessageUIClosed))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ShowSendMessageToUserUI' did not start successfully"));
        this->OnCallFailed.Broadcast(false);
        return;
    }
}

void UOnlineExternalUISubsystemShowSendMessageToUserUI::HandleCallback_OnShowSendMessageUIClosed(bool bMessageSent)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false);
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

    this->OnShowSendMessageUIClosed.Broadcast(bMessageSent);
}

bool UOnlineExternalUISubsystem::ShowPlatformMessageBox(FUniqueNetIdRepl UserId, EPlatformMessageType_ MessageType)
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


    auto __Result = Handle->ShowPlatformMessageBox(UserId.GetUniqueNetId().ToSharedRef().Get(), (EPlatformMessageType)(MessageType));

    return __Result;
}

void UOnlineExternalUISubsystem::ReportEnterInGameStoreUI()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->ReportEnterInGameStoreUI();
}

void UOnlineExternalUISubsystem::ReportExitInGameStoreUI()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->ReportExitInGameStoreUI();
}

