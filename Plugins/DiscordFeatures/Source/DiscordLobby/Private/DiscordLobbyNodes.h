// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordLobbyManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiscordLobbyNodes.generated.h"

// TODO: move this class to its own file.
UCLASS()
class UDiscordLobbyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction", meta = (AutoCreateRefTerm="Metadata"))
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetLobbyTransactionProperties(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const EDiscordLobbyType Type, const TMap<FString, FString> Metadata, const int64 UserId, const int64 Capacity, const bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetType(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const EDiscordLobbyType Type = EDiscordLobbyType::Private);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetOwner(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const int64 UserId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetCapacity(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const int64 Capacity);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetMetadata(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const FString & Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& DeleteMetadata(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const FString & Key);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction")
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction& SetLocked(UPARAM(ref) FDiscordLobbyTransaction& Transaction, const bool bLocked);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Search")
	static UPARAM(DisplayName = "Search Query") FDiscordLobbySearchQuery& Filter(UPARAM(ref) FDiscordLobbySearchQuery& SearchQuery, const FString& Key, const EDiscordLobbySearchComparison Comparison, const EDiscordLobbySearchCast Cast, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Search")
	static UPARAM(DisplayName = "Search Query") FDiscordLobbySearchQuery& Sort(UPARAM(ref) FDiscordLobbySearchQuery& SearchQuery, const FString& Key, const EDiscordLobbySearchCast Cast, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Search")
	static UPARAM(DisplayName = "Search Query") FDiscordLobbySearchQuery& Limit(UPARAM(ref) FDiscordLobbySearchQuery& SearchQuery, const int64 NewLimit);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Search")
	static UPARAM(DisplayName = "Search Query") FDiscordLobbySearchQuery& Distance(UPARAM(ref) FDiscordLobbySearchQuery& SearchQuery, const EDiscordLobbySearchDistance Distance);
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction", meta=(DisplayName = "Set Metadata"))
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyMemberTransaction& Member_SetMetadata(UPARAM(ref) FDiscordLobbyMemberTransaction& Transaction, const FString& Key, const FString& Value);
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby|Transaction", meta = (DisplayName = "Delete Metadata"))
	static UPARAM(DisplayName = "Transaction") FDiscordLobbyMemberTransaction& Member_DeleteMetadata(UPARAM(ref) FDiscordLobbyMemberTransaction& Transaction, const FString& Key);

};

UCLASS(Abstract)
class UBaseLobbyAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
protected:
	void Error(const TCHAR* Error);

	UPROPERTY()
	UDiscordLobbyManager* Manager;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLobbyEvent, EDiscordResult, Result, const FDiscordLobby&, Lobby);

UCLASS()
class UCreateLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnLobbyCreated;

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Create Lobby"))
	static UCreateLobbyProxy* CreateLobby(UDiscordLobbyManager* LobbyManager, const FDiscordLobbyTransaction& Transaction);

	virtual void Activate();
	
private:
	FDiscordLobbyTransaction LobbyTransaction;

	UFUNCTION()
	void OnResult(EDiscordResult Result, FDiscordLobby& Lobby);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordLobbyResultEvent, EDiscordResult, Result);

UCLASS()
class UUpdateLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Update Lobby"))
	static UUpdateLobbyProxy* UpdateLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const FDiscordLobbyTransaction& Transaction);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	FDiscordLobbyTransaction Transaction;
	int64 LobbyId;
};

UCLASS()
class UDeleteLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Delete Lobby"))
	static UDeleteLobbyProxy* DeleteLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
};

UCLASS()
class UConnectLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnLobbyConnected;

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Connect Lobby"))
	static UConnectLobbyProxy* ConnectLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const FString& LobbySecret);

	virtual void Activate();
	
private:
	int64 LobbyId;
	FString LobbySecret;

	UFUNCTION()
	void OnResult(EDiscordResult Result, FDiscordLobby& Lobby);
};

UCLASS()
class UConnectLobbyWithActivitySecretProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnLobbyConnected;

	UPROPERTY(BlueprintAssignable)
	FLobbyEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Connect Lobby With Activity Secret"))
	static UConnectLobbyWithActivitySecretProxy* ConnectLobbyWithActivitySecret(UDiscordLobbyManager* LobbyManager, const FString& ActivitySecret);

	virtual void Activate();
	
private:
	FString ActivitySecret;

	UFUNCTION()
	void OnResult(EDiscordResult Result, FDiscordLobby& Lobby);
};


UCLASS()
class UDisconnectLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Disconnect Lobby"))
	static UDisconnectLobbyProxy* DisconnectLobby(UDiscordLobbyManager* LobbyManager, const int64 LobbyId);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
};


UCLASS()
class UUpdateMemberLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Update Member"))
	static UUpdateMemberLobbyProxy* UpdateMember(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const int64 UserId, const FDiscordLobbyMemberTransaction& Transaction);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
	int64 UserId;
	FDiscordLobbyMemberTransaction Transaction;
};


UCLASS()
class USendLobbyMessageProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Send Lobby Message"))
	static USendLobbyMessageProxy* SendLobbyMessage(UDiscordLobbyManager* LobbyManager, const int64 LobbyId, const TArray<uint8>& Data);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
	TArray<uint8> Message;
};

UCLASS()
class USearchLobbyProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Search"))
	static USearchLobbyProxy* Search(UDiscordLobbyManager* LobbyManager, const FDiscordLobbySearchQuery& SearchQuery);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	FDiscordLobbySearchQuery SearchQuery;
};

UCLASS()
class UConnectVoiceProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Connect Voice"))
	static UConnectVoiceProxy* ConnectVoice(UDiscordLobbyManager* LobbyManager, const int64 LobbyId);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
};

UCLASS()
class UDisconnectVoiceProxy : public UBaseLobbyAsyncProxy
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyResultEvent OnError;

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Disconnect Voice"))
	static UDisconnectVoiceProxy* DisconnectVoice(UDiscordLobbyManager* LobbyManager, const int64 LobbyId);

	virtual void Activate();

private:
	UFUNCTION()
	void OnResult(EDiscordResult Result);

	int64 LobbyId;
};






