// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordStorageManager.generated.h"


USTRUCT(BlueprintType)
struct FDiscordFileStat
{
	GENERATED_BODY()
private:
	friend class UDiscordStorageManager;
	FDiscordFileStat(FRawDiscord::DiscordFileStat* const Raw);

public:
	FDiscordFileStat() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Storage")
	FString FileName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Storage")
	FUint64 Size = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Storage")
	FUint64 LastModified = 0;
};

DECLARE_DELEGATE_TwoParams(FReadAsyncCallback, EDiscordResult, TArray<uint8>&);

UCLASS()
class DISCORDSTORAGE_API UDiscordStorageManager : public UDiscordManager
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure, meta = (CompactNodeTitle = "STORAGE MANAGER", BlueprintAutocast))
	static UPARAM(DisplayName = "Storage Manager") UDiscordStorageManager* GetStorageManager(UDiscordCore* const DiscordCore);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "Path") FString GetPath();

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "Read") int32 Read(const FString& Name, UPARAM(ref) TArray<uint8>& Buffer);

	void ReadAsync(const FString& Name, const FReadAsyncCallback& Callback);

	void ReadAsyncPartial(const FString& Name, const uint64 Offset, const uint64 Length, const FReadAsyncCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage")
	void Write(const FString& Name, TArray<uint8>& Data);

	void WriteAsync(const FString& Name, TArray<uint8>& Data, const FDiscordResultCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage")
	void Delete(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "Exists") bool Exists(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "File Stat") FDiscordFileStat Stat(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "Count") int32 Count();

	UFUNCTION(BlueprintCallable, Category = "Discord|Storage", BlueprintPure)
	UPARAM(DisplayName = "File Stat") FDiscordFileStat StatAt(const int32 Index);
};
