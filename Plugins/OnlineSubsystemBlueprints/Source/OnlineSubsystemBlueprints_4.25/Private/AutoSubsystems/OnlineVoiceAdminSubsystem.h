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
#include "Interfaces/OnlineVoiceAdminInterface.h"
#include "Types/OSBVoiceChatTypes.h"

#include "OnlineVoiceAdminSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceAdminSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineVoiceAdmin, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineVoiceAdminSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|VoiceAdmin")
    bool IsSubsystemAvailable();

    friend class UOnlineVoiceAdminSubsystemCreateChannelCredentials;
    
    friend class UOnlineVoiceAdminSubsystemKickParticipant;
    
    friend class UOnlineVoiceAdminSubsystemSetParticipantHardMute;
    };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineVoiceAdminSubsystemCreateChannelCredentialsCallbackPin, FOnlineErrorInfo, Result, FUniqueNetIdRepl, LocalUserId, const TArray<FVoiceAdminChannelCredentialsBP>&, Credentials);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceAdminSubsystemCreateChannelCredentials : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemCreateChannelCredentialsCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemCreateChannelCredentialsCallbackPin OnVoiceAdminCreateChannelCredentialsComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceAdmin", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineVoiceAdminSubsystemCreateChannelCredentials* CreateChannelCredentials(
        UOnlineVoiceAdminSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FString ChannelName, TArray<FUniqueNetIdRepl> TargetUserIds
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineVoiceAdminSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnVoiceAdminCreateChannelCredentialsComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const TArray<FVoiceAdminChannelCredentials>& Credentials);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FString __Store__ChannelName;

UPROPERTY()
    TArray<FUniqueNetIdRepl> __Store__TargetUserIds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnlineVoiceAdminSubsystemKickParticipantCallbackPin, FOnlineErrorInfo, Result, FUniqueNetIdRepl, LocalUserId, FUniqueNetIdRepl, TargetUserId);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceAdminSubsystemKickParticipant : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemKickParticipantCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemKickParticipantCallbackPin OnVoiceAdminKickParticipantComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceAdmin", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineVoiceAdminSubsystemKickParticipant* KickParticipant(
        UOnlineVoiceAdminSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FString ChannelName, FUniqueNetIdRepl TargetUserId
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineVoiceAdminSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnVoiceAdminKickParticipantComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FString __Store__ChannelName;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnlineVoiceAdminSubsystemSetParticipantHardMuteCallbackPin, FOnlineErrorInfo, Result, FUniqueNetIdRepl, LocalUserId, FUniqueNetIdRepl, TargetUserId, bool, bIsNowMuted);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineVoiceAdminSubsystemSetParticipantHardMute : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemSetParticipantHardMuteCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FOnlineVoiceAdminSubsystemSetParticipantHardMuteCallbackPin OnVoiceAdminSetParticipantHardMuteComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceAdmin", meta = (BlueprintInternalUseOnly = "true"))
    static UOnlineVoiceAdminSubsystemSetParticipantHardMute* SetParticipantHardMute(
        UOnlineVoiceAdminSubsystem* Subsystem
        ,FUniqueNetIdRepl LocalUserId, FString ChannelName, FUniqueNetIdRepl TargetUserId, bool bMuted
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UOnlineVoiceAdminSubsystem* __Store__Subsystem;
    UPROPERTY()
    FString __Store__CallUniquenessId;
    void HandleCallback_OnVoiceAdminSetParticipantHardMuteComplete(const FOnlineError& Result, const FUniqueNetId& LocalUserId, const FUniqueNetId& TargetUserId, bool bIsNowMuted);

    UPROPERTY()
    FUniqueNetIdRepl __Store__LocalUserId;

UPROPERTY()
    FString __Store__ChannelName;

UPROPERTY()
    FUniqueNetIdRepl __Store__TargetUserId;

UPROPERTY()
    bool __Store__bMuted;
};
