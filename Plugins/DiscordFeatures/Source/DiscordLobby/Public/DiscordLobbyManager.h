// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordLobbyManager.generated.h"

UENUM(BlueprintType)
enum class EDiscordLobbyType : uint8
{
	// For Blueprints compatibility, don't use it
	None UMETA(Hidden),

	Private = 1,
	Public
};

UENUM(BlueprintType)
enum class EDiscordLobbySearchComparison : uint8
{
	LessThanOrEqual		= 0,
	LessThan			= 1,
	Equal				= 2,
	GraterThan			= 3,
	GreaterOrEqualThan	= 4,
	NotEqual			= 5
};

UENUM(BlueprintType)
enum class EDiscordLobbySearchCast : uint8
{
	// For Blueprints compatibility, don't use it
	None UMETA(Hidden),

	String = 1,
	Number
};

UENUM(BlueprintType)
enum class EDiscordLobbySearchDistance : uint8
{
	// Within the same region
	Local = 0,
	// Within the same and adjacent regions
	Default,
	// Far distances, like US to EU
	Extended,
	// All regions
	Global
};

USTRUCT(BlueprintType)
struct DISCORDLOBBY_API FDiscordLobby
{
	GENERATED_BODY()
private:
	friend class UDiscordLobbyManager;
	friend class FLobbyCallbackFunctions;
	FDiscordLobby(FRawDiscord::DiscordLobby* Raw);

public:
	FDiscordLobby() = default;

	// The unique id of the lobby
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	int64 Id = 0;
	// If the lobby is public or private
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	EDiscordLobbyType Type = EDiscordLobbyType::Private;
	// The userId of the lobby owner
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	int64 OwnerId = 0;
	// The password to the lobby
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	FString Secret;
	// The max capacity of the lobby
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	int64 Capacity = 0;
	// Whether or not the lobby can be joined
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Lobby")
	bool bLocked = false;
};

USTRUCT(BlueprintType)
struct DISCORDLOBBY_API FDiscordLobbyTransaction
{
	GENERATED_BODY()
private:
	friend class UDiscordLobbyManager;
	friend class FLobbyCallbackFunctions;
	FDiscordLobbyTransaction(FRawDiscord::IDiscordLobbyTransaction* const InRaw) : Raw(InRaw) {};

public:
	FDiscordLobbyTransaction() : Raw(nullptr) {};
	FDiscordLobbyTransaction(const FDiscordLobbyTransaction& Other) : Raw(Other.Raw) {};

	FDiscordLobbyTransaction& operator=(const FDiscordLobbyTransaction& Other)
	{
		Raw = Other.Raw;
		return *this;
	}

	void SetType(const EDiscordLobbyType Type);
	void SetOwner(const int64 UserId);
	void SetCapacity(const uint32 Capacity);
	void SetMetadata(const FString& Key, const FString& Value);
	void DeleteMetadata(const FString& Key);
	void SetLocked(const bool bLocked);

private:
	FRawDiscord::IDiscordLobbyTransaction* Raw;
};

USTRUCT(BlueprintType)
struct DISCORDLOBBY_API FDiscordLobbyMemberTransaction
{
	GENERATED_BODY()
private:
	friend class UDiscordLobbyManager;
	FDiscordLobbyMemberTransaction(FRawDiscord::IDiscordLobbyMemberTransaction* const InRaw) : Raw(InRaw) {}
public:
	FDiscordLobbyMemberTransaction() : Raw(nullptr) {}
	FDiscordLobbyMemberTransaction(const FDiscordLobbyMemberTransaction& Other) : Raw(Other.Raw) {};

	FDiscordLobbyMemberTransaction& operator=(const FDiscordLobbyMemberTransaction& Other)
	{
		Raw = Other.Raw;
		return *this;
	}

	void SetMetadata(const FString& Key, const FString& Value);
	void DeleteMetadata(const FString& Key);
	
private:
	FRawDiscord::IDiscordLobbyMemberTransaction* Raw;
};

USTRUCT(BlueprintType)
struct DISCORDLOBBY_API FDiscordLobbySearchQuery
{
	GENERATED_BODY()
private:
	friend class UDiscordLobbyManager;
	FDiscordLobbySearchQuery(FRawDiscord::IDiscordLobbySearchQuery* const InRaw) : Raw(InRaw) {}
public:
	FDiscordLobbySearchQuery() : Raw(nullptr) {}
	FDiscordLobbySearchQuery(const FDiscordLobbySearchQuery& Other) : Raw(Other.Raw) {};

	FDiscordLobbySearchQuery& operator=(const FDiscordLobbySearchQuery& Other)
	{
		Raw = Other.Raw;
		return *this;
	}

	void Filter(const FString& Key, const EDiscordLobbySearchComparison Comparison, const EDiscordLobbySearchCast Cast, const FString& Value);
	void Sort(const FString& Key, const EDiscordLobbySearchCast Cast, const FString& Value);
	void Limit(const uint32 NewLimit);
	void Distance(const EDiscordLobbySearchDistance Distance);

private:
	FRawDiscord::IDiscordLobbySearchQuery* Raw;
};


DECLARE_DELEGATE_TwoParams(FLobbyCallback, EDiscordResult, FDiscordLobby&);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam   (FDiscordLobbyUpdateEvent,		 const int64, LobbyId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams  (FDiscordLobbyDeleteEvent,		 const int64, LobbyId, const int64, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams  (FDiscordLobbyMemberEvent,		 const int64, LobbyId, const int64, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDiscordLobbyMessageEvent,		 const int64, LobbyId, const int64, UserId, const TArray<uint8>&, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDiscordLobbySpeakingEvent,		 const int64, LobbyId, const int64, UserId, const bool, bSpeaking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams (FDiscordLobbyNetworkMessageEvent, const int64, LobbyId, const int64, UserId, const uint8, ChannelId, const TArray<uint8>&, Data);


UCLASS()
class DISCORDLOBBY_API UDiscordLobbyManager : public UDiscordManager
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyUpdateEvent OnLobbyUpdate;
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyDeleteEvent OnLobbyDelete;
	
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyMemberEvent OnMemberConnect;
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyMemberEvent OnMemberUpdate;
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyMemberEvent OnMemberDisconnect;

	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyMessageEvent OnLobbyMessage;
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbySpeakingEvent OnSpeaking;
	
	UPROPERTY(BlueprintAssignable)
	FDiscordLobbyNetworkMessageEvent OnNetworkMessage;

public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby", BlueprintPure, meta = (CompactNodeTitle = "LOBBY MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Lobby Manager") UDiscordLobbyManager* GetLobbyManager(UDiscordCore* DiscordCore);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction GetLobbyCreateTransaction();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Transaction") FDiscordLobbyTransaction GetLobbyUpdateTransaction(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Transaction") FDiscordLobbyMemberTransaction GetMemberUpdateTransaction(const int64 LobbyId, const int64 MemberId);

	void CreateLobby(const FDiscordLobbyTransaction& Transaction, const FLobbyCallback& Callback);

	void UpdateLobby(const int64 LobbyId, const FDiscordLobbyTransaction& Transaction, const FDiscordResultCallback& Callback = FDiscordResultCallback());

	void DeleteLobby(const int64 LobbyId, const FDiscordResultCallback& Callback = FDiscordResultCallback());

	void ConnectLobby(const int64 LobbyId, const FString& LobbySecret, const FLobbyCallback& Callback);

	void ConnectLobbyWithActivitySecret(const FString& ActivitySecret, const FLobbyCallback& Callback);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Secret") FString GetLobbyActivitySecret(const int64 LobbyId);

	void DisconnectLobby(const int64 LobbyId, const FDiscordResultCallback& Callback = FDiscordResultCallback());

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Lobby") FDiscordLobby GetLobby(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Count") int32 LobbyMetadataCount(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Key") FString GetLobbyMetadataKey(const int64 LobbyId, const int32 Index);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Value") FString GetLobbyMetadataValue(const int64 LobbyId, const FString& Key);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Count") int32 MemberCount(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(Displayname = "User Id") int64 GetMemberUserId(const int64 LobbyId, const int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "User") FDiscordUser GetMemberUser(const int64 LobbyId, const int64 UserId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Count") int32 MemberMetadataCount(const int64 LobbyId, const int64 UserId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Key") FString GetMemberMetadataKey(const int64 LobbyId, const int64 UserId, const int32 Index);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Value") FString GetMemberMetadataValue(const int64 LobbyId, const int64 UserId, const FString& Key);

	void UpdateMember(const int64 LobbyId, const int64 UserId, const FDiscordLobbyMemberTransaction& Transaction, const FDiscordResultCallback& Callback = FDiscordResultCallback());

	void SendLobbyMessage(const int64 LobbyId, TArray<uint8>& Data, const FDiscordResultCallback& Callback = FDiscordResultCallback());

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Search Query") FDiscordLobbySearchQuery GetSearchQuery();

	void Search(const FDiscordLobbySearchQuery& Search, const FDiscordResultCallback& Callback);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Count") int32 LobbyCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Lobby")
	UPARAM(DisplayName = "Id")  int64 GetLobbyId(const int32 Index);

	void ConnectVoice(const int64 LobbyId, const FDiscordResultCallback& Callback);

	void DisconnectVoice(const int64 LobbyId, const FDiscordResultCallback& Callback);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby")
	void ConnectNetwork(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby")
	void DisconnectNetwork(const int64 LobbyId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby")
	void FlushNetwork();

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby")
	void OpenNetworkChannel(const int64 LobbyId, const uint8 ChannelId, const bool bReliable);

	UFUNCTION(BlueprintCallable, Category = "Discord|Lobby")
	void SendNetworkMessage(const int64 LobbyId, const int64 UserId, const uint8 ChannelId, UPARAM(ref) TArray<uint8>& Data);

private:
	void Internal_OnLobbyUpdate			  (const int64 LobbyId);
	void Internal_OnLobbyDelete			  (const int64 LobbyId, const uint32 Reason);
	void Internal_OnLobbyMemberConnect	  (const int64 LobbyId, const int64 UserId);
	void Internal_OnLobbyMemberUpdate	  (const int64 LobbyId, const int64 UserId);
	void Internal_OnLobbyMemberDisconnect (const int64 LobbyId, const int64 UserId);
	void Internal_OnLobbyMessage		  (const int64 LobbyId, const int64 UserId, uint8* Data, const uint32 Length);
	void Internal_OnLobbySpeaking		  (const int64 LobbyId, const int64 UserId, const bool bSpeaking);
	void Internal_OnLobbyNetworkMessage	  (const int64 LobbyId, const int64 UserId, const uint8 Channel, uint8* Data, const uint32 Length);

};
