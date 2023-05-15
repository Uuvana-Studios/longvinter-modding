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
#include "Interfaces/OnlineTournamentInterface.h"

#include "OnlineTournamentSubsystem.generated.h"

UCLASS(BlueprintType)
class ONLINESUBSYSTEMBLUEPRINTS_API UOnlineTournamentSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

    friend class UOnlineHelpers;

private:
    struct FWorldContext* WorldContext;
    TWeakPtr<class IOnlineTournament, ESPMode::ThreadSafe> HandleWk;
    TSharedPtr<class IOnlineTournament, ESPMode::ThreadSafe> GetHandle();
    bool IsHandleValid(const TSharedPtr<class IOnlineTournament, ESPMode::ThreadSafe>& InHandle);

public:
    // When async functions use global events as callbacks (such as in identity and session),
    // we need to prevent multiple requests running at the same time where those results would
    // have the same identifying data in the callback (because then we don't know which result
    // belongs to which request). For these types of async functions, they use this set to make
    // sure that only one unique request runs at a time.
    TSet<FString> __InFlightRequests;

    UOnlineTournamentSubsystem();
    virtual void BeginDestroy() override;

    UFUNCTION(BlueprintPure, Category = "Online|Tournament")
    bool IsSubsystemAvailable();

    // @generator-incompatible
    // void QueryTournamentList(const FUniqueNetIdRef UserId, const FOnlineTournamentQueryFilter& QueryFilter, const FOnlineTournamentQueryTournamentListComplete& Delegate);

    // @generator-incompatible
    // TArray<FOnlineTournamentIdRef> GetTournamentList(const FUniqueNetIdRef UserId);

    // @generator-incompatible
    // void QueryTournamentDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentIdRef>& TournamentIds, const FOnlineTournamentQueryTournamentDetailsComplete& Delegate);

    // @generator-incompatible
    // TSharedPtr<const IOnlineTournamentDetails> GetTournamentDetails(const FUniqueNetIdRef UserId, const FOnlineTournamentIdRef TournamentId);

    // @generator-incompatible
    // TArray<TSharedPtr<const IOnlineTournamentDetails>> GetTournamentDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentIdRef>& TournamentIds);

    // @generator-incompatible
    // void QueryMatchList(const FUniqueNetIdRef UserId, const FOnlineTournamentIdRef TournamentId, const FOnlineTournamentQueryMatchListComplete& Delegate);

    // @generator-incompatible
    // TArray<FOnlineTournamentMatchIdRef> GetMatchList(const FUniqueNetIdRef UserId, const FOnlineTournamentIdRef TournamentId);

    // @generator-incompatible
    // void QueryMatchDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentMatchIdRef>& MatchIds, const FOnlineTournamentQueryMatchDetailsComplete& Delegate);

    // @generator-incompatible
    // TSharedPtr<const IOnlineTournamentMatchDetails> GetMatchDetails(const FUniqueNetIdRef UserId, const FOnlineTournamentMatchIdRef MatchId);

    // @generator-incompatible
    // TArray<TSharedPtr<const IOnlineTournamentMatchDetails>> GetMatchDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentMatchIdRef>& MatchIds);

    // @generator-incompatible
    // void QueryParticipantList(const FUniqueNetIdRef UserId, const FOnlineTournamentIdRef TournamentId, const FOnlineTournamentParticipantQueryFilter& QueryFilter, const FOnlineTournamentQueryParticipantListComplete& Delegate);

    // @generator-incompatible
    // TArray<TSharedRef<const IOnlineTournamentParticipantDetails>> GetParticipantList(const FUniqueNetIdRef UserId, const FOnlineTournamentIdRef TournamentId, const EOnlineTournamentParticipantType ParticipantType);

    // @generator-incompatible
    // void QueryTeamDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentTeamIdRef>& TeamIds, const FOnlineTournamentQueryTeamDetailsComplete& Delegate);

    // @generator-incompatible
    // TSharedPtr<const IOnlineTournamentTeamDetails> GetTeamDetails(const FUniqueNetIdRef UserId, const FOnlineTournamentTeamIdRef TeamId);

    // @generator-incompatible
    // TArray<TSharedPtr<const IOnlineTournamentTeamDetails>> GetTeamDetails(const FUniqueNetIdRef UserId, const TArray<FOnlineTournamentTeamIdRef>& TeamIds);

    // @generator-incompatible
    // void SubmitMatchResults(const FUniqueNetIdRef UserId, const FOnlineTournamentMatchIdRef MatchId, const FOnlineTournamentMatchResults& MatchResults, const FOnlineTournamentSubmitMatchResultsComplete& Delegate);

    // @generator-incompatible
    // FDelegateHandle AddOnOnlineTournamentTournamentJoined(const FOnOnlineTournamentTournamentJoinedDelegate& Delegate);

    // @generator-incompatible
    // void RemoveOnOnlineTournamentTournamentJoined(const FDelegateHandle& DelegateHandle);

    // @generator-incompatible
    // FDelegateHandle AddOnOnlineTournamentMatchJoinedDelegate(const FOnOnlineTournamentMatchJoinedDelegate& Delegate);

    // @generator-incompatible
    // void RemoveOnOnlineTournamentMatchJoinedDelegate(const FDelegateHandle& DelegateHandle);
};
