// Copyright June Rhodes. All Rights Reserved.

#include "OnlineHelpers.h"

#include "AutoSubsystems/OnlinePartySubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemBlueprintsModule.h"
#include "UObject/Object.h"

FString UOnlineHelpers::Conv_FUniqueNetIdReplToString(FUniqueNetIdRepl InNetId)
{
    return InNetId.ToString();
}

FName UOnlineHelpers::FUniqueNetIdGetType(FUniqueNetIdRepl InNetId)
{
    return InNetId.GetType();
}

bool UOnlineHelpers::FUniqueNetIdIsValid(FUniqueNetIdRepl InNetId)
{
    return InNetId.IsValid();
}

bool UOnlineHelpers::EqualEqual_PartyIdPartyId(UPartyId *A, UPartyId *B)
{
    if (!IsValid(A) && !IsValid(B))
    {
        return true;
    }
    if (!IsValid(A) || !IsValid(B))
    {
        return false;
    }

    return A->ToNative() == B->ToNative();
}

bool UOnlineHelpers::EqualEqual_FUniqueNetIdReplFUniqueNetIdRepl(
    const FUniqueNetIdRepl &InA,
    const FUniqueNetIdRepl &InB)
{
    return InA == InB;
}

bool UOnlineHelpers::IsValid_PartyId(UPartyId *A)
{
    if (!IsValid(A))
    {
        return false;
    }

    if (!A->IsValid())
    {
        return false;
    }

    return true;
}

bool UOnlineHelpers::EqualEqual_LobbyIdLobbyId(ULobbyId *A, ULobbyId *B)
{
    if (!IsValid(A) && !IsValid(B))
    {
        return true;
    }
    if (!IsValid(A) || !IsValid(B))
    {
        return false;
    }

    return A->ToNative() == B->ToNative();
}

bool UOnlineHelpers::IsValid_LobbyId(ULobbyId *A)
{
    if (!IsValid(A))
    {
        return false;
    }

    if (!A->IsValid())
    {
        return false;
    }

    return true;
}

void UOnlineHelpers::GetResolvedConnectStringByName(
    class UOnlineSessionSubsystem *Subsystem,
    FName SessionName,
    FName PortType,
    bool &bWasSuccessful,
    FString &OutConnectInfo)
{
    bWasSuccessful = false;
    OutConnectInfo = TEXT("");

    if (!IsValid(Subsystem))
    {
        return;
    }

    IOnlineSessionPtr SessionPtr = Subsystem->GetHandle();
    if (!SessionPtr.IsValid())
    {
        return;
    }

    bWasSuccessful = SessionPtr->GetResolvedConnectString(SessionName, OutConnectInfo, PortType);
}

void UOnlineHelpers::GetResolvedConnectStringBySearchResult(
    class UOnlineSessionSubsystem *Subsystem,
    FOnlineSessionSearchResultBP SearchResult,
    FName PortType,
    bool &bWasSuccessful,
    FString &OutConnectInfo)
{
    bWasSuccessful = false;
    OutConnectInfo = TEXT("");

    if (!IsValid(Subsystem))
    {
        return;
    }

    IOnlineSessionPtr SessionPtr = Subsystem->GetHandle();
    if (!SessionPtr.IsValid())
    {
        return;
    }

    FOnlineSessionSearchResult SearchResultNative = SearchResult.ToNative();

    bWasSuccessful = SessionPtr->GetResolvedConnectString(SearchResultNative, PortType, OutConnectInfo);
}

FUniqueNetIdRepl UOnlineHelpers::GetControllerUniqueNetId(APlayerController *InPlayerController)
{
    if (!IsValid(InPlayerController))
    {
        return FUniqueNetIdRepl();
    }

    if (InPlayerController->IsLocalPlayerController())
    {
        ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
        if (IsValid(LocalPlayer))
        {
            return LocalPlayer->GetPreferredUniqueNetId();
        }
    }

    UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
    if (IsValid(RemoteNetConnection))
    {
        return RemoteNetConnection->PlayerId;
    }

    UE_LOG(LogOnlineBlueprint, Error, TEXT("Player controller does not have a valid remote network connection"));
    return FUniqueNetIdRepl();
}

UMutablePartyData *UOnlineHelpers::CreateMutablePartyData(UReadablePartyData *InReadablePartyData)
{
    if (!IsValid(InReadablePartyData))
    {
        return nullptr;
    }

    return UMutablePartyData::FromNative(MakeShared<FOnlinePartyData>(InReadablePartyData->ToNativeConst().Get()));
}

UFileData *UOnlineHelpers::CreateFileDataFromString(const FString &InFileData)
{
    FTCHARToUTF8 Data(*InFileData);
    TArray<uint8> Bytes((uint8 *)Data.Get(), Data.Length());
    return UFileData::FromNative(Bytes);
}

FString UOnlineHelpers::ReadFileDataAsString(UFileData *InFileData)
{
    if (!IsValid(InFileData))
    {
        return TEXT("");
    }
    FUTF8ToTCHAR Data((const ANSICHAR *)InFileData->GetData().GetData(), InFileData->GetData().Num());
    return FString(Data.Length(), Data.Get());
}

UFileData *UOnlineHelpers::CreateFileDataFromSaveGame(USaveGame *SaveGame)
{
    if (!IsValid(SaveGame))
    {
        return nullptr;
    }
    TArray<uint8> SaveData;
    if (UGameplayStatics::SaveGameToMemory(SaveGame, SaveData))
    {
        return UFileData::FromNative(SaveData);
    }
    return nullptr;
}

USaveGame *UOnlineHelpers::ReadFileDataAsSaveGame(UFileData *FileData)
{
    if (!IsValid(FileData))
    {
        return nullptr;
    }
    return UGameplayStatics::LoadGameFromMemory(FileData->GetData());
}

UOnlinePartyJoinInfo *UOnlineHelpers::Conv_ULobbyIdToUOnlinePartyJoinInfo(
    FUniqueNetIdRepl InLocalUserId,
    UOnlinePartySubsystem *InPartySubsystem,
    ULobbyId *InLobbyId)
{
    if (!InLocalUserId.IsValid() || !IsValid(InPartySubsystem) || !IsValid(InLobbyId))
    {
        return nullptr;
    }

    TSharedPtr<IOnlinePartySystem, ESPMode::ThreadSafe> PartySystem = InPartySubsystem->GetHandle();
    if (!PartySystem.IsValid())
    {
        return nullptr;
    }

    FString JoinInfo = PartySystem->MakeJoinInfoJson(*InLocalUserId.GetUniqueNetId(), InLobbyId->ToNative());
    if (JoinInfo.IsEmpty())
    {
        return nullptr;
    }

    IOnlinePartyJoinInfoConstPtr JoinInfoPtr = PartySystem->MakeJoinInfoFromJson(JoinInfo);
    if (!JoinInfoPtr.IsValid())
    {
        return nullptr;
    }

    return UOnlinePartyJoinInfo::FromNative(*JoinInfoPtr);
}

TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> GetMessageSanitizerSafe(IOnlineSubsystem *OSS)
{
    auto Identity = OSS->GetIdentityInterface();
    if (!Identity.IsValid())
    {
        return nullptr;
    }

    // On some platforms, it is not safe to obtain the message sanitizer unless the local
    // user is signed in.
    if (Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
    {
        return nullptr;
    }

    FString EmptyPlat = TEXT("");
    return OSS->GetMessageSanitizer(0, EmptyPlat);
}
