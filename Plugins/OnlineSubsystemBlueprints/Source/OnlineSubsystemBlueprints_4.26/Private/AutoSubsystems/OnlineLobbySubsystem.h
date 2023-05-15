// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "Delegates/DelegateCombinations.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Types/OnlineUserRef.h"
#include "Types/FriendTypes.h"
#include "Types/OnlineAccountCredential.h"
#include "Types/OnlineErrorInfo.h"
#include "Types/OnlineRecentPlayerRef.h"
#include "Types/OnlineLoginStatus.h"
#include "Types/BlockedQueryResultInfo.h"
#include "Types/OnlineUserPrivilege.h"
#include "Types/MessageTypes.h"
#include "Types/ExternalUITypes.h"
#include "Types/OSBFileTypes.h"
#include "Types/OSBPartyTypes.h"
#include "Types/OSBLobbyTypes.h"
#include "Types/OSBSessionTypes.h"
#include "Types/OSBLeaderboardTypes.h"
#include "Types/OSBStatsTypes.h"
#include "Types/OSBAchievementTypes.h"
#include "Types/OSBVoiceChatTypes.h"
#include "Helpers/ArrayConversion.h"
#include "Helpers/UniqueNetIdConversion.h"
#include "Interfaces/OnlineLobbyInterface.h"

#include "OnlineLobbySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLobby_LobbyUpdate_BP, FUniqueNetIdRepl, UserId, ULobbyId*, LobbyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLobby_LobbyDelete_BP, FUniqueNetIdRepl, UserId, ULobbyId*, LobbyId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLobby_MemberConnect_BP, FUniqueNetIdRepl, UserId, ULobbyId*, LobbyId, FUniqueNetIdRepl, MemberId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLobby_MemberUpdate_BP, FUniqueNetIdRepl, UserId, ULobbyId*, LobbyId, FUniqueNetIdRepl, MemberId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLobby_MemberDisconnect_BP, FUniqueNetIdRepl, UserId, ULobbyId*, LobbyId, FUniqueNetIdRepl, MemberId, bool, bWasKicked);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineLobby, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineLobby, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnLobbyUpdate;
    FDelegateHandle DelegateHandle_OnLobbyDelete;
    FDelegateHandle DelegateHandle_OnMemberConnect;
    FDelegateHandle DelegateHandle_OnMemberUpdate;
    FDelegateHandle DelegateHandle_OnMemberDisconnect;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineLobbySubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Lobby")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Lobby")
    FLobby_LobbyUpdate_BP OnLobbyUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Online|Lobby")
    FLobby_LobbyDelete_BP OnLobbyDelete;

    UPROPERTY(BlueprintAssignable, Category = "Online|Lobby")
    FLobby_MemberConnect_BP OnMemberConnect;

    UPROPERTY(BlueprintAssignable, Category = "Online|Lobby")
    FLobby_MemberUpdate_BP OnMemberUpdate;

    UPROPERTY(BlueprintAssignable, Category = "Online|Lobby")
    FLobby_MemberDisconnect_BP OnMemberDisconnect;

    // @generator-incompatible
    // FDateTime GetUtcNow();

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    UOnlineLobbyTransaction* MakeCreateLobbyTransaction(FUniqueNetIdRepl UserId);

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    UOnlineLobbyTransaction* MakeUpdateLobbyTransaction(FUniqueNetIdRepl UserId, ULobbyId* LobbyId);

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    UOnlineLobbyMemberTransaction* MakeUpdateLobbyMemberTransaction(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId);

    friend class UOnlineLobbySubsystemCreateLobby;
    
    friend class UOnlineLobbySubsystemUpdateLobby;
    
    friend class UOnlineLobbySubsystemDeleteLobby;
    
    friend class UOnlineLobbySubsystemConnectLobby;
    
    friend class UOnlineLobbySubsystemDisconnectLobby;
    
    friend class UOnlineLobbySubsystemUpdateMemberSelf;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    bool GetMemberCount(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, int32& OutMemberCount);

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    bool GetMemberUserId(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, int32 MemberIndex, FUniqueNetIdRepl& OutMemberId);

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    bool GetMemberMetadataValue(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId, FString MetadataKey, FVariantDataBP& OutMetadataValue);

    friend class UOnlineLobbySubsystemSearch;
    
    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    bool GetLobbyMetadataValue(FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FString MetadataKey, FVariantDataBP& OutMetadataValue);

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby")
    ULobbyId* ParseSerializedLobbyId(FString InLobbyId);

    friend class UOnlineLobbySubsystemKickMember;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineLobbySubsystemCreateLobbyCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId, ULobby*, Lobby);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemCreateLobby : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemCreateLobbyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemCreateLobbyCallbackPin OnLobbyCreateOrConnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemCreateLobby* CreateLobby(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, UOnlineLobbyTransaction* Transaction
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyCreateOrConnectComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TSharedPtr<class FOnlineLobby>& Lobby);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    UOnlineLobbyTransaction* __Store__Transaction;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineLobbySubsystemUpdateLobbyCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemUpdateLobby : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemUpdateLobbyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemUpdateLobbyCallbackPin OnLobbyOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemUpdateLobby* UpdateLobby(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, UOnlineLobbyTransaction* Transaction
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;

UPROPERTY()
    UOnlineLobbyTransaction* __Store__Transaction;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineLobbySubsystemDeleteLobbyCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemDeleteLobby : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemDeleteLobbyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemDeleteLobbyCallbackPin OnLobbyOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemDeleteLobby* DeleteLobby(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineLobbySubsystemConnectLobbyCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId, ULobby*, Lobby);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemConnectLobby : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemConnectLobbyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemConnectLobbyCallbackPin OnLobbyCreateOrConnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemConnectLobby* ConnectLobby(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyCreateOrConnectComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TSharedPtr<class FOnlineLobby>& Lobby);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineLobbySubsystemDisconnectLobbyCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemDisconnectLobby : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemDisconnectLobbyCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemDisconnectLobbyCallbackPin OnLobbyOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemDisconnectLobby* DisconnectLobby(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineLobbySubsystemUpdateMemberSelfCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemUpdateMemberSelf : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemUpdateMemberSelfCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemUpdateMemberSelfCallbackPin OnLobbyOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemUpdateMemberSelf* UpdateMemberSelf(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, UOnlineLobbyMemberTransaction* Transaction
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;

UPROPERTY()
    UOnlineLobbyMemberTransaction* __Store__Transaction;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineLobbySubsystemSearchCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId, const TArray<ULobbyId*>&, Lobbies);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemSearch : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemSearchCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemSearchCallbackPin OnLobbySearchComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemSearch* Search(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FOnlineLobbySearchQueryBP Query
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbySearchComplete(const FOnlineError& Error, const FUniqueNetId& UserId, const TArray<TSharedRef<const FOnlineLobbyId>>& Lobbies);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FOnlineLobbySearchQueryBP __Store__Query;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlineLobbySubsystemKickMemberCallbackPin, FOnlineErrorInfo, Error, FUniqueNetIdRepl, UserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineLobbySubsystemKickMember : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemKickMemberCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineLobbySubsystemKickMemberCallbackPin OnLobbyOperationComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Lobby", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineLobbySubsystemKickMember* KickMember(
        UOnlineLobbySubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, ULobbyId* LobbyId, FUniqueNetIdRepl MemberId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineLobbySubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnLobbyOperationComplete(const FOnlineError& Error, const FUniqueNetId& UserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    ULobbyId* __Store__LobbyId;

UPROPERTY()
    FUniqueNetIdRepl __Store__MemberId;
};
