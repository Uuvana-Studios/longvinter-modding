// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordTypes.h"
#include "Tickable.h"
#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "DiscordCore.generated.h"

class UDiscordManager;

// Raw events dispatched to managers
namespace
{
#if DISCORD_WITH_NETWORK
	DECLARE_DELEGATE_FourParams	(FDiscordNetworkOnMessage, FRawDiscord::DiscordNetworkPeerId, FRawDiscord::DiscordNetworkChannelId, uint8*, uint32);
	DECLARE_DELEGATE_OneParam	(FDiscordNetworkOnRouteUpdate,	const char*);
#endif // DISCORD_WITH_NETWORK

#if DISCORD_WITH_ACHIEVEMENT
	DECLARE_DELEGATE_OneParam	(FDiscordAchievementOnUserAchievementUpdate,	FRawDiscord::DiscordUserAchievement*);
#endif // DISCORD_WITH_ACHIEVEMENT

#if DISCORD_WITH_STORE
	DECLARE_DELEGATE_OneParam(FDiscordOnEntitlementCreate, FRawDiscord::DiscordEntitlement*);
	DECLARE_DELEGATE_OneParam(FDiscordOnEntitlementDelete, FRawDiscord::DiscordEntitlement*);
#endif // DISCORD_WITH_STORE

#if DISCORD_WITH_RELATIONSHIP
	DECLARE_DELEGATE		 (FDiscordOnRelationshipRefreshed);
	DECLARE_DELEGATE_OneParam(FDiscordOnRelationshipUpdate, FRawDiscord::DiscordRelationship*);
#endif // DISCORD_WITH_RELATIONSHIP

#if DISCORD_WITH_OVERLAY
	DECLARE_DELEGATE_OneParam(FDiscordOnOverlayToggle, const bool);
#endif // DISCORD_WITH_OVERLAY

#if DISCORD_WITH_LOBBY
	DECLARE_DELEGATE_OneParam   (FDiscordOnLobbyUpdate,			  const int64);
	DECLARE_DELEGATE_TwoParams  (FDiscordOnLobbyDelete,			  const int64, const uint32);
	DECLARE_DELEGATE_TwoParams  (FDiscordOnLobbyMemberConnect,	  const int64, const int64);
	DECLARE_DELEGATE_TwoParams  (FDiscordOnLobbyMemberUpdate,	  const int64, const int64);
	DECLARE_DELEGATE_TwoParams  (FDiscordOnLobbyMemberDisconnect, const int64, const int64);
	DECLARE_DELEGATE_FourParams (FDiscordOnLobbyMessage,		  const int64, const int64, uint8*, const uint32);
	DECLARE_DELEGATE_ThreeParams(FDiscordOnLobbySpeaking,		  const int64, const int64, const bool);
	DECLARE_DELEGATE_FiveParams (FDiscordOnLobbyNetworkMessage,	  const int64, const int64, const uint8, uint8*, const uint32);
#endif // DISCORD_WITH_LOBBY

#if DISCORD_WITH_VOICE
	DECLARE_DELEGATE(FDiscordVoiceOnSettingsUpdate);
#endif // DISCORD_WITH_VOICE

#if DISCORD_WITH_ACTIVITY
	DECLARE_DELEGATE_OneParam	(FDiscordActivityOnActivityJoin,		const char*);
	DECLARE_DELEGATE_OneParam	(FDiscordActivityOnActivitySpectate,	const char*);
	DECLARE_DELEGATE_OneParam   (FDiscordActivityOnActivityJoinRequest, FRawDiscord::DiscordUser*);
	DECLARE_DELEGATE_ThreeParams(FDiscordActivityOnActivityInvite,		FRawDiscord::EDiscordActivityActionType, FRawDiscord::DiscordUser*, FRawDiscord::DiscordActivity*);
#endif // DISCORD_WITH_ACTIVITY

#if DISCORD_WITH_USER
	DECLARE_DELEGATE(FDiscordOnUserUpdate);
#endif // DISCORD_WITH_USER
};

UENUM()
enum class EDiscordCoreCreationFlags : uint8
{
	Default = 0,
	NoRequireDiscord = 1,
};

/**
 *	Enum used to add Blueprint branching on Discord operations that might fail.
 */
UENUM()
enum class EDiscordOperationBranching : uint8
{
	Success,
	Error
};

/**
 * Discord Result wrapped into a UENUM.
 */
UENUM(BlueprintType)
enum class EDiscordResult : uint8
{
    Ok                              = 0,
    ServiceUnavailable              = 1,
    InvalidVersion                  = 2,
    LockFailed                      = 3,
    InternalError                   = 4,
    InvalidPayload                  = 5,
    InvalidCommand                  = 6,
    InvalidPermissions              = 7,
    NotFetched                      = 8,
    NotFound                        = 9,
    Conflict                        = 10,
    InvalidSecret                   = 11,
    InvalidJoinSecret               = 12,
    NoEligibleActivity              = 13,
    InvalidInvite                   = 14,
    NotAuthenticated                = 15,
    InvalidAccessToken              = 16,
    ApplicationMismatch             = 17,
    InvalidDataUrl                  = 18,
    InvalidBase64                   = 19,
    NotFiltered                     = 20,
    LobbyFull                       = 21,
    InvalidLobbySecret              = 22,
    InvalidFilename                 = 23,
    InvalidFileSize                 = 24,
    InvalidEntitlement              = 25,
    NotInstalled                    = 26,
    NotRunning                      = 27,
    InsufficientBuffer              = 28,
    PurchaseCanceled                = 29,
    InvalidGuild                    = 30,
    InvalidEvent                    = 31,
    InvalidChannel                  = 32,
    InvalidOrigin                   = 33,
    RateLimited                     = 34,
    OAuth2Error                     = 35,
    SelectChannelTimeout            = 36,
    GetGuildTimeout                 = 37,
    SelectVoiceForceRequired        = 38,
    CaptureShortcutAlreadyListening = 39,
    UnauthorizedForAchievement      = 40,
    InvalidGiftCode                 = 41,
    PurchaseError                   = 42,
    TransactionAborted              = 43,
    DrawingInitFailed               = 44,
	InvalidManager = 45
};

UENUM(BlueprintType)
enum class EDiscordLogLevel : uint8
{ 
	// For Blueprint Compatibility
	None = 0 UMETA(Hidden),

	Error = 1,
	Warn,
	Info,
	Debug,
};


UENUM(BlueprintType)
enum class EDiscordEntitlementType : uint8
{
	// Invalid. For Blueprint compatibility. Don't use it.
	None = 0 UMETA(Hidden),

	// entitlement was purchased
	Purchase = 1,
	// entitlement for a Discord Nitro subscription
	PremiumSubscription = 2,
	// entitlement was gifted by a developer
	DeveloperGift = 3,
	// entitlement was purchased by a dev in application test mode
	TestModePurchase = 4,
	// entitlement was granted when the SKU was free
	FreePurchase = 5,
	// entitlement was gifted by another user
	UserGift = 6,
	// entitlement was claimed by user for free as a Nitro Subscriber
	PremiumPurchase = 7
};

UENUM(BlueprintType)
enum class EDiscordActivityActionType : uint8
{
	None UMETA(DisplayName = "Invalid", Tooltip = "It exists only for UBT. Don't use it", Hidden),
	Join = 1,
	Spectate,
};

DECLARE_DELEGATE_OneParam(FDiscordResultCallback, EDiscordResult);

namespace FDiscordResult
{
	FORCEINLINE EDiscordResult ToEDiscordResult(const FRawDiscord::EDiscordResult Result)
	{
		return static_cast<EDiscordResult>(Result);
	}

	DISCORDCORE_API const TCHAR* DiscordResultToString(const EDiscordResult Result);
	DISCORDCORE_API const TCHAR* DiscordResultToString(const FRawDiscord::EDiscordResult Result);
};


USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordPartySize
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	int32 CurrentSize = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	int32 MaxSize = 0;
};


/**
 *	Wrap an uint64 for Blueprints.
 */
USTRUCT(BlueprintType)
struct DISCORDCORE_API FUint64
{
	GENERATED_BODY()
public:
	FUint64() : Value(0) {};
	FUint64(const uint64 Val) : Value(Val) {};

	FORCEINLINE	uint64 Get() const { return Value; };
	FORCEINLINE void   Set(const uint64& Val) { Value = Val;  };
	
	FORCEINLINE operator uint64()		{ return Value; }
	FORCEINLINE operator uint64() const { return Value; }

private:
	uint64 Value;
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordUser
{
	GENERATED_BODY()
private:
	friend class UDiscordActivityManager;
	friend class UDiscordRelationshipManager;
	friend class UDiscordLobbyManager;
	friend class UDiscordUserManager;
	friend struct FDiscordRelationship;

	FDiscordUser(struct FRawDiscord::DiscordUser* Raw);

public:
	FDiscordUser() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	int64 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	FString Username;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	FString Discriminator;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	FString Avatar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	bool bBot = false;
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordEntitlement
{
	GENERATED_BODY()
private:
	friend class UDiscordStoreManager;
	FDiscordEntitlement(struct FRawDiscord::DiscordEntitlement* Raw);
public:
	FDiscordEntitlement() {};

	// The unique ID of the entitlement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	int64 Id = 0;
	// The kind of entitlement it is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	EDiscordEntitlementType Type = EDiscordEntitlementType::None;
	// The ID of the SKU to which the user is entitled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord")
	int64 SkuId = 0;
};


UENUM()
enum class EDiscordActivityType : uint8
{
    Playing,
    Streaming,
    Listening,
    Watching
};

UENUM()
enum class EDiscordActivityJoinRequestReply : uint8
{
    No,
    Yes,
    Ignore,
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordActivityTimestamps
{
    GENERATED_BODY()
public:
    // Unix timestamp - send this to have an "elapsed" timer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    int64 Start = 0;
    // Unix timestamp - send this to have a "remaining" timer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    int64 End = 0;
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordActivitySecrets
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString Match;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString Join;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString Spectate;
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordActivityParty
{
    GENERATED_BODY()
public:
	FDiscordActivityParty() = default;
	FDiscordActivityParty(const FDiscordActivityParty&);
	FDiscordActivityParty(FDiscordActivityParty&&);

	FDiscordActivityParty& operator=(const FDiscordActivityParty&);
	FDiscordActivityParty& operator=(FDiscordActivityParty&&);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString Id;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FDiscordPartySize Size;
};

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordActivityAssets
{
    GENERATED_BODY()
public:
	FDiscordActivityAssets() = default;
	FDiscordActivityAssets(const FDiscordActivityAssets&);
	FDiscordActivityAssets(FDiscordActivityAssets&&);

	FDiscordActivityAssets& operator=(const FDiscordActivityAssets&);
	FDiscordActivityAssets& operator=(FDiscordActivityAssets&&);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString LargeImage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString LargeText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString SmallImage;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString SmallText;
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FDiscordLogEvent, EDiscordLogLevel, DiscordLogLevel, const FString&, Message);

USTRUCT(BlueprintType)
struct DISCORDCORE_API FDiscordActivity
{
    GENERATED_BODY()
private:
    friend class UDiscordActivityManager;
    friend class UDiscordActivityManager;
	friend struct FDiscordPresence;
    FDiscordActivity(struct FRawDiscord::DiscordActivity* Raw);

public:
    FDiscordActivity() = default;
    FDiscordActivity(const FDiscordActivity&);
    FDiscordActivity(FDiscordActivity&&);

	FDiscordActivity& operator=(const FDiscordActivity&);
	FDiscordActivity& operator=(FDiscordActivity&&);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    EDiscordActivityType Type = EDiscordActivityType::Playing;
    // Your application id - this is a read-only field
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Discord")
    int64 ApplicationId = 0;
    // Name of the application - this is a read-only field
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Discord")
    FString Name;
    // The player's current party status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString State;
    //What the player is currently doing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FString Details;
    // Helps create elapsed/remaining timestamps on a player's profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FDiscordActivityTimestamps  Timestamps;
    // Assets to display on the player's profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FDiscordActivityAssets   Assets;
    // Information about the player's party
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FDiscordActivityParty    Party;
    // Secret passwords for joining and spectating the player's game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    FDiscordActivitySecrets  Secrets;
    // Whether this activity is an instanced context, like a match
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Discord")
    bool bInstance = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRunCallbacks, EDiscordResult, Error);

/**
 *  A Discord Core object.
 **/
UCLASS(BlueprintType)
class DISCORDCORE_API UDiscordCore : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
private:
	friend struct FDiscordCoreEventsDispatcher;
	
	friend class UDiscordNetworkManager;
	friend class UDiscordAchievementManager;
	friend class UDiscordActivityManager;
	friend class UDiscordApplicationManager;
	friend class UDiscordVoiceManager;
	friend class UDiscordImageManager;
	friend class UDiscordLobbyManager;
	friend class UDiscordOverlayManager;
	friend class UDiscordRelationshipManager;
	friend class UDiscordStoreManager;
	friend class UDiscordStorageManager;
	friend class UDiscordUserManager;

	friend void DiscordLogCallback(void* const Data, FRawDiscord::EDiscordLogLevel Level, const char* message);

private:
	struct FDiscordApplication 
	{
		FRawDiscord::IDiscordCore*					core			= nullptr;
		FRawDiscord::IDiscordUserManager*			users			= nullptr;
		FRawDiscord::IDiscordAchievementManager*	achievements	= nullptr;
		FRawDiscord::IDiscordActivityManager*		activities		= nullptr;
		FRawDiscord::IDiscordRelationshipManager*	relationships	= nullptr;
		FRawDiscord::IDiscordApplicationManager*	application		= nullptr;
		FRawDiscord::IDiscordLobbyManager*			lobbies			= nullptr;
		FRawDiscord::IDiscordNetworkManager*		network			= nullptr;
		FRawDiscord::IDiscordVoiceManager*			voice			= nullptr;
		FRawDiscord::IDiscordImageManager*			image			= nullptr;
		FRawDiscord::IDiscordStorageManager*		storage			= nullptr;
		FRawDiscord::IDiscordStoreManager*			store			= nullptr;
		FRawDiscord::IDiscordOverlayManager*		overlay			= nullptr;

		uint64 user_id = 0;
	};

	// Begin FTickableGameObject
	virtual TStatId GetStatId() const;
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const override;
	// End FTickableGameObject

private:

	/**
	 * Call CreateDiscordCore() instead.
	*/
	UDiscordCore();

public:
	~UDiscordCore();

	/**
	 * Creates a new Discord Core.
	 * @return A new Discord Core.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Core")
	static UPARAM(DisplayName = "New Core") UDiscordCore* CreateDiscordCore(const EDiscordCoreCreationFlags CreationFlags);

	/**
	 * Converts this uint64 number to string.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Misc", BlueprintPure)
	static UPARAM(DisplayName = "Uint64 String") FString Uint64ToString(const FUint64& Value) { return FString::Printf(TEXT("%lld"), Value.Get()); }

	/**
	 * Create an Uint64 from a string.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Misc", BlueprintPure)
	static UPARAM(DisplayName = "Uint64 Value") FUint64 StringToUint64(const FString& Value) { return FCString::Strtoui64(*Value, nullptr, 10); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Core", DisplayName = "Get Application ID")
	FUint64 Blueprint_GetApplicationId() { return GetApplicationId(); }
	uint64 GetApplicationId();

	UFUNCTION(BlueprintCallable, Category = "Discord|Misc", BlueprintPure)
	static int64 GetTimeSinceEpoch();

	UFUNCTION(BlueprintCallable, Category = "Discord|Core")
	void SetLogHook(const FDiscordLogEvent& OnLogCallback);

	/**
	 * Called when an error occured during running the core's callback.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnRunCallbacks OnRunCallbacks;

private:
	// We don't want to run initialization in the constructor with the editor or while packaging.
	bool InitCore(const EDiscordCoreCreationFlags CreationFlags);

	void OnCurrentUserUpdated();

	FDiscordLogEvent LogHookEvent;

private:
	/* Delegates to dispatch the events to each manager */
#if DISCORD_WITH_NETWORK
	FDiscordNetworkOnMessage					NetworkOnMessage;
	FDiscordNetworkOnRouteUpdate				NetworkOnRouteUpdate;
#endif // DISCORD_WITH_NETWORK

#if DISCORD_WITH_ACHIEVEMENT
	FDiscordAchievementOnUserAchievementUpdate	AchievementOnUserAchivementUpdate;
#endif // DISCORD_WITH_ACHIEVEMENTS

#if DISCORD_WITH_ACTIVITY
	FDiscordActivityOnActivityJoin		  ActivityOnJoin;	
	FDiscordActivityOnActivitySpectate	  ActivityOnSpectate;
	FDiscordActivityOnActivityJoinRequest ActivityOnJoinRequest;
	FDiscordActivityOnActivityInvite	  ActivityOnInvite;
#endif // DISCORD_WITH_ACTIVITY

#if DISCORD_WITH_LOBBY
	FDiscordOnLobbyUpdate			LobbyOnUpdate;
	FDiscordOnLobbyDelete			LobbyOnDelete;
	FDiscordOnLobbyMemberConnect	LobbyOnMemberConnect;
	FDiscordOnLobbyMemberUpdate		LobbyOnMemberUpdate;
	FDiscordOnLobbyMemberDisconnect LobbyOnMemberDisconnect;
	FDiscordOnLobbyMessage			LobbyOnMessage;
	FDiscordOnLobbySpeaking			LobbyOnSpeaking;
	FDiscordOnLobbyNetworkMessage   LobbyOnNetworkMessage;
#endif // DISCORD_WITH_LOBBY

#if DISCORD_WITH_VOICE
	FDiscordVoiceOnSettingsUpdate VoiceOnSettingsUpdate;
#endif // DISCORD_WITH_VOICE

#if DISCORD_WITH_OVERLAY
	FDiscordOnOverlayToggle	OverlayOnToggle;
#endif // DISCORD_WITH_OVERLAY

#if DISCORD_WITH_RELATIONSHIP
	FDiscordOnRelationshipRefreshed RelationshipOnRefreshed;
	FDiscordOnRelationshipUpdate	RelationshipOnUpdate;
#endif // DISCORD_WITH_RELATIONSHIP

#if DISCORD_WITH_STORE
	FDiscordOnEntitlementCreate EntitlementOnCreate;
	FDiscordOnEntitlementDelete EntitlementOnDelete;
#endif // DISCORD_WITH_STORE

#if DISCORD_WITH_USER
	FDiscordOnUserUpdate UserOnUpdate;
#endif // DISCORD_WITH_USER

private:
	FDiscordApplication						App;
	FRawDiscord::IDiscordUserEvents			UserEvents;
	FRawDiscord::IDiscordActivityEvents		ActivitiesEvents;
	FRawDiscord::IDiscordRelationshipEvents	RelationshipsEvents;
	FRawDiscord::IDiscordAchievementEvents	AchievementEvents;
	FRawDiscord::IDiscordApplicationEvents	ApplicationEvents;
	FRawDiscord::IDiscordImageEvents		ImageEvents;
	FRawDiscord::IDiscordLobbyEvents		LobbyEvents;
	FRawDiscord::IDiscordNetworkEvents		NetworkEvents;
	FRawDiscord::IDiscordOverlayEvents		OverlayEvents;
	FRawDiscord::IDiscordStorageEvents		StorageEvents;
	FRawDiscord::IDiscordStoreEvents		StoreEvents;
	FRawDiscord::IDiscordVoiceEvents		VoiceEvents;

private:
	UPROPERTY() UDiscordManager* NetworkManager;
	UPROPERTY() UDiscordManager* AchievementManager;
	UPROPERTY() UDiscordManager* ActivityManager;
	UPROPERTY() UDiscordManager* ApplicationManager;
	UPROPERTY() UDiscordManager* VoiceManager;
	UPROPERTY() UDiscordManager* ImageManager;
	UPROPERTY() UDiscordManager* LobbyManager;
	UPROPERTY() UDiscordManager* OverlayManager;
	UPROPERTY() UDiscordManager* RelationshipManager;
	UPROPERTY() UDiscordManager* StorageManager;
	UPROPERTY() UDiscordManager* StoreManager;
	UPROPERTY() UDiscordManager* UserManager;

};

