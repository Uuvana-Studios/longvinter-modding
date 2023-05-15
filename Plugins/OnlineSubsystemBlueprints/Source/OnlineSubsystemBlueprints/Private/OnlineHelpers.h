// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "AutoSubsystems/OnlinePartySubsystem.h"
#include "AutoSubsystems/OnlineSessionSubsystem.h"
#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/OSBFileTypes.h"
#include "Types/OSBPartyTypes.h"

#include "OnlineHelpers.generated.h"

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /** Convert the unique net ID to it's string representation */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "ToString (FUniqueNetIdRepl)",
             CompactNodeTitle = "->",
             Keywords = "FUniqueNetIdRepl netid cast convert to string",
             BlueprintAutocast))
    static FString Conv_FUniqueNetIdReplToString(FUniqueNetIdRepl InNetId);

    /** Return the subsystem type of the unique net ID */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "Get Subsystem Type",
             Keywords = "FUniqueNetIdRepl netid subsystem type name",
             BlueprintAutocast))
    static FName FUniqueNetIdGetType(FUniqueNetIdRepl InNetId);

    /** Return true if the unique net ID is valid */
    UFUNCTION(
        BlueprintPure,
        Category = "Online",
        Meta =
            (DisplayName = "Is Valid (FUniqueNetIdRepl)",
             Keywords = "FUniqueNetIdRepl netid subsystem is valid",
             BlueprintAutocast))
    static bool FUniqueNetIdIsValid(FUniqueNetIdRepl InNetId);

    /** Returns true if the unique net IDs are equal (A == B) */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Equal Unique Net Id", CompactNodeTitle = "==", Keywords = "== equal"),
        Category = "Online")
    static bool EqualEqual_FUniqueNetIdReplFUniqueNetIdRepl(const FUniqueNetIdRepl &InA, const FUniqueNetIdRepl &InB);

    /** Returns true if the party IDs are equal (A == B) */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Equal Party Id", CompactNodeTitle = "==", Keywords = "== equal"),
        Category = "Online")
    static bool EqualEqual_PartyIdPartyId(UPartyId *A, UPartyId *B);

    /** Returns true if the party ID is valid */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Party Id Is Valid?", CompactNodeTitle = "Valid?", Keywords = "is valid"),
        Category = "Online")
    static bool IsValid_PartyId(UPartyId *A);

    /** Returns true if the lobby IDs are equal (A == B) */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Equal Lobby Id", CompactNodeTitle = "==", Keywords = "== equal"),
        Category = "Online")
    static bool EqualEqual_LobbyIdLobbyId(ULobbyId *A, ULobbyId *B);

    /** Returns true if the lobby ID is valid */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Lobby Id Is Valid?", CompactNodeTitle = "Valid?", Keywords = "is valid"),
        Category = "Online")
    static bool IsValid_LobbyId(ULobbyId *A);

    /** Returns the primary party type */
    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Primary Party Type"), Category = "Online")
    static int64 GetPrimaryPartyType()
    {
        return (int64)IOnlinePartySystem::GetPrimaryPartyTypeId().GetValue();
    }

    /** Gets the connection string for a session by name */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Get Resolved Connect String (from Session Name)"),
        Category = "Online|Sessions")
    static void GetResolvedConnectStringByName(
        class UOnlineSessionSubsystem *Subsystem,
        FName SessionName,
        FName PortType,
        bool &bWasSuccessful,
        FString &OutConnectInfo);

    /** Gets the connection string for a search result */
    UFUNCTION(
        BlueprintPure,
        meta = (DisplayName = "Get Resolved Connect String (from Search Result)"),
        Category = "Online|Sessions")
    static void GetResolvedConnectStringBySearchResult(
        class UOnlineSessionSubsystem *Subsystem,
        FOnlineSessionSearchResultBP SearchResult,
        FName PortType,
        bool &bWasSuccessful,
        FString &OutConnectInfo);

    /** Gets the unique net ID for the player controller */
    UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Controller Unique Net Id"), Category = "Online")
    static FUniqueNetIdRepl GetControllerUniqueNetId(APlayerController *PlayerController);

    /**
     * Create a mutable party data based on read-only party data. You must use this if you want to modify the party data
     * you got from GetPartyData so that you can pass it to UpdatePartyData.
     */
    UFUNCTION(
        BlueprintCallable,
        meta = (DisplayName = "Create Modifiable Party Data", Keywords = "modify party data set"),
        Category = "Online")
    static UMutablePartyData *CreateMutablePartyData(UReadablePartyData *ReadOnlyPartyData);

    /**
     * Create file data from a string; that is, the string will be the entire contents of the file.
     */
    UFUNCTION(
        BlueprintCallable,
        meta =
            (DisplayName = "Create File Data from String",
             Keywords = "file data filedata create user title cloud string"),
        Category = "Online|Files")
    static UFileData *CreateFileDataFromString(const FString &String);

    /**
     * Read file data as a string, and return the string value.
     */
    UFUNCTION(
        BlueprintCallable,
        meta = (DisplayName = "Read File Data as String", Keywords = "file data filedata read user title cloud string"),
        Category = "Online|Files")
    static FString ReadFileDataAsString(UFileData *FileData);

    /**
     * Create file data from a save game.
     */
    UFUNCTION(
        BlueprintCallable,
        meta =
            (DisplayName = "Create File Data from Save Game",
             Keywords = "file data filedata create user title cloud save game"),
        Category = "Online|Files")
    static UFileData *CreateFileDataFromSaveGame(USaveGame *SaveGame);

    /**
     * Read file data as a string, and return the string value.
     */
    UFUNCTION(
        BlueprintCallable,
        meta =
            (DisplayName = "Read File Data as Save Game",
             Keywords = "file data filedata read user title cloud save game"),
        Category = "Online|Files")
    static USaveGame *ReadFileDataAsSaveGame(UFileData *FileData);

    /**
     * Convert a lobby ID into a joinable party info.
     */
    UFUNCTION(
        BlueprintCallable,
        meta =
            (DisplayName = "Convert Lobby Id to Party Join Info",
             Keywords = "convert lobby id party join info",
             BlueprintAutocast),
        Category = "Online")
    static UOnlinePartyJoinInfo *Conv_ULobbyIdToUOnlinePartyJoinInfo(
        FUniqueNetIdRepl InLocalUserId,
        UOnlinePartySubsystem *InPartySubsystem,
        ULobbyId *InLobbyId);
};

TSharedPtr<class IMessageSanitizer, ESPMode::ThreadSafe> GetMessageSanitizerSafe(IOnlineSubsystem *OSS);