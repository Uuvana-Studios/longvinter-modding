// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "OnlineSubsystemTypes.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OSBFileTypes.generated.h"

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UFileData : public UObject
{
    GENERATED_BODY()

private:
    TArray<uint8> Data;

public:
    static UFileData *FromNative(const TArray<uint8> &InData);
    TArray<uint8> ToNative() const
    {
        return this->Data;
    };
    const TArray<uint8> &GetData() const
    {
        return this->Data;
    };
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FCloudFileHeaderBP
{
    GENERATED_BODY()

public:
    /** Hash value, if applicable, of the given file contents */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    FString Hash;

    /** The hash algorithm used to sign this file */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    FName HashType;

    /** Filename as downloaded */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    FString DLName;

    /** Logical filename, maps to the downloaded filename */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    FString FileName;

    /** File size */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    int32 FileSize;

    /** The full URL to download the file if it is stored in a CDN or separate host site */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    FString URL;

    /** The chunk id this file represents */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Online|Files")
    int64 ChunkID;

    static FCloudFileHeaderBP FromNative(const FCloudFileHeader &FileHeader);
};

USTRUCT(BlueprintType)
struct ONLINESUBSYSTEMBLUEPRINTS_API FPagedQueryBP
{
    GENERATED_BODY()

public:
    /** The first entry to fetch, starting at 0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Files")
    int32 Start;

    /** The number of entries to fetch. -1 means ALL. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online|Files")
    int32 Count;

    FPagedQuery ToNative() const;
};