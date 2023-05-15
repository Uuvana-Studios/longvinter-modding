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
#include "Types/OSBVoiceChatTypes.h"

#include "VoiceChatUser.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChatUser_OnVoiceChatAvailableAudioDevicesChangedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChatUser_OnVoiceChatReconnectedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVoiceChatUser_OnVoiceChatConnectedDelegate_BP);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatDisconnectedDelegate_BP, FVoiceChatResultBP, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatLoggedInDelegate_BP, FString, PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatLoggedOutDelegate_BP, FString, PlayerName);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatCallStatsUpdatedDelegate_BP, const FVoiceChatCallStats&, CallStats);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUser_OnVoiceChatChannelJoinedDelegate_BP, FString, ChannelName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUser_OnVoiceChatChannelExitedDelegate_BP, FString, ChannelName, FVoiceChatResultBP, Reason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUser_OnVoiceChatPlayerAddedDelegate_BP, FString, ChannelName, FString, PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUser_OnVoiceChatPlayerRemovedDelegate_BP, FString, ChannelName, FString, PlayerName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoiceChatUser_OnVoiceChatPlayerTalkingUpdatedDelegate_BP, FString, ChannelName, FString, PlayerName, bool, bIsTalking);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoiceChatUser_OnVoiceChatPlayerMuteUpdatedDelegate_BP, FString, ChannelName, FString, PlayerName, bool, bIsMuted);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVoiceChatUser_OnVoiceChatPlayerVolumeUpdatedDelegate_BP, FString, ChannelName, FString, PlayerName, float, Volume);

UCLASS(BlueprintType, Transient, Meta = (DontUseGenericSpawnObject, IsBlueprintBase = "false"))
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUser : public UObject
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    FDelegateHandle DelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatReconnectedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatConnectedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatDisconnectedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatLoggedInDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatLoggedOutDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatChannelJoinedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatChannelExitedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerAddedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerRemovedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate;
    FDelegateHandle DelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate;
    void RegisterFunctionDelegates();
    void UnregisterFunctionDelegates();

public:
private:
    bool bOwnsHandle;
    IVoiceChatUser* HandlePtr;
    IVoiceChatUser* GetHandle();
    bool IsHandleValid(IVoiceChatUser* InHandle);

public:
    UVoiceChatUser();
    void AssignUserHandle(IVoiceChatUser* InHandlePtr, bool bInOwnsHandle);
    virtual void BeginDestroy() override;

    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatAvailableAudioDevicesChangedDelegate_BP OnVoiceChatAvailableAudioDevicesChanged;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatAvailableAudioDevicesChangedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatReconnectedDelegate_BP OnVoiceChatReconnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatReconnectedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatConnectedDelegate_BP OnVoiceChatConnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatConnectedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatDisconnectedDelegate_BP OnVoiceChatDisconnected;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatDisconnectedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatLoggedInDelegate_BP OnVoiceChatLoggedIn;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatLoggedInDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatLoggedOutDelegate_BP OnVoiceChatLoggedOut;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatLoggedOutDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatChannelJoinedDelegate_BP OnVoiceChatChannelJoined;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatChannelJoinedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatChannelExitedDelegate_BP OnVoiceChatChannelExited;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatChannelExitedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerAddedDelegate_BP OnVoiceChatPlayerAdded;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerAddedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerRemovedDelegate_BP OnVoiceChatPlayerRemoved;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerRemovedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerTalkingUpdatedDelegate_BP OnVoiceChatPlayerTalkingUpdated;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerTalkingUpdatedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerMuteUpdatedDelegate_BP OnVoiceChatPlayerMuteUpdated;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerMuteUpdatedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "Online|VoiceChatUser")
    FVoiceChatUser_OnVoiceChatPlayerVolumeUpdatedDelegate_BP OnVoiceChatPlayerVolumeUpdated;
    FDelegateHandle FunctionDelegateHandle_FOnVoiceChatPlayerVolumeUpdatedDelegate;
    
    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetSetting(FString Name, FString Value);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString GetSetting(FString Name);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioInputVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioOutputVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetAudioInputVolume();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetAudioOutputVolume();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioInputDeviceMuted(bool bIsMuted);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetAudioOutputDeviceMuted(bool bIsMuted);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool GetAudioInputDeviceMuted();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool GetAudioOutputDeviceMuted();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetAvailableInputDevices();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetAvailableOutputDevices();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetInputDevice(FString InputDevice);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetOutputDevice(FString OutputDevice);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetInputDevice();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetOutputDevice();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetDefaultInputDevice();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetDefaultOutputDevice();

    friend class UVoiceChatUserConnect;
    
    friend class UVoiceChatUserDisconnect;
    
    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsConnecting();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsConnected();

    friend class UVoiceChatUserLogin;
    
    friend class UVoiceChatUserLogout;
    
    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsLoggingIn();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsLoggedIn();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetLoggedInPlayerName();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void BlockPlayers(TArray<FString> PlayerNames);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void UnblockPlayers(TArray<FString> PlayerNames);

    friend class UVoiceChatUserJoinChannel;
    
    friend class UVoiceChatUserLeaveChannel;
    
    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void Set3DPosition(FString ChannelName, FVector SpeakerPosition, FVector ListenerPosition, FVector ListenerForwardDirection, FVector ListenerUpDirection);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    TArray<FString> GetPlayersInChannel(FString ChannelName);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    EVoiceChatChannelType_ GetChannelType(FString ChannelName);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsPlayerTalking(FString PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetPlayerMuted(FString PlayerName, bool bMuted);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    bool IsPlayerMuted(FString PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void SetPlayerVolume(FString PlayerName, float Volume);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    float GetPlayerVolume(FString PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToAllChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToNoChannels();

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    void TransmitToSpecificChannel(FString ChannelName);

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    EVoiceChatTransmitMode_ GetTransmitMode();

    UFUNCTION(BlueprintPure, Category = "Online|VoiceChatUser")
    FString GetTransmitChannel();

    // @generator-incompatible
    // FDelegateHandle StartRecording(const FOnVoiceChatRecordSamplesAvailableDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void StopRecording(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatAfterCaptureAudioReadDelegate(const FOnVoiceChatAfterCaptureAudioReadDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatAfterCaptureAudioReadDelegate(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatBeforeCaptureAudioSentDelegate(const FOnVoiceChatBeforeCaptureAudioSentDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatBeforeCaptureAudioSentDelegate(FDelegateHandle Handle);

    // @generator-incompatible
    // FDelegateHandle RegisterOnVoiceChatBeforeRecvAudioRenderedDelegate(const FOnVoiceChatBeforeRecvAudioRenderedDelegate::FDelegate& Delegate);

    // @generator-incompatible
    // void UnregisterOnVoiceChatBeforeRecvAudioRenderedDelegate(FDelegateHandle Handle);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString InsecureGetLoginToken(FString PlayerName);

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser")
    FString InsecureGetJoinToken(FString ChannelName, EVoiceChatChannelType_ ChannelType, FVoiceChatChannel3dPropertiesBP Channel3dProperties);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUserConnectCallbackPin, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserConnect : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserConnectCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserConnectCallbackPin OnVoiceChatConnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserConnect* Connect(
        UVoiceChatUser* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatConnectComplete(const FVoiceChatResult& Result);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoiceChatUserDisconnectCallbackPin, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserDisconnect : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserDisconnectCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserDisconnectCallbackPin OnVoiceChatDisconnectComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserDisconnect* Disconnect(
        UVoiceChatUser* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatDisconnectComplete(const FVoiceChatResult& Result);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUserLoginCallbackPin, FString, PlayerName, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLogin : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLoginCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLoginCallbackPin OnVoiceChatLoginComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLogin* Login(
        UVoiceChatUser* Subsystem
        ,int32 PlatformId, FString PlayerName, FString Credentials
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatLoginComplete(const FString& PlayerName, const FVoiceChatResult& Result);

    UPROPERTY()
    int32 __Store__PlatformId;

UPROPERTY()
    FString __Store__PlayerName;

UPROPERTY()
    FString __Store__Credentials;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUserLogoutCallbackPin, FString, PlayerName, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLogout : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLogoutCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLogoutCallbackPin OnVoiceChatLogoutComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLogout* Logout(
        UVoiceChatUser* Subsystem
        
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatLogoutComplete(const FString& PlayerName, const FVoiceChatResult& Result);

    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUserJoinChannelCallbackPin, FString, ChannelName, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserJoinChannel : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserJoinChannelCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserJoinChannelCallbackPin OnVoiceChatChannelJoinComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserJoinChannel* JoinChannel(
        UVoiceChatUser* Subsystem
        ,FString ChannelName, FString ChannelCredentials, EVoiceChatChannelType_ ChannelType, FVoiceChatChannel3dPropertiesBP Channel3dProperties
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatChannelJoinComplete(const FString& ChannelName, const FVoiceChatResult& Result);

    UPROPERTY()
    FString __Store__ChannelName;

UPROPERTY()
    FString __Store__ChannelCredentials;

UPROPERTY()
    EVoiceChatChannelType_ __Store__ChannelType;

UPROPERTY()
    FVoiceChatChannel3dPropertiesBP __Store__Channel3dProperties;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoiceChatUserLeaveChannelCallbackPin, FString, ChannelName, FVoiceChatResultBP, Result);

UCLASS()
class ONLINESUBSYSTEMBLUEPRINTS_API UVoiceChatUserLeaveChannel : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLeaveChannelCallbackPin OnCallFailed;
    UPROPERTY(BlueprintAssignable)
    FVoiceChatUserLeaveChannelCallbackPin OnVoiceChatChannelLeaveComplete;

    UFUNCTION(BlueprintCallable, Category = "Online|VoiceChatUser", meta = (BlueprintInternalUseOnly = "true"))
    static UVoiceChatUserLeaveChannel* LeaveChannel(
        UVoiceChatUser* Subsystem
        ,FString ChannelName
    );

    virtual void Activate() override;

private:
    UPROPERTY()
    UVoiceChatUser* __Store__Subsystem;
    void HandleCallback_OnVoiceChatChannelLeaveComplete(const FString& ChannelName, const FVoiceChatResult& Result);

    UPROPERTY()
    FString __Store__ChannelName;
};
