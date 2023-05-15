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
#include "Interfaces/OnlineChatInterface.h"

#include "OnlineChatSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FChat_ChatRoomCreated_BP, FUniqueNetIdRepl, Param1, FString, Param2, bool, Param3, FString, Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FChat_ChatRoomConfigured_BP, FUniqueNetIdRepl, Param1, FString, Param2, bool, Param3, FString, Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FChat_ChatRoomJoinPublic_BP, FUniqueNetIdRepl, Param1, FString, Param2, bool, Param3, FString, Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FChat_ChatRoomJoinPrivate_BP, FUniqueNetIdRepl, Param1, FString, Param2, bool, Param3, FString, Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FChat_ChatRoomExit_BP, FUniqueNetIdRepl, Param1, FString, Param2, bool, Param3, FString, Param4);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChat_ChatRoomMemberJoin_BP, FUniqueNetIdRepl, Param1, FString, Param2, FUniqueNetIdRepl, Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChat_ChatRoomMemberExit_BP, FUniqueNetIdRepl, Param1, FString, Param2, FUniqueNetIdRepl, Param3);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChat_ChatRoomMemberUpdate_BP, FUniqueNetIdRepl, Param1, FString, Param2, FUniqueNetIdRepl, Param3);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChat_ChatRoomMessageReceived_BP, const FUniqueNetId&, Param1, const FChatRoomId&, Param2, const TSharedRef<FChatMessage>&, Param3);

// @parameters-not-compatible
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChat_ChatPrivateMessageReceived_BP, const FUniqueNetId&, Param1, const TSharedRef<FChatMessage>&, Param2);

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineChatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineChat, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineChat, ESPMode::ThreadSafe>& InHandle);
    FDelegateHandle DelegateHandle_OnChatRoomCreated;
    FDelegateHandle DelegateHandle_OnChatRoomConfigured;
    FDelegateHandle DelegateHandle_OnChatRoomJoinPublic;
    FDelegateHandle DelegateHandle_OnChatRoomJoinPrivate;
    FDelegateHandle DelegateHandle_OnChatRoomExit;
    FDelegateHandle DelegateHandle_OnChatRoomMemberJoin;
    FDelegateHandle DelegateHandle_OnChatRoomMemberExit;
    FDelegateHandle DelegateHandle_OnChatRoomMemberUpdate;

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineChatSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Chat")
    bool IsSubsystemAvailable();

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomCreated_BP OnChatRoomCreated;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomConfigured_BP OnChatRoomConfigured;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomJoinPublic_BP OnChatRoomJoinPublic;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomJoinPrivate_BP OnChatRoomJoinPrivate;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomExit_BP OnChatRoomExit;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberJoin_BP OnChatRoomMemberJoin;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberExit_BP OnChatRoomMemberExit;

    UPROPERTY(BlueprintAssignable, Category = "Online|Chat")
    FChat_ChatRoomMemberUpdate_BP OnChatRoomMemberUpdate;

    // @generator-incompatible
    // bool CreateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig);

    // @generator-incompatible
    // bool ConfigureRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FChatRoomConfig& ChatRoomConfig);

    // @generator-incompatible
    // bool JoinPublicRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig);

    // @generator-incompatible
    // bool JoinPrivateRoom(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FString& Nickname, const FChatRoomConfig& ChatRoomConfig);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool ExitRoom(FUniqueNetIdRepl UserId, FString RoomId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool SendRoomChat(FUniqueNetIdRepl UserId, FString RoomId, FString MsgBody);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    bool SendPrivateChat(FUniqueNetIdRepl UserId, FUniqueNetIdRepl RecipientId, FString MsgBody);

    UFUNCTION(BlueprintPure, Category = "Online|Chat")
    bool IsChatAllowed(FUniqueNetIdRepl UserId, FUniqueNetIdRepl RecipientId);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    void GetJoinedRooms(FUniqueNetIdRepl UserId, TArray<FString>& OutRooms);

    // @generator-incompatible
    // TSharedPtr<FChatRoomInfo> GetRoomInfo(const FUniqueNetId& UserId, const FChatRoomId& RoomId);

    // @generator-incompatible
    // bool GetMembers(const FUniqueNetId& UserId, const FChatRoomId& RoomId, TArray<TSharedRef<FChatRoomMember>>& OutMembers);

    // @generator-incompatible
    // TSharedPtr<FChatRoomMember> GetMember(const FUniqueNetId& UserId, const FChatRoomId& RoomId, const FUniqueNetId& MemberId);

    // @generator-incompatible
    // bool GetLastMessages(const FUniqueNetId& UserId, const FChatRoomId& RoomId, int32 NumMessages, TArray<TSharedRef<FChatMessage>>& OutMessages);

    // @generator-incompatible
    // bool IsMessageFromLocalUser(const FUniqueNetId& UserId, const FChatMessage& Message, const bool bIncludeExternalInstances);

    UFUNCTION(BlueprintCallable, Category = "Online|Chat")
    void DumpChatState();
};
