// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineUserInterface.h"
#if defined(UE_5_0_OR_LATER)
#include "Online/CoreOnline.h"
#else
#include "UObject/CoreOnline.h"
#endif

#include "OnlineUserRef.generated.h"

UCLASS(BlueprintType, Blueprintable, Transient, Meta = (DontUseGenericSpawnObject))
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineUserRef : public UObject
{
    GENERATED_BODY()

private:
    TSharedPtr<FOnlineUser> User;
    TSharedPtr<FOnlineBlockedPlayer> BlockedPlayer;

protected:
    void AssignFromUser(const TSharedPtr<FOnlineUser> &User);
    void AssignFromBlockedPlayer(const TSharedPtr<FOnlineBlockedPlayer> &BlockedPlayer);

public:
    static UOnlineUserRef *FromUser(const TSharedPtr<FOnlineUser> &User);
    TSharedPtr<FOnlineUser> GetUser() const
    {
        return this->User;
    }

    static UOnlineUserRef *FromBlockedPlayer(const TSharedPtr<FOnlineBlockedPlayer> &BlockedPlayer);
    TSharedPtr<FOnlineBlockedPlayer> GetBlockedPlayer() const
    {
        return this->BlockedPlayer;
    }

    /**
     * Returns the unique player ID for this user.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    FUniqueNetIdRepl GetUserId();

    /**
     * Returns the real name of this user.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    FString GetRealName();

    /**
     * Returns the display name for this user.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    FString GetDisplayName();

    /**
     * Returns an attribute of the user, and whether or not the attribute was found.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Online")
    FString GetUserAttribute(const FString &Key, bool &Found);

    /**
     * Attempt to set a local user attribute. Not supported by default.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Online")
    void SetUserLocalAttribute(const FString &Key, const FString &Value, bool &Success);

protected:
    virtual FUniqueNetIdRepl GetUserId_Native();
    virtual FString GetRealName_Native();
    virtual FString GetDisplayName_Native();
    virtual FString GetUserAttribute_Native(const FString &Key, bool &Found);
    virtual void SetUserLocalAttribute_Native(const FString &Key, const FString &Value, bool &Success);
};

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UUserOnlineAccountRef : public UOnlineUserRef
{
    GENERATED_BODY()

private:
    TSharedPtr<FUserOnlineAccount> UserOnlineAccount;

protected:
    void AssignFromUserOnlineAccount(const TSharedPtr<FUserOnlineAccount> &User);

public:
    static UUserOnlineAccountRef *FromUserOnlineAccount(const TSharedPtr<FUserOnlineAccount> &BlockedPlayer);
    TSharedPtr<FUserOnlineAccount> GetUserOnlineAccount() const
    {
        return this->UserOnlineAccount;
    }

    /**
     * Returns the access token for this user.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetAccessToken();

    /**
     * Returns an authentication attribute of the user, and whether or not the attribute was found.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    FString GetAuthAttribute(FString Key, bool &Found);

    /**
     * Sets an attribute of the user, and whether or not it could be set.
     */
    UFUNCTION(BlueprintPure, Category = "Online")
    bool SetUserAttribute(FString Key, FString Value);
};

/**
 * Contains the query options when looking up external IDs.
 */
USTRUCT(BlueprintType) struct ONLINESUBSYSTEMBLUEPRINTS_API FExternalIdQueryOptionsBP
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    bool bLookupByDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Online")
    FString AuthType;

    static FExternalIdQueryOptionsBP FromNative(const FExternalIdQueryOptions &Obj);
    FExternalIdQueryOptions ToNative();
};