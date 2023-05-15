// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Helper that matches each type used in the SDK to a friendly name to use in UE4.
 * Used internally, you shouldn't use it.
 * Note that members order for structs is important.
*/
namespace FRawDiscord
{

#ifndef _DISCORD_GAME_SDK_H_
#define _DISCORD_GAME_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define DISCORD_VERSION 2
#define DISCORD_APPLICATION_MANAGER_VERSION 1
#define DISCORD_USER_MANAGER_VERSION 1
#define DISCORD_IMAGE_MANAGER_VERSION 1
#define DISCORD_ACTIVITY_MANAGER_VERSION 1
#define DISCORD_RELATIONSHIP_MANAGER_VERSION 1
#define DISCORD_LOBBY_MANAGER_VERSION 1
#define DISCORD_NETWORK_MANAGER_VERSION 1
#define DISCORD_OVERLAY_MANAGER_VERSION 1
#define DISCORD_STORAGE_MANAGER_VERSION 1
#define DISCORD_STORE_MANAGER_VERSION 1
#define DISCORD_VOICE_MANAGER_VERSION 1
#define DISCORD_ACHIEVEMENT_MANAGER_VERSION 1

enum class EDiscordResult {
    Ok = 0,
    ServiceUnavailable = 1,
    InvalidVersion = 2,
    LockFailed = 3,
    InternalError = 4,
    InvalidPayload = 5,
    InvalidCommand = 6,
    InvalidPermissions = 7,
    NotFetched = 8,
    NotFound = 9,
    Conflict = 10,
    InvalidSecret = 11,
    InvalidJoinSecret = 12,
    NoEligibleActivity = 13,
    InvalidInvite = 14,
    NotAuthenticated = 15,
    InvalidAccessToken = 16,
    ApplicationMismatch = 17,
    InvalidDataUrl = 18,
    InvalidBase64 = 19,
    NotFiltered = 20,
    LobbyFull = 21,
    InvalidLobbySecret = 22,
    InvalidFilename = 23,
    InvalidFileSize = 24,
    InvalidEntitlement = 25,
    NotInstalled = 26,
    NotRunning = 27,
    InsufficientBuffer = 28,
    PurchaseCanceled = 29,
    InvalidGuild = 30,
    InvalidEvent = 31,
    InvalidChannel = 32,
    InvalidOrigin = 33,
    RateLimited = 34,
    OAuth2Error = 35,
    SelectChannelTimeout = 36,
    GetGuildTimeout = 37,
    SelectVoiceForceRequired = 38,
    CaptureShortcutAlreadyListening = 39,
    UnauthorizedForAchievement = 40,
    InvalidGiftCode = 41,
    PurchaseError = 42,
    TransactionAborted = 43,
    InvalidManager = 44
};

enum class EDiscordCreateFlags {
    Default = 0,
    NoRequireDiscord = 1,
};

enum class EDiscordLogLevel {
    Error = 1,
    Warn,
    Info,
    Debug,
};

enum class EDiscordUserFlag {
    Partner = 2,
    HypeSquadEvents = 4,
    HypeSquadHouse1 = 64,
    HypeSquadHouse2 = 128,
    HypeSquadHouse3 = 256,
};

enum class EDiscordPremiumType {
    None = 0,
    Tier1 = 1,
    Tier2 = 2,
};

enum class EDiscordImageType {
    User
};

enum class EDiscordActivityType {
    Playing,
    Streaming,
    Listening,
    Watching,
};

enum class EDiscordActivityActionType {
    Join = 1,
    Spectate,
};

enum class EDiscordActivityJoinRequestReply {
    No,
    Yes,
    Ignore,
};

enum class EDiscordStatus {
    Offline = 0,
    Online = 1,
    Idle = 2,
    DoNotDisturb = 3,
};

enum class EDiscordRelationshipType {
    None,
    Friend,
    Blocked,
    PendingIncoming,
    PendingOutgoing,
    Implicit,
};

enum class EDiscordLobbyType {
    Private = 1,
    Public,
};

enum class EDiscordLobbySearchComparison {
    LessThanOrEqual = -2,
    LessThan,
    Equal,
    GreaterThan,
    GreaterThanOrEqual,
    NotEqual,
};

enum class EDiscordLobbySearchCast {
    String = 1,
    Number,
};

enum class EDiscordLobbySearchDistance {
    Local,
    Default,
    Extended,
    Global,
};

enum class EDiscordEntitlementType {
    Purchase = 1,
    PremiumSubscription,
    DeveloperGift,
    TestModePurchase,
    FreePurchase,
    UserGift,
    PremiumPurchase,
};

enum class EDiscordSkuType {
    Application = 1,
    DLC,
    Consumable,
    Bundle,
};

enum class EDiscordInputModeType {
    VoiceActivity = 0,
    PushToTalk,
};

typedef int64_t DiscordClientId;
typedef int32_t DiscordVersion;
typedef int64_t DiscordSnowflake;
typedef int64_t DiscordTimestamp;
typedef DiscordSnowflake DiscordUserId;
typedef char DiscordLocale[128];
typedef char DiscordBranch[4096];
typedef DiscordSnowflake DiscordLobbyId;
typedef char DiscordLobbySecret[128];
typedef char DiscordMetadataKey[256];
typedef char DiscordMetadataValue[4096];
typedef uint64_t DiscordNetworkPeerId;
typedef uint8_t DiscordNetworkChannelId;
typedef char DiscordPath[4096];
typedef char DiscordDateTime[64];

struct DiscordUser {
    DiscordUserId id;
    char username[256];
    char discriminator[8];
    char avatar[128];
    bool bot;
};

struct DiscordOAuth2Token {
    char access_token[128];
    char scopes[1024];
    DiscordTimestamp expires;
};

struct DiscordImageHandle {
    EDiscordImageType type;
    int64_t id;
    uint32_t size;
};

struct DiscordImageDimensions {
    uint32_t width;
    uint32_t height;
};

struct DiscordActivityTimestamps {
    DiscordTimestamp start;
    DiscordTimestamp end;
};

struct DiscordActivityAssets {
    char large_image[128];
    char large_text[128];
    char small_image[128];
    char small_text[128];
};

struct DiscordPartySize {
    int32_t current_size;
    int32_t max_size;
};

struct DiscordActivityParty {
    char id[128];
    struct DiscordPartySize size;
};

struct DiscordActivitySecrets {
    char match[128];
    char join[128];
    char spectate[128];
};

struct DiscordActivity {
    EDiscordActivityType type;
    int64_t application_id;
    char name[128];
    char state[128];
    char details[128];
    struct DiscordActivityTimestamps timestamps;
    struct DiscordActivityAssets assets;
    struct DiscordActivityParty party;
    struct DiscordActivitySecrets secrets;
    bool instance;
};

struct DiscordPresence {
    EDiscordStatus status;
    struct DiscordActivity activity;
};

struct DiscordRelationship {
    EDiscordRelationshipType type;
    struct DiscordUser user;
    struct DiscordPresence presence;
};

struct DiscordLobby {
    DiscordLobbyId id;
    EDiscordLobbyType type;
    DiscordUserId owner_id;
    DiscordLobbySecret secret;
    uint32_t capacity;
    bool locked;
};

struct DiscordFileStat {
    char filename[260];
    uint64_t size;
    uint64_t last_modified;
};

struct DiscordEntitlement {
    DiscordSnowflake id;
    EDiscordEntitlementType type;
    DiscordSnowflake sku_id;
};

struct DiscordSkuPrice {
    uint32_t amount;
    char currency[16];
};

struct DiscordSku {
    DiscordSnowflake id;
    EDiscordSkuType type;
    char name[256];
    struct DiscordSkuPrice price;
};

struct DiscordInputMode {
    EDiscordInputModeType type;
    char shortcut[256];
};

struct DiscordUserAchievement {
    DiscordSnowflake user_id;
    DiscordSnowflake achievement_id;
    uint8_t percent_complete;
    DiscordDateTime unlocked_at;
};

struct IDiscordLobbyTransaction {
    EDiscordResult (*set_type)(struct IDiscordLobbyTransaction* lobby_transaction, EDiscordLobbyType type);
    EDiscordResult (*set_owner)(struct IDiscordLobbyTransaction* lobby_transaction, DiscordUserId owner_id);
    EDiscordResult (*set_capacity)(struct IDiscordLobbyTransaction* lobby_transaction, uint32_t capacity);
    EDiscordResult (*set_metadata)(struct IDiscordLobbyTransaction* lobby_transaction, DiscordMetadataKey key, DiscordMetadataValue value);
    EDiscordResult (*delete_metadata)(struct IDiscordLobbyTransaction* lobby_transaction, DiscordMetadataKey key);
    EDiscordResult (*set_locked)(struct IDiscordLobbyTransaction* lobby_transaction, bool locked);
};

struct IDiscordLobbyMemberTransaction {
    EDiscordResult (*set_metadata)(struct IDiscordLobbyMemberTransaction* lobby_member_transaction, DiscordMetadataKey key, DiscordMetadataValue value);
    EDiscordResult (*delete_metadata)(struct IDiscordLobbyMemberTransaction* lobby_member_transaction, DiscordMetadataKey key);
};

struct IDiscordLobbySearchQuery {
    EDiscordResult (*filter)(struct IDiscordLobbySearchQuery* lobby_search_query, DiscordMetadataKey key, EDiscordLobbySearchComparison comparison, EDiscordLobbySearchCast cast, DiscordMetadataValue value);
    EDiscordResult (*sort)(struct IDiscordLobbySearchQuery* lobby_search_query, DiscordMetadataKey key, EDiscordLobbySearchCast cast, DiscordMetadataValue value);
    EDiscordResult (*limit)(struct IDiscordLobbySearchQuery* lobby_search_query, uint32_t limit);
    EDiscordResult (*distance)(struct IDiscordLobbySearchQuery* lobby_search_query, EDiscordLobbySearchDistance distance);
};

typedef void* IDiscordApplicationEvents;

struct IDiscordApplicationManager {
    void (*validate_or_exit)(struct IDiscordApplicationManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*get_current_locale)(struct IDiscordApplicationManager* manager, DiscordLocale* locale);
    void (*get_current_branch)(struct IDiscordApplicationManager* manager, DiscordBranch* branch);
    void (*get_oauth2_token)(struct IDiscordApplicationManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordOAuth2Token* oauth2_token));
    void (*get_ticket)(struct IDiscordApplicationManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, const char* data));
};

struct IDiscordUserEvents {
    void (*on_current_user_update)(void* event_data);
};

struct IDiscordUserManager {
    EDiscordResult (*get_current_user)(struct IDiscordUserManager* manager, struct DiscordUser* current_user);
    void (*get_user)(struct IDiscordUserManager* manager, DiscordUserId user_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordUser* user));
    EDiscordResult (*get_current_user_premium_type)(struct IDiscordUserManager* manager, EDiscordPremiumType* premium_type);
    EDiscordResult (*current_user_has_flag)(struct IDiscordUserManager* manager, EDiscordUserFlag flag, bool* has_flag);
};

typedef void* IDiscordImageEvents;

struct IDiscordImageManager {
    void (*fetch)(struct IDiscordImageManager* manager, struct DiscordImageHandle handle, bool refresh, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordImageHandle handle_result));
    EDiscordResult (*get_dimensions)(struct IDiscordImageManager* manager, struct DiscordImageHandle handle, struct DiscordImageDimensions* dimensions);
    EDiscordResult (*get_data)(struct IDiscordImageManager* manager, struct DiscordImageHandle handle, uint8_t* data, uint32_t data_length);
};

struct IDiscordActivityEvents {
    void (*on_activity_join)(void* event_data, const char* secret);
    void (*on_activity_spectate)(void* event_data, const char* secret);
    void (*on_activity_join_request)(void* event_data, struct DiscordUser* user);
    void (*on_activity_invite)(void* event_data, EDiscordActivityActionType type, struct DiscordUser* user, struct DiscordActivity* activity);
};

struct IDiscordActivityManager {
    EDiscordResult (*register_command)(struct IDiscordActivityManager* manager, const char* command);
    EDiscordResult (*register_steam)(struct IDiscordActivityManager* manager, uint32_t steam_id);
    void (*update_activity)(struct IDiscordActivityManager* manager, struct DiscordActivity* activity, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*clear_activity)(struct IDiscordActivityManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*send_request_reply)(struct IDiscordActivityManager* manager, DiscordUserId user_id, EDiscordActivityJoinRequestReply reply, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*send_invite)(struct IDiscordActivityManager* manager, DiscordUserId user_id, EDiscordActivityActionType type, const char* content, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*accept_invite)(struct IDiscordActivityManager* manager, DiscordUserId user_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
};

struct IDiscordRelationshipEvents {
    void (*on_refresh)(void* event_data);
    void (*on_relationship_update)(void* event_data, struct DiscordRelationship* relationship);
};

struct IDiscordRelationshipManager {
    void (*filter)(struct IDiscordRelationshipManager* manager, void* filter_data, bool (*filter)(void* filter_data, struct DiscordRelationship* relationship));
    EDiscordResult (*count)(struct IDiscordRelationshipManager* manager, int32_t* count);
    EDiscordResult (*get)(struct IDiscordRelationshipManager* manager, DiscordUserId user_id, struct DiscordRelationship* relationship);
    EDiscordResult (*get_at)(struct IDiscordRelationshipManager* manager, uint32_t index, struct DiscordRelationship* relationship);
};

struct IDiscordLobbyEvents {
    void (*on_lobby_update)(void* event_data, int64_t lobby_id);
    void (*on_lobby_delete)(void* event_data, int64_t lobby_id, uint32_t reason);
    void (*on_member_connect)(void* event_data, int64_t lobby_id, int64_t user_id);
    void (*on_member_update)(void* event_data, int64_t lobby_id, int64_t user_id);
    void (*on_member_disconnect)(void* event_data, int64_t lobby_id, int64_t user_id);
    void (*on_lobby_message)(void* event_data, int64_t lobby_id, int64_t user_id, uint8_t* data, uint32_t data_length);
    void (*on_speaking)(void* event_data, int64_t lobby_id, int64_t user_id, bool speaking);
    void (*on_network_message)(void* event_data, int64_t lobby_id, int64_t user_id, uint8_t channel_id, uint8_t* data, uint32_t data_length);
};

struct IDiscordLobbyManager {
    EDiscordResult (*get_lobby_create_transaction)(struct IDiscordLobbyManager* manager, struct IDiscordLobbyTransaction** transaction);
    EDiscordResult (*get_lobby_update_transaction)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, struct IDiscordLobbyTransaction** transaction);
    EDiscordResult (*get_member_update_transaction)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, struct IDiscordLobbyMemberTransaction** transaction);
    void (*create_lobby)(struct IDiscordLobbyManager* manager, struct IDiscordLobbyTransaction* transaction, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordLobby* lobby));
    void (*update_lobby)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, struct IDiscordLobbyTransaction* transaction, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*delete_lobby)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*connect_lobby)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordLobbySecret secret, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordLobby* lobby));
    void (*connect_lobby_with_activity_secret)(struct IDiscordLobbyManager* manager, DiscordLobbySecret activity_secret, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, struct DiscordLobby* lobby));
    void (*disconnect_lobby)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    EDiscordResult (*get_lobby)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, struct DiscordLobby* lobby);
    EDiscordResult (*get_lobby_activity_secret)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordLobbySecret* secret);
    EDiscordResult (*get_lobby_metadata_value)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordMetadataKey key, DiscordMetadataValue* value);
    EDiscordResult (*get_lobby_metadata_key)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t index, DiscordMetadataKey* key);
    EDiscordResult (*lobby_metadata_count)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t* count);
    EDiscordResult (*member_count)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t* count);
    EDiscordResult (*get_member_user_id)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t index, DiscordUserId* user_id);
    EDiscordResult (*get_member_user)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, struct DiscordUser* user);
    EDiscordResult (*get_member_metadata_value)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, DiscordMetadataKey key, DiscordMetadataValue* value);
    EDiscordResult (*get_member_metadata_key)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, int32_t index, DiscordMetadataKey* key);
    EDiscordResult (*member_metadata_count)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, int32_t* count);
    void (*update_member)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, struct IDiscordLobbyMemberTransaction* transaction, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*send_lobby_message)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, uint8_t* data, uint32_t data_length, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    EDiscordResult (*get_search_query)(struct IDiscordLobbyManager* manager, struct IDiscordLobbySearchQuery** query);
    void (*search)(struct IDiscordLobbyManager* manager, struct IDiscordLobbySearchQuery* query, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*lobby_count)(struct IDiscordLobbyManager* manager, int32_t* count);
    EDiscordResult (*get_lobby_id)(struct IDiscordLobbyManager* manager, int32_t index, DiscordLobbyId* lobby_id);
    void (*connect_voice)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*disconnect_voice)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    EDiscordResult (*connect_network)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id);
    EDiscordResult (*disconnect_network)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id);
    EDiscordResult (*flush_network)(struct IDiscordLobbyManager* manager);
    EDiscordResult (*open_network_channel)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, uint8_t channel_id, bool reliable);
    EDiscordResult (*send_network_message)(struct IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, uint8_t channel_id, uint8_t* data, uint32_t data_length);
};

struct IDiscordNetworkEvents {
    void (*on_message)(void* event_data, DiscordNetworkPeerId peer_id, DiscordNetworkChannelId channel_id, uint8_t* data, uint32_t data_length);
    void (*on_route_update)(void* event_data, const char* route_data);
};

struct IDiscordNetworkManager {
    /**
     * Get the local peer ID for this process.
     */
    void (*get_peer_id)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId* peer_id);
    /**
     * Send pending network messages.
     */
    EDiscordResult (*flush)(struct IDiscordNetworkManager* manager);
    /**
     * Open a connection to a remote peer.
     */
    EDiscordResult (*open_peer)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id, const char* route_data);
    /**
     * Update the route data for a connected peer.
     */
    EDiscordResult (*update_peer)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id, const char* route_data);
    /**
     * Close the connection to a remote peer.
     */
    EDiscordResult (*close_peer)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id);
    /**
     * Open a message channel to a connected peer.
     */
    EDiscordResult (*open_channel)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id, DiscordNetworkChannelId channel_id, bool reliable);
    /**
     * Close a message channel to a connected peer.
     */
    EDiscordResult (*close_channel)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id, DiscordNetworkChannelId channel_id);
    /**
     * Send a message to a connected peer over an opened message channel.
     */
    EDiscordResult (*send_message)(struct IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id, DiscordNetworkChannelId channel_id, uint8_t* data, uint32_t data_length);
};

struct IDiscordOverlayEvents {
    void (*on_toggle)(void* event_data, bool locked);
};

struct IDiscordOverlayManager {
    void (*is_enabled)(struct IDiscordOverlayManager* manager, bool* enabled);
    void (*is_locked)(struct IDiscordOverlayManager* manager, bool* locked);
    void (*set_locked)(struct IDiscordOverlayManager* manager, bool locked, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*open_activity_invite)(struct IDiscordOverlayManager* manager, EDiscordActivityActionType type, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*open_guild_invite)(struct IDiscordOverlayManager* manager, const char* code, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*open_voice_settings)(struct IDiscordOverlayManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
};

typedef void* IDiscordStorageEvents;

struct IDiscordStorageManager {
    EDiscordResult (*read)(struct IDiscordStorageManager* manager, const char* name, uint8_t* data, uint32_t data_length, uint32_t* read);
    void (*read_async)(struct IDiscordStorageManager* manager, const char* name, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, uint8_t* data, uint32_t data_length));
    void (*read_async_partial)(struct IDiscordStorageManager* manager, const char* name, uint64_t offset, uint64_t length, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result, uint8_t* data, uint32_t data_length));
    EDiscordResult (*write)(struct IDiscordStorageManager* manager, const char* name, uint8_t* data, uint32_t data_length);
    void (*write_async)(struct IDiscordStorageManager* manager, const char* name, uint8_t* data, uint32_t data_length, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    EDiscordResult (*delete_)(struct IDiscordStorageManager* manager, const char* name);
    EDiscordResult (*exists)(struct IDiscordStorageManager* manager, const char* name, bool* exists);
    void (*count)(struct IDiscordStorageManager* manager, int32_t* count);
    EDiscordResult (*stat)(struct IDiscordStorageManager* manager, const char* name, struct DiscordFileStat* stat);
    EDiscordResult (*stat_at)(struct IDiscordStorageManager* manager, int32_t index, struct DiscordFileStat* stat);
    EDiscordResult (*get_path)(struct IDiscordStorageManager* manager, DiscordPath* path);
};

struct IDiscordStoreEvents {
    void (*on_entitlement_create)(void* event_data, struct DiscordEntitlement* entitlement);
    void (*on_entitlement_delete)(void* event_data, struct DiscordEntitlement* entitlement);
};

struct IDiscordStoreManager {
    void (*fetch_skus)(struct IDiscordStoreManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*count_skus)(struct IDiscordStoreManager* manager, int32_t* count);
    EDiscordResult (*get_sku)(struct IDiscordStoreManager* manager, DiscordSnowflake sku_id, struct DiscordSku* sku);
    EDiscordResult (*get_sku_at)(struct IDiscordStoreManager* manager, int32_t index, struct DiscordSku* sku);
    void (*fetch_entitlements)(struct IDiscordStoreManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*count_entitlements)(struct IDiscordStoreManager* manager, int32_t* count);
    EDiscordResult (*get_entitlement)(struct IDiscordStoreManager* manager, DiscordSnowflake entitlement_id, struct DiscordEntitlement* entitlement);
    EDiscordResult (*get_entitlement_at)(struct IDiscordStoreManager* manager, int32_t index, struct DiscordEntitlement* entitlement);
    EDiscordResult (*has_sku_entitlement)(struct IDiscordStoreManager* manager, DiscordSnowflake sku_id, bool* has_entitlement);
    void (*start_purchase)(struct IDiscordStoreManager* manager, DiscordSnowflake sku_id, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
};

struct IDiscordVoiceEvents {
    void (*on_settings_update)(void* event_data);
};

struct IDiscordVoiceManager {
    EDiscordResult (*get_input_mode)(struct IDiscordVoiceManager* manager, struct DiscordInputMode* input_mode);
    void (*set_input_mode)(struct IDiscordVoiceManager* manager, struct DiscordInputMode input_mode, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    EDiscordResult (*is_self_mute)(struct IDiscordVoiceManager* manager, bool* mute);
    EDiscordResult (*set_self_mute)(struct IDiscordVoiceManager* manager, bool mute);
    EDiscordResult (*is_self_deaf)(struct IDiscordVoiceManager* manager, bool* deaf);
    EDiscordResult (*set_self_deaf)(struct IDiscordVoiceManager* manager, bool deaf);
    EDiscordResult (*is_local_mute)(struct IDiscordVoiceManager* manager, DiscordSnowflake user_id, bool* mute);
    EDiscordResult (*set_local_mute)(struct IDiscordVoiceManager* manager, DiscordSnowflake user_id, bool mute);
    EDiscordResult (*get_local_volume)(struct IDiscordVoiceManager* manager, DiscordSnowflake user_id, uint8_t* volume);
    EDiscordResult (*set_local_volume)(struct IDiscordVoiceManager* manager, DiscordSnowflake user_id, uint8_t volume);
};

struct IDiscordAchievementEvents {
    void (*on_user_achievement_update)(void* event_data, struct DiscordUserAchievement* user_achievement);
};

struct IDiscordAchievementManager {
    void (*set_user_achievement)(struct IDiscordAchievementManager* manager, DiscordSnowflake achievement_id, uint8_t percent_complete, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*fetch_user_achievements)(struct IDiscordAchievementManager* manager, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
    void (*count_user_achievements)(struct IDiscordAchievementManager* manager, int32_t* count);
    EDiscordResult (*get_user_achievement)(struct IDiscordAchievementManager* manager, DiscordSnowflake user_achievement_id, struct DiscordUserAchievement* user_achievement);
    EDiscordResult (*get_user_achievement_at)(struct IDiscordAchievementManager* manager, int32_t index, struct DiscordUserAchievement* user_achievement);
};

typedef void* IDiscordCoreEvents;

struct IDiscordCore {
    void (*destroy)(struct IDiscordCore* core);
    EDiscordResult (*run_callbacks)(struct IDiscordCore* core);
    void (*set_log_hook)(struct IDiscordCore* core, EDiscordLogLevel min_level, void* hook_data, void (*hook)(void* hook_data, EDiscordLogLevel level, const char* message));
    struct IDiscordApplicationManager* (*get_application_manager)(struct IDiscordCore* core);
    struct IDiscordUserManager* (*get_user_manager)(struct IDiscordCore* core);
    struct IDiscordImageManager* (*get_image_manager)(struct IDiscordCore* core);
    struct IDiscordActivityManager* (*get_activity_manager)(struct IDiscordCore* core);
    struct IDiscordRelationshipManager* (*get_relationship_manager)(struct IDiscordCore* core);
    struct IDiscordLobbyManager* (*get_lobby_manager)(struct IDiscordCore* core);
    struct IDiscordNetworkManager* (*get_network_manager)(struct IDiscordCore* core);
    struct IDiscordOverlayManager* (*get_overlay_manager)(struct IDiscordCore* core);
    struct IDiscordStorageManager* (*get_storage_manager)(struct IDiscordCore* core);
    struct IDiscordStoreManager* (*get_store_manager)(struct IDiscordCore* core);
    struct IDiscordVoiceManager* (*get_voice_manager)(struct IDiscordCore* core);
    struct IDiscordAchievementManager* (*get_achievement_manager)(struct IDiscordCore* core);
};

struct DiscordCreateParams {
    DiscordClientId client_id;
    uint64_t flags;
    IDiscordCoreEvents* events;
    void* event_data;
    IDiscordApplicationEvents* application_events;
    DiscordVersion application_version;
    struct IDiscordUserEvents* user_events;
    DiscordVersion user_version;
    IDiscordImageEvents* image_events;
    DiscordVersion image_version;
    struct IDiscordActivityEvents* activity_events;
    DiscordVersion activity_version;
    struct IDiscordRelationshipEvents* relationship_events;
    DiscordVersion relationship_version;
    struct IDiscordLobbyEvents* lobby_events;
    DiscordVersion lobby_version;
    struct IDiscordNetworkEvents* network_events;
    DiscordVersion network_version;
    struct IDiscordOverlayEvents* overlay_events;
    DiscordVersion overlay_version;
    IDiscordStorageEvents* storage_events;
    DiscordVersion storage_version;
    struct IDiscordStoreEvents* store_events;
    DiscordVersion store_version;
    struct IDiscordVoiceEvents* voice_events;
    DiscordVersion voice_version;
    struct IDiscordAchievementEvents* achievement_events;
    DiscordVersion achievement_version;
};

#ifdef __cplusplus
inline
#else
static
#endif
void DiscordCreateParamsSetDefault(struct DiscordCreateParams* params)
{
    memset(params, 0, sizeof(struct DiscordCreateParams));
    params->application_version = DISCORD_APPLICATION_MANAGER_VERSION;
    params->user_version = DISCORD_USER_MANAGER_VERSION;
    params->image_version = DISCORD_IMAGE_MANAGER_VERSION;
    params->activity_version = DISCORD_ACTIVITY_MANAGER_VERSION;
    params->relationship_version = DISCORD_RELATIONSHIP_MANAGER_VERSION;
    params->lobby_version = DISCORD_LOBBY_MANAGER_VERSION;
    params->network_version = DISCORD_NETWORK_MANAGER_VERSION;
    params->overlay_version = DISCORD_OVERLAY_MANAGER_VERSION;
    params->storage_version = DISCORD_STORAGE_MANAGER_VERSION;
    params->store_version = DISCORD_STORE_MANAGER_VERSION;
    params->voice_version = DISCORD_VOICE_MANAGER_VERSION;
    params->achievement_version = DISCORD_ACHIEVEMENT_MANAGER_VERSION;
}

EDiscordResult DiscordCreate(DiscordVersion version, struct DiscordCreateParams* params, struct IDiscordCore** result);

#ifdef __cplusplus
}
#endif

#endif
};
