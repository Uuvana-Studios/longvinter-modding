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
#include "Interfaces/OnlinePurchaseInterface.h"

#include "OnlinePurchaseSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPurchase_UnexpectedPurchaseReceipt_BP, FUniqueNetIdRepl, UserId);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePurchaseSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlinePurchase, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlinePurchase, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlinePurchase, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnUnexpectedPurchaseReceipt;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlinePurchaseSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Purchase")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Purchase")
    FPurchase_UnexpectedPurchaseReceipt_BP OnUnexpectedPurchaseReceipt;

    UFUNCTION(BlueprintCallable, Category = "Online|Purchase")
    bool IsAllowedToPurchase(FUniqueNetIdRepl UserId);

    // @generator-incompatible
    // void Checkout(const FUniqueNetId& UserId, const FPurchaseCheckoutRequest& CheckoutRequest, const FOnPurchaseCheckoutComplete& Delegate);

    UFUNCTION(BlueprintCallable, Category = "Online|Purchase")
    void FinalizePurchase(FUniqueNetIdRepl UserId, FString ReceiptId);

    // @generator-incompatible
    // void RedeemCode(const FUniqueNetId& UserId, const FRedeemCodeRequest& RedeemCodeRequest, const FOnPurchaseRedeemCodeComplete& Delegate);

    friend class UOnlinePurchaseSubsystemQueryReceipts;
    
    // @generator-incompatible
    // void GetReceipts(const FUniqueNetId& UserId, TArray<FPurchaseReceipt>& OutReceipts);

    friend class UOnlinePurchaseSubsystemFinalizeReceiptValidationInfo;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnlinePurchaseSubsystemQueryReceiptsCallbackPin, FOnlineErrorInfo, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePurchaseSubsystemQueryReceipts : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePurchaseSubsystemQueryReceiptsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePurchaseSubsystemQueryReceiptsCallbackPin OnQueryReceiptsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Purchase", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePurchaseSubsystemQueryReceipts* QueryReceipts(
        UOnlinePurchaseSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, bool bRestoreReceipts
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePurchaseSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnQueryReceiptsComplete(const FOnlineError& Result);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    bool __Store__bRestoreReceipts;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnlinePurchaseSubsystemFinalizeReceiptValidationInfoCallbackPin, FOnlineErrorInfo, Result, FString, ValidationInfo);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlinePurchaseSubsystemFinalizeReceiptValidationInfo : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlinePurchaseSubsystemFinalizeReceiptValidationInfoCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlinePurchaseSubsystemFinalizeReceiptValidationInfoCallbackPin OnFinalizeReceiptValidationInfoComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|Purchase", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlinePurchaseSubsystemFinalizeReceiptValidationInfo* FinalizeReceiptValidationInfo(
        UOnlinePurchaseSubsystem* Subsystem
        ,FUniqueNetIdRepl UserId, FString InReceiptValidationInfo
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlinePurchaseSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnFinalizeReceiptValidationInfoComplete(const FOnlineError& Result, const FString& ValidationInfo);

    UPROPERTY()
    FUniqueNetIdRepl __Store__UserId;

UPROPERTY()
    FString __Store__InReceiptValidationInfo;
};
