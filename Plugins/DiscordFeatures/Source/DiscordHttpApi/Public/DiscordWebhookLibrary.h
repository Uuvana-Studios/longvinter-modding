// Copy

#pragma once

#include "CoreMinimal.h"
#include "DiscordWebhookLibrary.generated.h"

UENUM(BlueprintType)
enum class EDiscordWebhookType : uint8
{
	None			= 0 UMETA(Hidden),
	// Incoming Webhooks can post messages to channels with a generated token
	Incoming		= 1,
	// Channel Follower Webhooks are internal webhooks used with Channel Following to post new messages into channels
	ChannelFollower = 2
};

/**
 *	A Discord User's flag.
 *  Note that values aren't the same as official SDK due to UBT requiring uint8 enums.
 *  Value is converted internaly when required.
 **/
UENUM(BlueprintType)
enum class EDiscordWebhookUserFlag : uint8
{
	None						= 0,
	DiscordEmployee				= 1,
	PartneredServerOwner		= 2,
	HypeSquadEvents				= 3,
	BugHunterLevel1				= 4,
	HouseBravery				= 5,
	HouseBrilliance				= 6,
	HouseBalance				= 7,
	EarlySupporter				= 8,
	TeamUser					= 9,
	System						= 10,
	BugHunterLevel2				= 11,
	VerifiedBot					= 12,
	EarlyVerifiedBotDeveloper	= 13
};

UENUM(BlueprintType)
enum class EDiscordAllowedMentionTypes : uint8
{
	RoleMentions,
	UserMentions,
	EveryoneMentions
};

UENUM(BlueprintType)
enum class EDiscordWebhookPremiumType : uint8
{
	None = 0,
	NitroClassic = 1,
	Nitro = 2
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookUser
{
	GENERATED_BODY()
public:
	// the user's id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int64 Id;

	// the user's username, not unique across the platform
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Username;

	// the user's 4-digit discord-tag
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Discriminator;
	
	// the user's avatar hash
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Avatar;
	
	// whether the user belongs to an OAuth2 application
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bBot;
	
	// whether the user is an Official Discord System user (part of the urgent message system)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bSystem;
	
	// 	whether the user has two factor enabled on their account
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bMfaEnabled;
	
	// 	the user's chosen language option
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Locale;
	
	// 	whether the email on this account has been verified
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bVerified;
	
	// the user's email
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Email;	

	// 	the flags on a user's account
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<EDiscordWebhookUserFlag> Flags;

	// 	the type of Nitro subscription on a user's account
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	EDiscordWebhookPremiumType PremiumType;

	// the public flags on a user's account
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<EDiscordWebhookUserFlag> PublicFlags;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhook
{
	GENERATED_BODY()
public:
	// the id of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int64 Id;

	// 	the type of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	EDiscordWebhookType Type;

	//	the guild id this webhook is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int64 GuildId;

	// 	the channel id this webhook is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int64 ChannelId;

	// 	the user this webhook was created by (not returned when getting a webhook with its token)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookUser User;

	// 	the default name of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Name;

	// 	the default avatar of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Avatar;

	// 	the secure token of the webhook (returned for Incoming Webhooks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Token;
};


USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedThumbnail
{
	GENERATED_BODY()
public:
	// source url of thumbnail (only supports http(s) and attachments)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;

	// a proxied url of the thumbnail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString ProxyUrl;
	
	// height of thumbnail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Height;
	
	// width of thumbnail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Width;

};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedImage
{
	GENERATED_BODY()
public:
	// source url of image (only supports http(s) and attachments)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;

	// a proxied url of the image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString ProxyUrl;
	
	// height of image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Height;
	
	// width of image
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Width;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedVideo
{
	GENERATED_BODY()
public:
	
	// source url of video (only supports http(s) and attachments)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;
	
	// height of video
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Height;
	
	// width of video
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Width;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedProvider
{
	GENERATED_BODY()
public:
	// Provider name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Name;

	// Provider URL
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedAuthor
{
	GENERATED_BODY()
public:
	// name of author
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Name;
	
	// url of author
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;
	
	// url of author icon(only supports http(s) and attachments)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString IconUrl;

	// a proxied url of author icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString ProxyIconUrl;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordAllowedMentions
{
	GENERATED_BODY()
public:
	// array of allowed mention types
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<EDiscordAllowedMentionTypes> Parse;

	// list of snowflakes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<int64> Roles;

	// list of snowflakes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<int64> Users;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedField
{
	GENERATED_BODY()
public:
	// 	name of the field
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Name;

	// 	value of the field
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Value;

	// whether or not this field should display inline
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bInline;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbedFooter
{
	GENERATED_BODY()
public:
	// footer text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Text;

	// url of footer icon (only supports http(s) and attachments)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString IconUrl;

	// a proxied url of footer icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString ProxyIconUrl;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookEmbed
{
	GENERATED_BODY()
public:
	// title of embed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Title;
	
	// type of embed (always "rich" for webhook embeds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Type;
	
	// description of embed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Description;

	// 	url of embed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Url;

	// ISO8601 timestamp (2020-10-10T17:45:09.176000+00:00) of embed content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Timestamp;

	// 	color code of the embed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	int32 Color;

	// footer information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedFooter Footer;

	// image information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedImage Image;

	// Thumbnail information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedThumbnail Thumbnail;

	// Video information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedVideo Video;

	// Provider information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedProvider Provider;

	// Author information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordWebhookEmbedAuthor Author;

	// fields information
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<FDiscordWebhookEmbedField> Field;
};

USTRUCT(BlueprintType)
struct DISCORDHTTPAPI_API FDiscordWebhookData
{
	GENERATED_BODY()
public:
	// the message contents (up to 2000 characters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Content;

	// override the default username of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Username;

	// override the default avatar of the webhook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString Avatar;

	// true if this is a TTS message
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	bool bTts;

	// the contents of the file being sent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FString File;

	// embedded rich content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	TArray<FDiscordWebhookEmbed> Embeds;

	// allowed mentions for the message
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Webhook")
	FDiscordAllowedMentions AllowedMentions;
};

DECLARE_DELEGATE_TwoParams(FDiscordCreateWebhook,		const bool /* bSucces */, const FDiscordWebhook&			/* NewWebhook */);
DECLARE_DELEGATE_TwoParams(FDiscordGetWebhooks,			const bool /* bSucces */, const TArray<FDiscordWebhook>&	/* ChannelWebhooks */);
DECLARE_DELEGATE_TwoParams(FDiscordGetWebhook,			const bool /* bSucces */, const FDiscordWebhook&			/* NewWebhook */);
DECLARE_DELEGATE_OneParam (FDiscordDeleteWebhook,		const bool /* bSucces */);
DECLARE_DELEGATE_OneParam (FDiscordExecuteWebhook,		const bool /* bSucces */);

class DISCORDHTTPAPI_API FDiscordWebhookLibrary
{
public:
	FDiscordWebhookLibrary() = delete;
	FDiscordWebhookLibrary(const FDiscordWebhookLibrary&)  = delete;
	FDiscordWebhookLibrary(const FDiscordWebhookLibrary&&) = delete;

public:
	/**
	 * Create a new WebHook.
	 * @param Name				Name of the webhook (1-80 characters).
	 * @param ImageData			Image for the default webhook avatar.
	 * @param ChannelId			ID of the channel we want to create a webhook.
	 * @param OnWebhookCreated	Callback called when the server responded to the creation request.
	*/
	static void CreateWebhook(const FString& Name, const FString& ImageData, const int64& ChannelId, const FDiscordCreateWebhook& OnWebhookCreated = FDiscordCreateWebhook());

	/**
	 * Returns a list of channel webhook objects. Requires the MANAGE_WEBHOOKS permission.
	 * @param ChannelId					ID of the channel we want to get the webhooks from.
	 * @param OnChannelWebhooksReceived Called when the server sent us the webhooks list.
	*/
	static void GetChannelWebhooks(const int64& ChannelId, const FDiscordGetWebhooks& OnChannelWebhooksReceived);

	/**
	 * Returns a list of channel webhook objects. Requires the MANAGE_WEBHOOKS permission.
	 * @param GuildId					ID of the guild we want to get the webhooks from.
	 * @param OnChannelWebhooksReceived Called when the server sent us the webhooks list.
	*/
	static void GetGuildWebhooks(const int64& GuildId, const FDiscordGetWebhooks& OnChannelWebhooksReceived);

	/**
	 * Returns the webhook object for the given id.
	 * @param WebhookId			The ID of the webhook we want.
	 * @param OnWebhookReceived Called when the server sent us information about the webhook.
	*/
	static void GetWebhook(const int64& WebhookId, const FDiscordGetWebhook& OnWebhookReceived);

	/**
	 * Returns the webhook object for the given id.
	 * This call does not require authentication and returns no user in the webhook object.
	 * @param WebhookId			The ID of the webhook we want.
	 * @param Token				Token to authenticate.
	 * @param OnWebhookReceived Called when the server sent us information about the webhook.
	*/
	static void GetWebhookWithToken(const int64& WebhookId, const FString& Token, const FDiscordGetWebhook& OnWebhookReceived);

	/**
	 * Modify a webhook. Requires the MANAGE_WEBHOOKS permission. Returns the updated webhook object on success.
	 * @param WebhookId			The ID of the Webhook we want to modify.
	 * @param NewName			[Optional] The new name for this Webhook. Leave it empty to not update the name.
	 * @param NewAvatar			[Optional] The new avatar for this Webhook. Leave it empty to not update the avatar.
	 * @param NewChannelId		[Optional] The new channel ID for thiw Webhook. Leave it to 0 to not update channel id.
	 * @param OnWebhookUpdated	Called when the server updated the webhook.
	*/
	static void ModifyWebhook(const int64 WebhookId, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId, const FDiscordGetWebhook& OnWebhookUpdated = FDiscordGetWebhook());

	/**
	 * Modify a webhook. Requires the MANAGE_WEBHOOKS permission. Returns the updated webhook object on success.
	 * @param WebhookId			The ID of the Webhook we want to modify.
	 * @param NewName			[Optional] The new name for this Webhook. Leave it empty to not update the name.
	 * @param NewAvatar			[Optional] The new avatar for this Webhook. Leave it empty to not update the avatar.
	 * @param NewChannelId		[Optional] The new channel ID for thiw Webhook. Leave it to 0 to not update channel id.
	 * @param OnWebhookUpdated	Called when the server updated the webhook.
	*/
	static void ModifyWebhookWithToken(const int64 WebhookId, const FString& Token, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId, const FDiscordGetWebhook& OnWebhookUpdated = FDiscordGetWebhook());

	/**
	 * Delete a webhook permanently.
	 * @param WebhookId			The ID of the Webhook we want to remove.
	 * @param OnWebhookDeleted  Called when the server deleted the Webhook.
	*/
	static void DeleteWebhook(const int64 WebhookId, const FDiscordDeleteWebhook& OnWebhookDeleted = FDiscordDeleteWebhook());

	/**
	 * Delete a webhook permanently.
	 * @param WebhookId			The ID of the Webhook we want to remove.
	 * @param OnWebhookDeleted  Called when the server deleted the Webhook.
	*/
	static void DeleteWebhookWithToken(const int64 WebhookId, const FString& Token, const FDiscordDeleteWebhook& OnWebhookDeleted = FDiscordDeleteWebhook());

	/**
	 * Execute the Webhook.
	 * @param WebhookId			The ID of the webhook we want to execute.
	 * @param WebhookToken		The secure token of the webhook we want to execute.
	 * @param WebhookData		The data to send to the webhook.
	 * @param OnWebhookExecuted Called when the webhook has been executed.
	*/
	static void ExecuteWebhook(const int64 WebhookId, const FString& WebhookToken, const FDiscordWebhookData& WebhookData = FDiscordWebhookData(), const FDiscordExecuteWebhook& OnWebhookExecuted = FDiscordExecuteWebhook());
};

