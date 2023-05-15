// Copyright June Rhodes. All Rights Reserved.

#include "AutoSubsystems/OnlineTitleFileSubsystem.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"

TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> UOnlineTitleFileSubsystem::GetHandle()
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

    TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> Pinned = this->HandleWk.Pin();
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
        auto __Handle__ = Subsystem->GetTitleFileInterface();
        this->HandleWk = __Handle__;
        Pinned = __Handle__;
    }

    return Pinned;
}

bool UOnlineTitleFileSubsystem::IsHandleValid(const TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe>& InHandle)
{
    return InHandle.IsValid();
}

UOnlineTitleFileSubsystem::UOnlineTitleFileSubsystem()
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
    UE_LOG(LogOnlineBlueprint, Verbose, TEXT("OnlineTitleFileSubsystem subsystem initialized in context %s"), *WorldContext->ContextHandle.ToString());

    // Perform the initial cache of the online subsystem pointer. Calling GetHandle()
    // is enough to cache the weak pointer.
    this->HandleWk = nullptr;
    this->GetHandle();

    // Now, register events.
    
    TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        this->DelegateHandle_OnEnumerateFilesComplete = Handle->AddOnEnumerateFilesCompleteDelegate_Handle(
            FOnEnumerateFilesComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& Error) {
                this->OnEnumerateFilesComplete.Broadcast(bWasSuccessful, Error);
            })
        );
        this->DelegateHandle_OnReadFileComplete = Handle->AddOnReadFileCompleteDelegate_Handle(
            FOnReadFileComplete::FDelegate::CreateWeakLambda(this, [this](bool bWasSuccessful, const FString& FileName) {
                this->OnReadFileComplete.Broadcast(bWasSuccessful, FileName);
            })
        );
        this->DelegateHandle_OnReadFileProgress = Handle->AddOnReadFileProgressDelegate_Handle(
            FOnReadFileProgress::FDelegate::CreateWeakLambda(this, [this](const FString& FileName, uint64 NumBytes) {
                this->OnReadFileProgress.Broadcast(FileName, (int64)(NumBytes));
            })
        );
    }
}

void UOnlineTitleFileSubsystem::BeginDestroy()
{
    if (this->HasAnyFlags(RF_ClassDefaultObject))
    {
        Super::BeginDestroy();
        return;
    }

    TSharedPtr<class IOnlineTitleFile, ESPMode::ThreadSafe> Handle = this->GetHandle();
    if (Handle.IsValid())
    {
        for (auto i = 0; i < MAX_LOCAL_PLAYERS; i++)
        {
        }
        Handle->ClearOnEnumerateFilesCompleteDelegate_Handle(this->DelegateHandle_OnEnumerateFilesComplete);
        Handle->ClearOnReadFileCompleteDelegate_Handle(this->DelegateHandle_OnReadFileComplete);
        Handle->ClearOnReadFileProgressDelegate_Handle(this->DelegateHandle_OnReadFileProgress);
        
    }

    Super::BeginDestroy();
}

bool UOnlineTitleFileSubsystem::IsSubsystemAvailable()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    return this->IsHandleValid(this->GetHandle());
}

bool UOnlineTitleFileSubsystem::GetFileContents(FString FileName, UFileData*& FileContents)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }

    TArray<uint8> __StoreTemp__FileContents;

    auto __Result = Handle->GetFileContents(FileName, __StoreTemp__FileContents);
    FileContents = UFileData::FromNative(__StoreTemp__FileContents);

    return __Result;
}

bool UOnlineTitleFileSubsystem::ClearFiles()
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ClearFiles();

    return __Result;
}

bool UOnlineTitleFileSubsystem::ClearFile(FString FileName)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return false;
    }


    auto __Result = Handle->ClearFile(FileName);

    return __Result;
}

void UOnlineTitleFileSubsystem::DeleteCachedFiles(bool bSkipEnumerated)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }


    Handle->DeleteCachedFiles(bSkipEnumerated);
}

UOnlineTitleFileSubsystemEnumerateFiles* UOnlineTitleFileSubsystemEnumerateFiles::EnumerateFiles(UOnlineTitleFileSubsystem* Subsystem ,FPagedQueryBP Page)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineTitleFileSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineTitleFileSubsystemEnumerateFiles* Node = NewObject<UOnlineTitleFileSubsystemEnumerateFiles>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = TEXT("OnEnumerateFilesComplete");
    Node->__Store__Page = Page;
    return Node;
}

void UOnlineTitleFileSubsystemEnumerateFiles::Activate()
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

    this->DelegateHandle_OnEnumerateFilesComplete = Handle->AddOnEnumerateFilesCompleteDelegate_Handle(
        FOnEnumerateFilesCompleteDelegate::CreateUObject(this, &UOnlineTitleFileSubsystemEnumerateFiles::HandleCallback_OnEnumerateFilesComplete));
    
    if (!Handle->EnumerateFiles(this->__Store__Page.ToNative()))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'EnumerateFiles' did not start successfully"));
        Handle->ClearOnEnumerateFilesCompleteDelegate_Handle(this->DelegateHandle_OnEnumerateFilesComplete);
        this->OnCallFailed.Broadcast(false, TEXT(""));
        return;
    }
}

void UOnlineTitleFileSubsystemEnumerateFiles::HandleCallback_OnEnumerateFilesComplete(bool bWasSuccessful, const FString& Error)
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
        Handle->ClearOnEnumerateFilesCompleteDelegate_Handle(this->DelegateHandle_OnEnumerateFilesComplete);

    this->OnEnumerateFilesComplete.Broadcast(bWasSuccessful, Error);
}

void UOnlineTitleFileSubsystem::GetFileList(TArray<FCloudFileHeaderBP>& Files)
{
    check(!this->HasAnyFlags(RF_ClassDefaultObject));
    auto Handle = this->GetHandle();
    if (!this->IsHandleValid(Handle))
    {
        return ;
    }

    TArray<FCloudFileHeader> __StoreTemp__Files;

    Handle->GetFileList(__StoreTemp__Files);
    Files = ConvertArrayElements<FCloudFileHeader, FCloudFileHeaderBP>(__StoreTemp__Files, [](const FCloudFileHeader& Val) { return FCloudFileHeaderBP::FromNative(Val); });
}

UOnlineTitleFileSubsystemReadFile* UOnlineTitleFileSubsystemReadFile::ReadFile(UOnlineTitleFileSubsystem* Subsystem ,FString FileName)
{
    if (!IsValid(Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("OnlineTitleFileSubsystem is not available. It's possible the online subsystem you are using doesn't support it. Alternatively, make sure that you are not attempting to use Online Subsystem Blueprints during the Init event in a GameInstance. Add a 'Delay 0.1' node between the Init event and the rest of your logic if you need to use Online Subsystem Blueprints this early in game startup."));
        return nullptr;
    }

    check(!Subsystem->HasAnyFlags(RF_ClassDefaultObject));

    UOnlineTitleFileSubsystemReadFile* Node = NewObject<UOnlineTitleFileSubsystemReadFile>();
    Node->__Store__Subsystem = Subsystem;
    Node->__Store__CallUniquenessId = FString::Printf(TEXT("OnReadFileComplete|%s"), *FileName);
    Node->__Store__FileName = FileName;
    return Node;
}

void UOnlineTitleFileSubsystemReadFile::Activate()
{
    auto Handle = this->__Store__Subsystem->GetHandle();
    if (!this->__Store__Subsystem->IsHandleValid(Handle))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem value not valid when Activate() was called"));
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
        return;
    }

    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        if (this->__Store__Subsystem->__InFlightRequests.Contains(this->__Store__CallUniquenessId))
        {
            UE_LOG(LogOnlineBlueprint, Error, TEXT("There is already a request running with ID '%s'. Wait until it is complete before starting another request with the same parameters."), *this->__Store__CallUniquenessId);
            this->OnCallFailed.Broadcast(false, TEXT(""), int64());
            return;
        }

        this->__Store__Subsystem->__InFlightRequests.Add(this->__Store__CallUniquenessId);
    }

    this->DelegateHandle_OnReadFileComplete = Handle->AddOnReadFileCompleteDelegate_Handle(
        FOnReadFileCompleteDelegate::CreateUObject(this, &UOnlineTitleFileSubsystemReadFile::HandleCallback_OnReadFileComplete));
    this->ProgressDelegateHandle_OnReadFileComplete = Handle->AddOnReadFileProgressDelegate_Handle(
        FOnReadFileProgressDelegate::CreateUObject(this, &UOnlineTitleFileSubsystemReadFile::HandleCallbackProgress_OnReadFileComplete));
    
    if (!Handle->ReadFile(this->__Store__FileName))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Call 'ReadFile' did not start successfully"));
        Handle->ClearOnReadFileCompleteDelegate_Handle(this->DelegateHandle_OnReadFileComplete);
        Handle->ClearOnReadFileProgressDelegate_Handle(this->ProgressDelegateHandle_OnReadFileComplete);
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
        return;
    }
}

void UOnlineTitleFileSubsystemReadFile::HandleCallback_OnReadFileComplete(bool bWasSuccessful, const FString& FileName)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
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
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
        return;
    }
    if (this->__Store__CallUniquenessId != TEXT(""))
    {
        this->__Store__Subsystem->__InFlightRequests.Remove(this->__Store__CallUniquenessId);
    }
        Handle->ClearOnReadFileCompleteDelegate_Handle(this->DelegateHandle_OnReadFileComplete);
        Handle->ClearOnReadFileProgressDelegate_Handle(this->ProgressDelegateHandle_OnReadFileComplete);

    this->OnReadFileComplete.Broadcast(bWasSuccessful, FileName, int64());
}
void UOnlineTitleFileSubsystemReadFile::HandleCallbackProgress_OnReadFileComplete(const FString& FileName, uint64 NumBytes)
{
    if (!IsValid(this->__Store__Subsystem))
    {
        UE_LOG(LogOnlineBlueprint, Error, TEXT("Subsystem was not valid during callback handling"));
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
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
        this->OnCallFailed.Broadcast(false, TEXT(""), int64());
        return;
    }

    this->OnReadFileProgress.Broadcast(false, FileName, (int64)(NumBytes));
}

