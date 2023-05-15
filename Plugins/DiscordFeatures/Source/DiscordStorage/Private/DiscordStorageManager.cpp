// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordStorageManager.h"
#include "DiscordManagerUtils.h"

FDiscordFileStat::FDiscordFileStat(FRawDiscord::DiscordFileStat* const Raw)
{
	FileName	 = UTF8_TO_TCHAR(Raw->filename);
	LastModified = Raw->last_modified;
	Size	     = Raw->size;
}

/* static */ UDiscordStorageManager* UDiscordStorageManager::GetStorageManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->StorageManager)
	{
		return Cast<UDiscordStorageManager>(DiscordCore->StorageManager);
	}

	UDiscordStorageManager* const Manager = NewObject<UDiscordStorageManager>();

	DiscordCore->StorageManager = Manager;

	Manager->DiscordCore = DiscordCore;

	return Manager;
}

FString	UDiscordStorageManager::GetPath()
{
	FRawDiscord::DiscordPath Path;
	FMemory::Memzero(Path);
	STORAGE_CALL_RAW_CHECKED(get_path, &Path);
	return UTF8_TO_TCHAR(Path);
}

int32 UDiscordStorageManager::Read(const FString& Name, TArray<uint8>& Buffer)
{
	uint32 Val = 0;
	STORAGE_CALL_RAW_CHECKED(read, TCHAR_TO_UTF8(*Name), Buffer.GetData(), Buffer.Num(), &Val);
	return Val;
}


static void ReadAsync_Callback(void* CallbackData, FRawDiscord::EDiscordResult Result, uint8* Data, uint32 Length)
{
	FReadAsyncCallback* const Cb = (FReadAsyncCallback*)CallbackData;

	TArray<uint8> Uint8Data(Data, static_cast<uint32>(Length));

	Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result), Uint8Data);

	delete Cb;
};

void UDiscordStorageManager::ReadAsync(const FString& Name, const FReadAsyncCallback& Callback)
{
	STORAGE_CALL_RAW_CHECKED(read_async, TCHAR_TO_UTF8(*Name), new FReadAsyncCallback(Callback), ReadAsync_Callback);
}

void UDiscordStorageManager::ReadAsyncPartial(const FString& Name, const uint64 Offset, const uint64 Length, const FReadAsyncCallback& Callback)
{
	STORAGE_CALL_RAW_CHECKED(read_async_partial, TCHAR_TO_UTF8(*Name), Offset, Length, new FReadAsyncCallback(Callback), ReadAsync_Callback);
}

void UDiscordStorageManager::Write(const FString& Name, TArray<uint8>& Data)
{
	STORAGE_CALL_RAW_CHECKED(write, TCHAR_TO_UTF8(*Name), Data.GetData(), Data.Num());
}

void UDiscordStorageManager::WriteAsync(const FString& Name, TArray<uint8>& Data, const FDiscordResultCallback& Callback)
{
	STORAGE_CALL_RAW_CHECKED(write_async, TCHAR_TO_UTF8(*Name), Data.GetData(), Data.Num(), new FDiscordResultCallback(Callback), FDiscordManagerUtils::DiscordResultCallback_AutoDelete);
}

void UDiscordStorageManager::Delete(const FString& Name)
{
	STORAGE_CALL_RAW_CHECKED(delete_, TCHAR_TO_UTF8(*Name));
}

bool UDiscordStorageManager::Exists(const FString& Name)
{
	bool bExists = false;
	STORAGE_CALL_RAW_CHECKED(exists, TCHAR_TO_UTF8(*Name), &bExists);
	return bExists;
}

FDiscordFileStat UDiscordStorageManager::Stat(const FString& Name)
{
	FRawDiscord::DiscordFileStat Raw;
	FMemory::Memzero(Raw);

	STORAGE_CALL_RAW_CHECKED(stat, TCHAR_TO_UTF8(*Name), &Raw);

	return FDiscordFileStat(&Raw);
}

int32 UDiscordStorageManager::Count()
{
	int32 Val = 0;
	STORAGE_CALL_RAW_CHECKED(count, &Val);
	return Val;
}

FDiscordFileStat UDiscordStorageManager::StatAt(const int32 Index)
{
	FRawDiscord::DiscordFileStat Raw;
	FMemory::Memzero(Raw);

	STORAGE_CALL_RAW_CHECKED(stat_at, Index, &Raw);

	return FDiscordFileStat(&Raw);
}

#undef STORAGE_CALL_RAW_CHECKED
