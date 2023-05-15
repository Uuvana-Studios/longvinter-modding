// Discord Rich Presence Plugin - (c) Jonathan Verbeek 2019

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Tickable.h"
#include "DiscordRichPresence/Public/Discord/discord.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiscordInterface.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDiscord, Log, All);

// A struct holding Discord activity information
USTRUCT(Blueprintable, BlueprintType)
struct FDiscordActivityInfo
{
	GENERATED_USTRUCT_BODY()

public:
	// The top line of the information panel (what the user is doing)
	// e.g "In-Game", "Domination", "In Menus"
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Texts")
	FString Details = "";

	// The bottom line of the information panel (the users current state)
	// e.g. "In Squad", "Playing Solo"
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Texts")
	FString State = "";

	// The large image to display (the key set at the Discord app settings)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Images")
	FString LargeImageKey = "";

	// The large image tooltip text to display
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Images")
	FString LargeImageTooltip = "";

	// The small image to display (the key set at the Discord app settings)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Images")
	FString SmallImageKey = "";

	// The small image tooltip text to display
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Images")
	FString SmallImageTooltip = "";

	// The start time to use
	// Note: always use UTC times (e.g. the UtcNow node), otherwise timezone mismatches will
	// happen.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Time")
	FDateTime StartTime;

	// The end time to use (this will automatically calculate the time left on Discord's end)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Time")
	FDateTime EndTime;

	// The current party size (when in party)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Party")
	int32 PartySize = 0;

	// The maximum party size (when in party)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Party")
	int32 PartyMaxSize = 0;

	// The party ID to set (when in party)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Party")
	FString PartyID = "";

	// The join secret. Setting this enables the "Join" button in Discord.
	// Tip: set this to a match ID (such as the Steam Lobby ID) to easily trace
	// the join request to a match. OnDiscordUserJoinRequest will be called when
	// someone tries to join your game via Discord
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Lobby")
	FString JoinSecret = "";

	// The spectate secret. Setting this enables the "Spectate" button in Discord.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Discord|Lobby")
	FString SpectateSecret = "";

	// Default constructor
	FDiscordActivityInfo()
	{
		StartTime = FDateTime(0);
		EndTime = FDateTime(0);
	}

	// Constructor taking a discord::Activity
	FDiscordActivityInfo(discord::Activity NativeActivity) 
	{
		Details = UTF8_TO_TCHAR(NativeActivity.GetDetails());
		State = UTF8_TO_TCHAR(NativeActivity.GetState());

		LargeImageKey = UTF8_TO_TCHAR(NativeActivity.GetAssets().GetLargeImage());
		LargeImageTooltip = UTF8_TO_TCHAR(NativeActivity.GetAssets().GetLargeText());
		SmallImageKey = UTF8_TO_TCHAR(NativeActivity.GetAssets().GetSmallImage());
		SmallImageTooltip = UTF8_TO_TCHAR(NativeActivity.GetAssets().GetSmallText());

		if (NativeActivity.GetTimestamps().GetStart() > 0 && NativeActivity.GetTimestamps().GetEnd())
		{
			StartTime = FDateTime::FromUnixTimestamp(NativeActivity.GetTimestamps().GetStart());
			EndTime = FDateTime::FromUnixTimestamp(NativeActivity.GetTimestamps().GetEnd());
		}

		PartySize = NativeActivity.GetParty().GetSize().GetCurrentSize();
		PartyMaxSize = NativeActivity.GetParty().GetSize().GetMaxSize();
		PartyID = UTF8_TO_TCHAR(NativeActivity.GetParty().GetId());

		JoinSecret = UTF8_TO_TCHAR(NativeActivity.GetSecrets().GetJoin());
		SpectateSecret = UTF8_TO_TCHAR(NativeActivity.GetSecrets().GetSpectate());
	}

	// Used to convert a FDiscordActivityInfo to a discord::Activity
	discord::Activity ConvertToNative()
	{
		// Compose a native discord::Activity struct out of the info we get
		discord::Activity NativeActivity{};

		// Set the type to be "Playing"
		NativeActivity.SetType(discord::ActivityType::Playing);

		// Set details string
		if (Details.Len() > 0)
		{
			NativeActivity.SetDetails(TCHAR_TO_UTF8(*Details));
		}

		// Set state string
		if (State.Len() > 0)
		{
			NativeActivity.SetState(TCHAR_TO_UTF8(*State));
		}

		// Set the large image, if specified
		if (LargeImageKey.Len() > 0)
		{
			NativeActivity.GetAssets().SetLargeImage(TCHAR_TO_UTF8(*LargeImageKey));
			NativeActivity.GetAssets().SetLargeText(TCHAR_TO_UTF8(*LargeImageTooltip));
		}

		// Set the small image, if specified
		if (SmallImageKey.Len() > 0)
		{
			NativeActivity.GetAssets().SetSmallImage(TCHAR_TO_UTF8(*SmallImageKey));
			NativeActivity.GetAssets().SetSmallText(TCHAR_TO_UTF8(*SmallImageTooltip));
		}

		// If the times aren't the same, display them on Discord
		if (StartTime != EndTime && EndTime > 0)
		{
			NativeActivity.GetTimestamps().SetStart(StartTime.ToUnixTimestamp());
			NativeActivity.GetTimestamps().SetEnd(EndTime.ToUnixTimestamp());
		}
		else if (StartTime > 0 && EndTime <= 0)
		{
			NativeActivity.GetTimestamps().SetStart(StartTime.ToUnixTimestamp());
		}

		// If party sizes were set, display them too
		if (PartySize > 0 && PartyMaxSize > 0)
		{
			NativeActivity.GetParty().GetSize().SetCurrentSize(PartySize);
			NativeActivity.GetParty().GetSize().SetMaxSize(PartyMaxSize);
		}

		// If a party ID was set, set it
		if (PartyID.Len() > 0)
		{
			NativeActivity.GetParty().SetId(TCHAR_TO_UTF8(*PartyID));
		}

		// If the join secret was set, submit it to Discord
		if (JoinSecret.Len() > 0)
		{
			NativeActivity.GetSecrets().SetJoin(TCHAR_TO_UTF8(*JoinSecret));
		}

		// If the spectate secret was set, submit it to Discord
		if (SpectateSecret.Len() > 0)
		{
			NativeActivity.GetSecrets().SetSpectate(TCHAR_TO_UTF8(*SpectateSecret));
		}

		return NativeActivity;
	}
};

// The type of an presence
UENUM(Blueprintable, BlueprintType)
enum EDiscordPresenceActivityActionType
{
	Join,
	Spectate
};

// A Discord user
USTRUCT(Blueprintable, BlueprintType)
struct FDiscordUser
{
	GENERATED_USTRUCT_BODY()

public:
	// The users Discord ID (not the tag)
	UPROPERTY(BlueprintReadOnly, Category = "Discord|User")
	FString ID = "";

	// The users Discord Username
	UPROPERTY(BlueprintReadOnly, Category = "Discord|User")
	FString Username = "";

	// The users Discord discriminator (the tag number after the hashtag)
	UPROPERTY(BlueprintReadOnly, Category = "Discord|User")
	FString Discriminator = "";

	// Indicates whether this user is a bot
	UPROPERTY(BlueprintReadOnly, Category = "Discord|User")
	bool bIsBot = false;

	// Default constructor
	FDiscordUser() {}

	// Constructor taking a discord user struct
	FDiscordUser(discord::User NativeUser)
	{
		ID = FString::Printf(TEXT("%I64d"), NativeUser.GetId());
		Username = UTF8_TO_TCHAR(NativeUser.GetUsername());
		Discriminator = UTF8_TO_TCHAR(NativeUser.GetDiscriminator());
		bIsBot = NativeUser.GetBot();
	}
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDiscordUserJoin, FString, JoinSecret);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDiscordUserSpectate, FString, SpectateSecret);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDiscordUserJoinRequest, FDiscordUser, User);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDiscordInvite, TEnumAsByte<EDiscordPresenceActivityActionType>, Type, FDiscordUser, User, FDiscordActivityInfo, Activity);

// Discord Core object used for interacting with Discord
static discord::Core* DiscordCore;

// Static delegates
static FOnDiscordUserJoin DiscordUserJoin;
static FOnDiscordUserSpectate DiscordUserSpectate;
static FOnDiscordUserJoinRequest DiscordUserJoinRequest;
static FOnDiscordInvite DiscordInvite;

// Used for all interaction with Discord
UCLASS(Blueprintable, BlueprintType)
class DISCORDRICHPRESENCE_API UDiscordInterface : public UBlueprintFunctionLibrary/*, public FTickableGameObject*/
{
	GENERATED_BODY()

public:
	// Initializes Discord. Before using the rich presence, you need to call this!
	// Returns true if Discord was initialized successfully
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static bool InitializeDiscord(FString ClientID);

	// Checks whether the Discord installation is complete (just a workaround for now!)
	static bool CheckForCompleteInstallation();

	// Shuts down Discord. You need to call this at end of your game!
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static void ShutdownDiscord();

	// Ticks the Discord callbacks, this needs to run on your game tick!
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static void TickDiscord();

	// Sets the current Discord activity status
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static void SetActivity(FDiscordActivityInfo Activity);

	// Clears the current Discord activity
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static void ClearActivity();

	// Registers this application to Discord (so invitations can launch up this game on the user's PC)
	// This will register a custom command you can specify, such as "my-awesome-game://run" or a file path
	UFUNCTION(BlueprintCallable, Category = "Discord")
	static void RegisterApplication(FString Command);

	// Registers this application to Discord (so invitations can launch up this game on the user's PC)
	// This will register the Steam App ID this game is currently running on
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Register Application (Steam)"), Category = "Discord")
	static void RegisterApplicationSteam();

	// Binds the events that can get called by Discord, you can leave pins empty if you don't need an event.
	// OnDiscordUserJoin: Fires when a user accepts a game invite OR when he was allowed to join
	// OnDiscordUserSpectate: Fires when a user accepts a spectate invite OR clicks the spectate button
	// OnDiscordUserJoinRequest: Fires when a user asks to join the current user's game
	// OnDiscordInvite: Fires when the user receives a join or spectate invite
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bind Discord Events"), Category = "Discord")
	static void BindEvents(FOnDiscordUserJoin OnDiscordUserJoin, FOnDiscordUserSpectate OnDiscordUserSpectate, FOnDiscordUserJoinRequest OnDiscordUserJoinRequest, FOnDiscordInvite OnDiscordInvite);

	// OnDiscordUserJoin: Fires when a user accepts a game invite OR when he was allowed to join
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bind Discord Event: OnDiscordUserJoin"), Category = "Discord")
	static void BindOnUserJoin(FOnDiscordUserJoin OnDiscordUserJoin);

	// OnDiscordUserSpectate: Fires when a user accepts a spectate invite OR clicks the spectate button
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bind Discord Event: OnDiscordUserSpectate"), Category = "Discord")
	static void BindOnUserSpectate(FOnDiscordUserSpectate OnDiscordUserSpectate);

	// OnDiscordUserJoinRequest: Fires when a user asks to join the current user's game
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bind Discord Event: OnDiscordUserJoinRequest"), Category = "Discord")
	static void BindOnUserJoinRequest(FOnDiscordUserJoinRequest OnDiscordUserJoinRequest);

	// OnDiscordInvite: Fires when the user receives a join or spectate invite
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Bind Discord Event: OnDiscordInvite"), Category = "Discord")
	static void BindOnUserInvite(FOnDiscordInvite OnDiscordInvite);
};
