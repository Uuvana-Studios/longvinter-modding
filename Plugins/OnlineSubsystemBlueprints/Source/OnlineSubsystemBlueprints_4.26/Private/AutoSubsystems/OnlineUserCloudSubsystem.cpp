// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineUserCloudSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> UOnlineUserCloudSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetUserCloudInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineUserCloudSubsystem::IsHandleValid(const TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineUserCloudSubsystem::UOnlineUserCloudSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineUserCloudSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnEnumerateUserFilesComplete = Handle->AddOnEnumerateUserFilesCompleteDelegate_Handle(
            FOnEnumerateUserFilesComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FUniqueNetId& UserId) {
                this->OnEnumerateUserFilesComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId));
            })
        );
        this->DelegateHandle_OnReadUserFileComplete = Handle->AddOnReadUserFileCompleteDelegate_Handle(
            FOnReadUserFileComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName) {
                this->OnReadUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
            })
        );
        this->DelegateHandle_OnWriteUserFileProgress = Handle->AddOnWriteUserFileProgressDelegate_Handle(
            FOnWriteUserFileProgress::FDelegate::CreateWeakLambda(this, [this](int32 BytesWritten, const FUniqueNetId& UserId, const FString& FileName) {
                this->OnWriteUserFileProgress.Broadcast(BytesWritten, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
            })
        );
        this->DelegateHandle_OnWriteUserFileComplete = Handle->AddOnWriteUserFileCompleteDelegate_Handle(
            FOnWriteUserFileComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName) {
                this->OnWriteUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
            })
        );
        this->DelegateHandle_OnWriteUserFileCanceled = Handle->AddOnWriteUserFileCanceledDelegate_Handle(
            FOnWriteUserFileCanceled::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName) {
                this->OnWriteUserFileCanceled.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
            })
        );
        this->DelegateHandle_OnDeleteUserFileComplete = Handle->AddOnDeleteUserFileCompleteDelegate_Handle(
            FOnDeleteUserFileComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName) {
                this->OnDeleteUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
            })
        );
    }
}

void UOnlineUserCloudSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineUserCloud, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnEnumerateUserFilesCompleteDelegate_Handle(this->DelegateHandle_OnEnumerateUserFilesComplete);
        Handle->ClearOnReadUserFileCompleteDelegate_Handle(this->DelegateHandle_OnReadUserFileComplete);
        Handle->ClearOnWriteUserFileProgressDelegate_Handle(this->DelegateHandle_OnWriteUserFileProgress);
        Handle->ClearOnWriteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileCanceledDelegate_Handle(this->DelegateHandle_OnWriteUserFileCanceled);
        Handle->ClearOnDeleteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnDeleteUserFileComplete);
        
    }

    Super::BeginDestroy();
}

bool UOnlineUserCloudSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

bool UOnlineUserCloudSubsystem::GetFileContents(FUniqueNetIdRepl UserId, FString FileName, UFileData*& FileContents)
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

    TArray<uint8> __StoreTemp__FileContents;

    auto __Result = Handle->GetFileContents(UserId.GetUniqueNetId().ToSharedRef().Get(), FileName, __StoreTemp__FileContents);
    FileContents = UFileData::FromNative(__StoreTemp__FileContents);

    return __Result;
}

bool UOnlineUserCloudSubsystem::ClearFiles(FUniqueNetIdRepl UserId)
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


    auto __Result = Handle->ClearFiles(UserId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

bool UOnlineUserCloudSubsystem::ClearFile(FUniqueNetIdRepl UserId, FString FileName)
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


    auto __Result = Handle->ClearFile(UserId.GetUniqueNetId().ToSharedRef().Get(), FileName);

    return __Result;
}

UOnlineUserCloudSubsystemEnumerateUserFiles* UOnlineUserCloudSubsystemEnumerateUserFiles::EnumerateUserFiles(UOnlineUserCloudSubsystem* Subsystem ,FUniqueNetIdRepl UserId)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserCloudSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserCloudSubsystemEnumerateUserFiles* Node = NewObject<UOnlineUserCloudSubsystemEnumerateUserFiles>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnEnumerateUserFilesComplete|%s"), *UserId.ToString());
    Node->__Store__UserId = UserId;
    return Node;
}

void UOnlineUserCloudSubsystemEnumerateUserFiles::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl());
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnEnumerateUserFilesComplete = Handle->AddOnEnumerateUserFilesCompleteDelegate_Handle(
        FOnEnumerateUserFilesCompleteDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemEnumerateUserFiles::HandleCallback_OnEnumerateUserFilesComplete));

    Handle->EnumerateUserFiles(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get());
}

void UOnlineUserCloudSubsystemEnumerateUserFiles::HandleCallback_OnEnumerateUserFilesComplete(bool bWasSuccessful, const FUniqueNetId& UserId)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl());
        return;
    }
    if (FUniqueNetIdRepl(UserId) != __Store__UserId)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnEnumerateUserFilesCompleteDelegate_Handle(this->DelegateHandle_OnEnumerateUserFilesComplete);

    this->OnEnumerateUserFilesComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId));
}

void UOnlineUserCloudSubsystem::GetUserFileList(FUniqueNetIdRepl UserId, TArray<FCloudFileHeaderBP>& UserFiles)
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

    TArray<FCloudFileHeader> __StoreTemp__UserFiles;

    Handle->GetUserFileList(UserId.GetUniqueNetId().ToSharedRef().Get(), __StoreTemp__UserFiles);
    UserFiles = ConvertArrayElements<FCloudFileHeader, FCloudFileHeaderBP>(__StoreTemp__UserFiles, [](const FCloudFileHeader& Val) { return FCloudFileHeaderBP::FromNative(Val); });
}

UOnlineUserCloudSubsystemReadUserFile* UOnlineUserCloudSubsystemReadUserFile::ReadUserFile(UOnlineUserCloudSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString FileName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserCloudSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserCloudSubsystemReadUserFile* Node = NewObject<UOnlineUserCloudSubsystemReadUserFile>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnReadUserFileComplete|%s|%s"), *UserId.ToString(), *FileName);
    Node->__Store__UserId = UserId;
    Node->__Store__FileName = FileName;
    return Node;
}

void UOnlineUserCloudSubsystemReadUserFile::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnReadUserFileComplete = Handle->AddOnReadUserFileCompleteDelegate_Handle(
        FOnReadUserFileCompleteDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemReadUserFile::HandleCallback_OnReadUserFileComplete));
    
    if (!Handle->ReadUserFile(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__FileName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadUserFile' did not start successfully"));
        Handle->ClearOnReadUserFileCompleteDelegate_Handle(this->DelegateHandle_OnReadUserFileComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
}

void UOnlineUserCloudSubsystemReadUserFile::HandleCallback_OnReadUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (FileName != __Store__FileName)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnReadUserFileCompleteDelegate_Handle(this->DelegateHandle_OnReadUserFileComplete);

    this->OnReadUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
}

UOnlineUserCloudSubsystemWriteUserFile* UOnlineUserCloudSubsystemWriteUserFile::WriteUserFile(UOnlineUserCloudSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString FileName, UFileData* FileContents, bool bCompressBeforeUpload)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserCloudSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserCloudSubsystemWriteUserFile* Node = NewObject<UOnlineUserCloudSubsystemWriteUserFile>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnWriteUserFileComplete|%s|%s"), *UserId.ToString(), *FileName);
    Node->__Store__UserId = UserId;
    Node->__Store__FileName = FileName;
    Node->__Store__FileContents = FileContents;
    Node->__Store__bCompressBeforeUpload = bCompressBeforeUpload;
    return Node;
}

void UOnlineUserCloudSubsystemWriteUserFile::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnWriteUserFileComplete = Handle->AddOnWriteUserFileCompleteDelegate_Handle(
        FOnWriteUserFileCompleteDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemWriteUserFile::HandleCallback_OnWriteUserFileComplete));
    this->ProgressDelegateHandle_OnWriteUserFileComplete = Handle->AddOnWriteUserFileProgressDelegate_Handle(
        FOnWriteUserFileProgressDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemWriteUserFile::HandleCallbackProgress_OnWriteUserFileComplete));
    this->CancelDelegateHandle_OnWriteUserFileComplete = Handle->AddOnWriteUserFileCanceledDelegate_Handle(
        FOnWriteUserFileCanceledDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemWriteUserFile::HandleCallbackCancel_OnWriteUserFileComplete));
    TArray<uint8> __StoreArg_FileContents = this->__Store__FileContents->ToNative();

    if (!Handle->WriteUserFile(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__FileName, __StoreArg_FileContents, this->__Store__bCompressBeforeUpload))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'WriteUserFile' did not start successfully"));
        Handle->ClearOnWriteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileProgressDelegate_Handle(this->ProgressDelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileCanceledDelegate_Handle(this->CancelDelegateHandle_OnWriteUserFileComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
}

void UOnlineUserCloudSubsystemWriteUserFile::HandleCallback_OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (FileName != __Store__FileName)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnWriteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileProgressDelegate_Handle(this->ProgressDelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileCanceledDelegate_Handle(this->CancelDelegateHandle_OnWriteUserFileComplete);

    this->OnWriteUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName, 0);
}
void UOnlineUserCloudSubsystemWriteUserFile::HandleCallbackProgress_OnWriteUserFileComplete(int32 BytesWritten, const FUniqueNetId& UserId, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (FileName != __Store__FileName)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }

    this->OnWriteUserFileProgress.Broadcast(false, ConvertDangerousUniqueNetIdToRepl(UserId), FileName, BytesWritten);
}
void UOnlineUserCloudSubsystemWriteUserFile::HandleCallbackCancel_OnWriteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (FileName != __Store__FileName)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""), 0);
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }

        Handle->ClearOnWriteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileProgressDelegate_Handle(this->ProgressDelegateHandle_OnWriteUserFileComplete);
        Handle->ClearOnWriteUserFileCanceledDelegate_Handle(this->CancelDelegateHandle_OnWriteUserFileComplete);

    this->OnWriteUserFileCanceled.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName, 0);
}

void UOnlineUserCloudSubsystem::CancelWriteUserFile(FUniqueNetIdRepl UserId, FString FileName)
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


    Handle->CancelWriteUserFile(UserId.GetUniqueNetId().ToSharedRef().Get(), FileName);
}

UOnlineUserCloudSubsystemDeleteUserFile* UOnlineUserCloudSubsystemDeleteUserFile::DeleteUserFile(UOnlineUserCloudSubsystem* Subsystem ,FUniqueNetIdRepl UserId, FString FileName, bool bShouldCloudDelete, bool bShouldLocallyDelete)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineUserCloudSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineUserCloudSubsystemDeleteUserFile* Node = NewObject<UOnlineUserCloudSubsystemDeleteUserFile>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnDeleteUserFileComplete|%s|%s"), *UserId.ToString(), *FileName);
    Node->__Store__UserId = UserId;
    Node->__Store__FileName = FileName;
    Node->__Store__bShouldCloudDelete = bShouldCloudDelete;
    Node->__Store__bShouldLocallyDelete = bShouldLocallyDelete;
    return Node;
}

void UOnlineUserCloudSubsystemDeleteUserFile::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (!(this->__Store__UserId.IsValid()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Parameter 'UserId' was not valid"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnDeleteUserFileComplete = Handle->AddOnDeleteUserFileCompleteDelegate_Handle(
        FOnDeleteUserFileCompleteDelegate::CreateUObject(this, &UOnlineUserCloudSubsystemDeleteUserFile::HandleCallback_OnDeleteUserFileComplete));
    
    if (!Handle->DeleteUserFile(this->__Store__UserId.GetUniqueNetId().ToSharedRef().Get(), this->__Store__FileName, this->__Store__bShouldCloudDelete, this->__Store__bShouldLocallyDelete))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'DeleteUserFile' did not start successfully"));
        Handle->ClearOnDeleteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnDeleteUserFileComplete);
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
}

void UOnlineUserCloudSubsystemDeleteUserFile::HandleCallback_OnDeleteUserFileComplete(bool bWasSuccessful, const FUniqueNetId& UserId, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (FileName != __Store__FileName)
    {
        // Not the event we're interested in.
        return;
    }

    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem handle was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, FUniqueNetIdRepl(), TEXT(""));
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnDeleteUserFileCompleteDelegate_Handle(this->DelegateHandle_OnDeleteUserFileComplete);

    this->OnDeleteUserFileComplete.Broadcast(bWasSuccessful, ConvertDangerousUniqueNetIdToRepl(UserId), FileName);
}

bool UOnlineUserCloudSubsystem::RequestUsageInfo(FUniqueNetIdRepl UserId)
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


    auto __Result = Handle->RequestUsageInfo(UserId.GetUniqueNetId().ToSharedRef().Get());

    return __Result;
}

void UOnlineUserCloudSubsystem::DumpCloudState(FUniqueNetIdRepl UserId)
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


    Handle->DumpCloudState(UserId.GetUniqueNetId().ToSharedRef().Get());
}

void UOnlineUserCloudSubsystem::DumpCloudFileState(FUniqueNetIdRepl UserId, FString FileName)
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


    Handle->DumpCloudFileState(UserId.GetUniqueNetId().ToSharedRef().Get(), FileName);
}

