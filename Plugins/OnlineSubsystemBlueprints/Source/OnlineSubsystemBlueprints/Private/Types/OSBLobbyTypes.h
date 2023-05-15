// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineLobbyInterface.h"
#include "OSBSessionTypes.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OSBLobbyTypes.generated.h"

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API ULobbyId : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<const FOnlineLobbyId> LobbyId;

public:
    static ULobbyId *FromNative(const FOnlineLobbyId &InObj);
    static ULobbyId *FromNative(const TSharedPtr<const FOnlineLobbyId> &InObj);
    const FOnlineLobbyId &ToNative() const
    {
        return this->LobbyId.ToSharedRef().Get();
    }
    bool IsValid() const
    {
        return this->LobbyId.IsValid();
    }

    UFUNCTION(BlueprintPure, Category = "Online")
    FString ToDebugString()
    {
        if (!this->IsValid())
        {
            return TEXT("");
        }

        return this->LobbyId->ToDebugString();
    }
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API ULobby : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineLobby> Lobby;

public:
    UFUNCTION(BlueprintPure, Category = "Online")
    ULobbyId *GetId();

    UFUNCTION(BlueprintPure, Category = "Online")
    FUniqueNetIdRepl GetOwnerId();

    static ULobby *FromNative(TSharedPtr<FOnlineLobby> InObj);
    TSharedPtr<FOnlineLobby> ToNative() const
    {
        return this->Lobby;
    }
    bool IsValid() const
    {
        return this->Lobby.IsValid();
    }
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbyTransaction : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineLobbyTransaction> Txn;

public:
    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetMetadata(const FString &Key, const FVariantDataBP &Value);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetMetadataByMap(const TMap<FString, FVariantDataBP> &Metadata);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void DeleteMetadata(const FString &Key);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void DeleteMetadataByArray(const TArray<FString> &MetadataKeys);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetLocked(bool Locked);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetCapacity(int64 Capacity);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetPublic(bool Public);

    static UOnlineLobbyTransaction *FromNative(TSharedPtr<FOnlineLobbyTransaction> InObj);
    TSharedPtr<FOnlineLobbyTransaction> ToNative() const
    {
        return this->Txn;
    }
    bool IsValid() const
    {
        return this->Txn.IsValid();
    }
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbyMemberTransaction : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineLobbyMemberTransaction> Txn;

public:
    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetMetadata(const FString &Key, const FVariantDataBP &Value);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void SetMetadataByMap(const TMap<FString, FVariantDataBP> &Metadata);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void DeleteMetadata(const FString &Key);

    UFUNCTION(BlueprintCallable, Category = "Online")
    void DeleteMetadataByArray(const TArray<FString> &MetadataKeys);

    static UOnlineLobbyMemberTransaction *FromNative(TSharedPtr<FOnlineLobbyMemberTransaction> InObj);
    TSharedPtr<FOnlineLobbyMemberTransaction> ToNative() const
    {
        return this->Txn;
    }
    bool IsValid() const
    {
        return this->Txn.IsValid();
    }
};

UENUM(BlueprintType, Meta = (ScriptName = "EOnlineLobbySearchQueryFilterComparator"))
enum class EOnlineLobbySearchQueryFilterComparator_ : uint8
{
    Equal = 0,
    NotEqual,
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual,
};

USTRUCT(BlueprintType) struct FOnlineLobbySearchQueryFilterBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FVariantDataBP Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    EOnlineLobbySearchQueryFilterComparator_ Comparison;

    static FOnlineLobbySearchQueryFilterBP FromNative(const FOnlineLobbySearchQueryFilter &InObj);
    FOnlineLobbySearchQueryFilter ToNative() const;
};

USTRUCT(BlueprintType) struct FOnlineLobbySearchQueryBP
{
    GENERATED_BODY()

public:
    /** The search filters. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    TArray<FOnlineLobbySearchQueryFilterBP> Filters;

    /** The number of results to return, if HasLimit is true. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    int64 Limit;

    /** If false, an unlimited number of results will be returned. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool HasLimit;

    static FOnlineLobbySearchQueryBP FromNative(const FOnlineLobbySearchQuery &InObj);
    FOnlineLobbySearchQuery ToNative() const;
};