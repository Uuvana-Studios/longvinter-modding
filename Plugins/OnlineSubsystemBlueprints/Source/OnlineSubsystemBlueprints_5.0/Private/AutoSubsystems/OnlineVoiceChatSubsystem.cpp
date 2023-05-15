// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineVoiceChatSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

IVoiceChat* UOnlineVoiceChatSubsystem::GetHandle()
{
    return this->HandlePtr;
}

bool UOnlineVoiceChatSubsystem::IsHandleValid(IVoiceChat* InHandle)
{
    return InHandle != nullptr;
}

UOnlineVoiceChatSubsystem::UOnlineVoiceChatSubsystem()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        this->HandlePtr = nullptr;
        return;
    }

    this->HandlePtr = IVoiceChat::Get();
    if (this->HandlePtr != nullptr)
    {
        this->PrimaryVoiceUser = NewObject<UVoiceChatUser>();
        this->PrimaryVoiceUser->AssignUserHandle(this->HandlePtr, false);
        this->RegisterFunctionDelegates();
    }
}

void UOnlineVoiceChatSubsystem::BeginDestroy()
{
    this->PrimaryVoiceUser = nullptr;
    if (this->HandlePtr != nullptr)
    {
        this->UnregisterFunctionDelegates();
    }
    Super::BeginDestroy();
}

bool UOnlineVoiceChatSubsystem::IsSubsystemAvailable()
{
    return this->HandlePtr != nullptr;
}

UVoiceChatUser* UOnlineVoiceChatSubsystem::CreateUser()
{
    if (this->HandlePtr == nullptr)
    {
        return nullptr;
    }

    IVoiceChatUser* NativeUser = this->HandlePtr->CreateUser();
    if (NativeUser == nullptr)
    {
        return nullptr;
    }

    UVoiceChatUser* User = NewObject<UVoiceChatUser>();
    User->AssignUserHandle(NativeUser, true);
    return User;
}

void UOnlineVoiceChatSubsystem::RegisterFunctionDelegates()
{
    auto Handle = this->GetHandle();
    if (this->IsHandleValid(Handle))
    {
        this->FunctionDelegateHandle_FOnVoiceChatReconnectedDelegate = Handle->OnVoiceChatReconnected().AddLambda([this]() {
            this->OnVoiceChatReconnected.Broadcast();
        });
        this->FunctionDelegateHandle_FOnVoiceChatConnectedDelegate = Handle->OnVoiceChatConnected().AddLambda([this]() {
            this->OnVoiceChatConnected.Broadcast();
        });
        this->FunctionDelegateHandle_FOnVoiceChatDisconnectedDelegate = Handle->OnVoiceChatDisconnected().AddLambda([this](const FVoiceChatResult& Reason) {
            this->OnVoiceChatDisconnected.Broadcast(FVoiceChatResultBP::FromNative(Reason));
        });
    }
}

void UOnlineVoiceChatSubsystem::UnregisterFunctionDelegates()
{
    auto Handle = this->GetHandle();
    if (this->IsHandleValid(Handle))
    {
        Handle->OnVoiceChatReconnected().Remove(this->FunctionDelegateHandle_FOnVoiceChatReconnectedDelegate);
        Handle->OnVoiceChatConnected().Remove(this->FunctionDelegateHandle_FOnVoiceChatConnectedDelegate);
        Handle->OnVoiceChatDisconnected().Remove(this->FunctionDelegateHandle_FOnVoiceChatDisconnectedDelegate);
    }
}

bool UOnlineVoiceChatSubsystem::Initialize_()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->Initialize();

    return __Result;
}

bool UOnlineVoiceChatSubsystem::Uninitialize_()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->Uninitialize();

    return __Result;
}

bool UOnlineVoiceChatSubsystem::IsInitialized()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->IsInitialized();

    return __Result;
}

UOnlineVoiceChatSubsystemConnect* UOnlineVoiceChatSubsystemConnect::Connect(UOnlineVoiceChatSubsystem* Subsystem )
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineVoiceChatSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineVoiceChatSubsystemConnect* Node = NewObject<UOnlineVoiceChatSubsystemConnect>();
    Node->__Store__Subsystem = Subsystem;
    return Node;
}

void UOnlineVoiceChatSubsystemConnect::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatConnectComplete = FOnVoiceChatConnectCompleteDelegate::CreateUObject(this, &UOnlineVoiceChatSubsystemConnect::HandleCallback_OnVoiceChatConnectComplete);

    Handle->Connect(__DelegateHandle_OnVoiceChatConnectComplete);
}

void UOnlineVoiceChatSubsystemConnect::HandleCallback_OnVoiceChatConnectComplete(const FVoiceChatResult& Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatConnectComplete.Broadcast(FVoiceChatResultBP::FromNative(Result));
}

UOnlineVoiceChatSubsystemDisconnect* UOnlineVoiceChatSubsystemDisconnect::Disconnect(UOnlineVoiceChatSubsystem* Subsystem )
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineVoiceChatSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineVoiceChatSubsystemDisconnect* Node = NewObject<UOnlineVoiceChatSubsystemDisconnect>();
    Node->__Store__Subsystem = Subsystem;
    return Node;
}

void UOnlineVoiceChatSubsystemDisconnect::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }
    auto __DelegateHandle_OnVoiceChatDisconnectComplete = FOnVoiceChatDisconnectCompleteDelegate::CreateUObject(this, &UOnlineVoiceChatSubsystemDisconnect::HandleCallback_OnVoiceChatDisconnectComplete);

    Handle->Disconnect(__DelegateHandle_OnVoiceChatDisconnectComplete);
}

void UOnlineVoiceChatSubsystemDisconnect::HandleCallback_OnVoiceChatDisconnectComplete(const FVoiceChatResult& Result)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(FVoiceChatResultBP());
        return;
    }

    this->OnVoiceChatDisconnectComplete.Broadcast(FVoiceChatResultBP::FromNative(Result));
}

bool UOnlineVoiceChatSubsystem::IsConnecting()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->IsConnecting();

    return __Result;
}

bool UOnlineVoiceChatSubsystem::IsConnected()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->IsConnected();

    return __Result;
}

