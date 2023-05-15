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
    }
}

void UOnlineVoiceChatSubsystem::UnregisterFunctionDelegates()
{
    auto Handle = this->GetHandle();
    if (this->IsHandleValid(Handle))
    {
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

