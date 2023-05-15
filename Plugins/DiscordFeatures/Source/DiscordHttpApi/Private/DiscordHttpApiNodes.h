// Copy

#pragma once

#include "DiscordWebhookLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordHttpApiNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebhookCreated, const FDiscordWebhook&, NewWebhoook);

UCLASS()
class UCreateWebhookProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the new webhook has been created.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnWebhookCreated OnWebhookCreated;
	
	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnWebhookCreated OnError;	

public:
	UCreateWebhookProxy() {};

	virtual void Activate();

	/**
	 * Create a new Webhook.
	 * @param Name The name of the new Webhook
	 * @param ImageData The avatar of the new Webhook
	 * @param ChannelId The id of the channel this webhook is attached to.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Create Webhook"))
	static UCreateWebhookProxy* CreateWebhook(const FString& Name, const FString& ImageData, const int64 ChannelId);

private:
	void OnWebhookCreatedInternal(const bool bSuccess, const FDiscordWebhook& NewWebhook);

private:
	FString Name;
	FString ImageData;
	int64   ChannelId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetWebhooks, const TArray<FDiscordWebhook>&, Webhooks);

UCLASS()
class UGetChannelWebhooksProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when we received the webhooks.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhooks OnWebhooksReceived;
	
	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhooks OnError;	

public:
	UGetChannelWebhooksProxy() {};

	virtual void Activate();

	/**
	 * Get the webhooks associated with the channel.
	 * @param ChannelId The id of the channel we want to get the webhooks.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Channel Webhooks"))
	static UGetChannelWebhooksProxy* GetChannelWebhooks(const int64 ChannelId);

private:
	void OnWebhooksReceivedInternal(const bool bSuccess, const TArray<FDiscordWebhook>& Webhooks);

private:
	int64   ChannelId;
};


UCLASS()
class UGetGuildWebhooksProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when we received the webhook.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhooks OnWebhooksReceived;
	
	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhooks OnError;	

public:
	UGetGuildWebhooksProxy() {};

	virtual void Activate();

	/**
	 * Get the webhooks associated with the channel.
	 * @param GuildId The id of the guild we want to get the webhooks.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Guild Webhooks"))
	static UGetGuildWebhooksProxy* GetGuildWebhooks(const int64 GuildId);

private:
	void OnWebhooksReceivedInternal(const bool bSuccess, const TArray<FDiscordWebhook>& Webhooks);

private:
	int64 GuildId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetWebhook, const FDiscordWebhook&, Webhoook);

UCLASS()
class UGetWebhookProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when we received the webhook.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnWebhookReceived;
	
	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnError;	

public:
	UGetWebhookProxy() {};

	virtual void Activate();

	/**
	 * Get the webhook from its ID.
	 * @param WebhookId The id of the webhook we want to get.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Webhook"))
	static UGetWebhookProxy* GetWebhook(const int64 WebhookId);

private:
	void OnWebhookReceivedInternal(const bool bSuccess, const FDiscordWebhook& Webhooks);

private:
	int64 WebhookId;
};

UCLASS()
class UGetWebhookWithTokenProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when we received the webhook.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnWebhookReceived;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnError;

public:
	UGetWebhookWithTokenProxy() {};

	virtual void Activate();

	/**
	 * Get the webhook from its ID.
	 * @param WebhookId The id of the webhook we want to get.
	 * @param Token		The token used to authenticate.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Get Webhook with Token"))
	static UGetWebhookWithTokenProxy* GetWebhookWithToken(const int64 WebhookId, const FString& Token);

private:
	void OnWebhookReceivedInternal(const bool bSuccess, const FDiscordWebhook& Webhooks);

private:
	int64 WebhookId;
	FString Token;
};

UCLASS()
class UModifyWebhookWithTokenProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the webhook has been modified.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnWebhooksModified;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnError;

public:
	UModifyWebhookWithTokenProxy() {};

	virtual void Activate();

	/**
	 * Modify a webhook. Requires the MANAGE_WEBHOOKS permission. Returns the updated webhook object on success.
	 * @param WebhookId			The ID of the Webhook we want to modify.
	 * @param Token				The token used to authenticate.
	 * @param NewName			[Optional] The new name for this Webhook. Leave it empty to not update the name.
	 * @param NewAvatar			[Optional] The new avatar for this Webhook. Leave it empty to not update the avatar.
	 * @param NewChannelId		[Optional] The new channel ID for thiw Webhook. Leave it to 0 to not update channel id.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Modify Webhook with Token"))
	static UModifyWebhookWithTokenProxy* ModifyWebhookWithToken(const int64 WebhookId, const FString& Token, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId);

private:
	void OnResponse(const bool bSuccess, const FDiscordWebhook& Webhooks);

private:
	FString Token;
	int64 WebhookId;
	FString NewName;
	FString NewAvatar;
	int64 NewChannelId;
};

UCLASS()
class UModifyWebhookProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the webhook has been modified.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnWebhooksModified;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnGetWebhook OnError;

public:
	UModifyWebhookProxy() {};

	virtual void Activate();

	/**
	 * Modify a webhook. Requires the MANAGE_WEBHOOKS permission. Returns the updated webhook object on success.
	 * @param WebhookId			The ID of the Webhook we want to modify.
	 * @param NewName			[Optional] The new name for this Webhook. Leave it empty to not update the name.
	 * @param NewAvatar			[Optional] The new avatar for this Webhook. Leave it empty to not update the avatar.
	 * @param NewChannelId		[Optional] The new channel ID for thiw Webhook. Leave it to 0 to not update channel id.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Modify Webhook"))
	static UModifyWebhookProxy* ModifyWebhook(const int64 WebhookId, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId);

private:
	void OnResponse(const bool bSuccess, const FDiscordWebhook& Webhooks);

private:
	int64 WebhookId;
	FString NewName;
	FString NewAvatar;
	int64 NewChannelId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscordWebhookEvent);

UCLASS()
class UDeleteWebhookProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the webhook has been deleted.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnWebhookDeleted;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnError;

public:
	UDeleteWebhookProxy() {};

	virtual void Activate();

	/**
	 * Delete a webhook permanently.
	 * @param WebhookId			The ID of the Webhook we want to remove.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Delete Webhook"))
	static UDeleteWebhookProxy* DeleteWebhook(const int64 WebhookId);

private:
	void OnResponse(const bool bSuccess);

private:
	int64 WebhookId;
};

UCLASS()
class UDeleteWebhookWithTokenProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the webhook has been deleted.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnWebhookDeleted;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnError;

public:
	UDeleteWebhookWithTokenProxy() {};

	virtual void Activate();

	/**
	 * Delete a webhook permanently.
	 * @param WebhookId			The ID of the Webhook we want to remove.
	 * @param Token				The token used to authenticate.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Delete Webhook with Token"))
	static UDeleteWebhookWithTokenProxy* DeleteWebhookWithToken(const int64 WebhookId, const FString& Token);

private:
	void OnResponse(const bool bSuccess);

private:
	int64 WebhookId;
	FString Token;
};

UCLASS()
class UExecuteWebhookProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the webhook has been deleted.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnWebhookExecuted;

	/**
	 * Called when an error occurred.
	 * See the output for more details.
	*/
	UPROPERTY(BlueprintAssignable)
	FDiscordWebhookEvent OnError;

public:
	UExecuteWebhookProxy() {};

	virtual void Activate();

	/**
	 * Execute the Webhook.
	 * @param WebhookId			The ID of the webhook we want to execute.
	 * @param WebhookToken		The secure token of the webhook we want to execute.
	 * @param WebhookData		The data to send to the webhook.
	*/
	UFUNCTION(BlueprintCallable, Category = "Discord|Webhook", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Execute Webhook"))
	static UExecuteWebhookProxy* ExecuteWebhook(const int64 WebhookId, const FString& WebhookToken, const FDiscordWebhookData& WebhookData);

private:
	void OnResponse(const bool bSuccess);

private:
	int64 WebhookId; 
	FString WebhookToken;
	FDiscordWebhookData WebhookData;
};
