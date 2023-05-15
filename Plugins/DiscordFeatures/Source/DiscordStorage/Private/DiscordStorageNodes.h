// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordStorageManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordStorageNodes.generated.h"

UCLASS(Abstract)
class UBaseStorageAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	UDiscordStorageManager* Manager;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReadAsyncEvent, const EDiscordResult, Result, const TArray<uint8>&, Data);

UCLASS()
class UReadAsyncProxy : public UBaseStorageAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnReadAsyncEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnReadAsyncEvent OnError;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Read Async"))
	static UReadAsyncProxy* ReadAsync(UDiscordStorageManager* StorageManager, const FString& Name);

private:
	FString Name;

	UFUNCTION()
	void OnResult(EDiscordResult Result, TArray<uint8>& Data);
};


UCLASS()
class UReadAsyncPartialProxy : public UBaseStorageAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnReadAsyncEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnReadAsyncEvent OnError;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Read Async"))
	static UReadAsyncPartialProxy* ReadAsyncPartial(UDiscordStorageManager* StorageManager, const FString& Name, const FUint64& Offset, const FUint64& Length);

private:
	FString Name;
	uint64 Length;
	uint64 Offset;

	UFUNCTION()
	void OnResult(EDiscordResult Result, TArray<uint8>& Data);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWriteAsyncEvent, const EDiscordResult, Result);


UCLASS()
class UWriteAsyncProxy : public UBaseStorageAsyncProxy
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FOnWriteAsyncEvent OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOnWriteAsyncEvent OnError;

	virtual void Activate();

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Write Async"))
	static UWriteAsyncProxy* WriteAsync(UDiscordStorageManager* StorageManager, const FString& Name, const TArray<uint8>& Data);

private:
	FString Name;
	TArray<uint8> Data;

	UFUNCTION()
	void OnResult(EDiscordResult Result);
};


