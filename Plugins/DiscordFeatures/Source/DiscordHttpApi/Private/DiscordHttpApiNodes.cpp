// Copy

#include "DiscordHttpApiNodes.h"

UCreateWebhookProxy* UCreateWebhookProxy::CreateWebhook(const FString& Name, const FString& ImageData, const int64 ChannelId)
{
	UCreateWebhookProxy* const Proxy = NewObject<UCreateWebhookProxy>();

	Proxy->ChannelId	= ChannelId;
	Proxy->ImageData	= ImageData;
	Proxy->Name			= Name;

	return Proxy;
}

void UCreateWebhookProxy::Activate()
{
	FDiscordWebhookLibrary::CreateWebhook(Name, ImageData, ChannelId, FDiscordCreateWebhook::CreateUObject(this, &UCreateWebhookProxy::OnWebhookCreatedInternal));
}

void UCreateWebhookProxy::OnWebhookCreatedInternal(const bool bSuccess, const FDiscordWebhook& NewWebhook)
{
	(bSuccess ? OnWebhookCreated : OnError).Broadcast(NewWebhook);
	SetReadyToDestroy();
}

UGetChannelWebhooksProxy* UGetChannelWebhooksProxy::GetChannelWebhooks(const int64 ChannelId)
{
	UGetChannelWebhooksProxy* const Proxy = NewObject<UGetChannelWebhooksProxy>();

	Proxy->ChannelId = ChannelId;

	return Proxy;
}

void UGetChannelWebhooksProxy::Activate()
{
	FDiscordWebhookLibrary::GetChannelWebhooks(ChannelId, FDiscordGetWebhooks::CreateUObject(this, &UGetChannelWebhooksProxy::OnWebhooksReceivedInternal));
}

void UGetChannelWebhooksProxy::OnWebhooksReceivedInternal(const bool bSuccess, const TArray<FDiscordWebhook>& Webhooks)
{
	(bSuccess ? OnWebhooksReceived : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UGetGuildWebhooksProxy* UGetGuildWebhooksProxy::GetGuildWebhooks(const int64 GuildId)
{
	UGetGuildWebhooksProxy* const Proxy = NewObject<UGetGuildWebhooksProxy>();

	Proxy->GuildId = GuildId;

	return Proxy;
}

void UGetGuildWebhooksProxy::Activate()
{
	FDiscordWebhookLibrary::GetGuildWebhooks(GuildId, FDiscordGetWebhooks::CreateUObject(this, &UGetGuildWebhooksProxy::OnWebhooksReceivedInternal));
}

void UGetGuildWebhooksProxy::OnWebhooksReceivedInternal(const bool bSuccess, const TArray<FDiscordWebhook>& Webhooks)
{
	(bSuccess ? OnWebhooksReceived : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UGetWebhookProxy* UGetWebhookProxy::GetWebhook(const int64 WebhookId)
{
	UGetWebhookProxy* const Proxy = NewObject<UGetWebhookProxy >();

	Proxy->WebhookId = WebhookId;

	return Proxy;
}

void UGetWebhookProxy::Activate()
{
	FDiscordWebhookLibrary::GetWebhook(WebhookId, FDiscordGetWebhook::CreateUObject(this, &UGetWebhookProxy::OnWebhookReceivedInternal));
}

void UGetWebhookProxy::OnWebhookReceivedInternal(const bool bSuccess, const FDiscordWebhook& Webhooks)
{
	(bSuccess ? OnWebhookReceived : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UGetWebhookWithTokenProxy* UGetWebhookWithTokenProxy::GetWebhookWithToken(const int64 WebhookId, const FString& Token)
{
	UGetWebhookWithTokenProxy* const Proxy = NewObject<UGetWebhookWithTokenProxy>();

	Proxy->WebhookId = WebhookId;
	Proxy->Token	 = Token;

	return Proxy;
}

void UGetWebhookWithTokenProxy::Activate()
{
	FDiscordWebhookLibrary::GetWebhookWithToken(WebhookId, Token, FDiscordGetWebhook::CreateUObject(this, &UGetWebhookWithTokenProxy::OnWebhookReceivedInternal));
}

void UGetWebhookWithTokenProxy::OnWebhookReceivedInternal(const bool bSuccess, const FDiscordWebhook& Webhooks)
{
	(bSuccess ? OnWebhookReceived : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UModifyWebhookWithTokenProxy* UModifyWebhookWithTokenProxy::ModifyWebhookWithToken(const int64 WebhookId, const FString& Token, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId)
{
	UModifyWebhookWithTokenProxy* const Proxy = NewObject<UModifyWebhookWithTokenProxy>();

	Proxy->WebhookId	= WebhookId;
	Proxy->Token		= Token;
	Proxy->NewName		= NewName;
	Proxy->NewAvatar	= NewAvatar;
	Proxy->NewChannelId = NewChannelId;

	return Proxy;
}

void UModifyWebhookWithTokenProxy::Activate()
{
	FDiscordWebhookLibrary::ModifyWebhookWithToken(WebhookId, Token, NewName, NewAvatar, NewChannelId, FDiscordGetWebhook::CreateUObject(this, &UModifyWebhookWithTokenProxy::OnResponse));
}

void UModifyWebhookWithTokenProxy::OnResponse(const bool bSuccess, const FDiscordWebhook& Webhooks)
{
	(bSuccess ? OnWebhooksModified : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UModifyWebhookProxy* UModifyWebhookProxy::ModifyWebhook(const int64 WebhookId, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId)
{
	UModifyWebhookProxy* const Proxy = NewObject<UModifyWebhookProxy>();

	Proxy->WebhookId	= WebhookId;
	Proxy->NewName		= NewName;
	Proxy->NewAvatar	= NewAvatar;
	Proxy->NewChannelId = NewChannelId;

	return Proxy;
}

void UModifyWebhookProxy::Activate()
{
	FDiscordWebhookLibrary::ModifyWebhook(WebhookId, NewName, NewAvatar, NewChannelId, FDiscordGetWebhook::CreateUObject(this, &UModifyWebhookProxy::OnResponse));
}

void UModifyWebhookProxy::OnResponse(const bool bSuccess, const FDiscordWebhook& Webhooks)
{
	(bSuccess ? OnWebhooksModified : OnError).Broadcast(Webhooks);
	SetReadyToDestroy();
}

UDeleteWebhookProxy* UDeleteWebhookProxy::DeleteWebhook(const int64 WebhookId)
{
	UDeleteWebhookProxy* const Proxy = NewObject<UDeleteWebhookProxy>();

	Proxy->WebhookId = WebhookId;

	return Proxy;
}

void UDeleteWebhookProxy::Activate()
{
	FDiscordWebhookLibrary::DeleteWebhook(WebhookId, FDiscordDeleteWebhook::CreateUObject(this, &UDeleteWebhookProxy::OnResponse));
}

void UDeleteWebhookProxy::OnResponse(const bool bSuccess)
{
	(bSuccess ? OnWebhookDeleted : OnError).Broadcast();
	SetReadyToDestroy();
}

UDeleteWebhookWithTokenProxy* UDeleteWebhookWithTokenProxy::DeleteWebhookWithToken(const int64 WebhookId, const FString& Token)
{
	UDeleteWebhookWithTokenProxy* const Proxy = NewObject<UDeleteWebhookWithTokenProxy>();

	Proxy->WebhookId = WebhookId;
	Proxy->Token	 = Token;

	return Proxy;
}

void UDeleteWebhookWithTokenProxy::Activate()
{
	FDiscordWebhookLibrary::DeleteWebhookWithToken(WebhookId, Token, FDiscordDeleteWebhook::CreateUObject(this, &UDeleteWebhookWithTokenProxy::OnResponse));
}

void UDeleteWebhookWithTokenProxy::OnResponse(const bool bSuccess)
{
	(bSuccess ? OnWebhookDeleted : OnError).Broadcast();
	SetReadyToDestroy();
}


UExecuteWebhookProxy* UExecuteWebhookProxy::ExecuteWebhook(const int64 WebhookId, const FString& WebhookToken, const FDiscordWebhookData& WebhookData)
{
	UExecuteWebhookProxy* const Proxy = NewObject<UExecuteWebhookProxy>();

	Proxy->WebhookId		 = WebhookId;
	Proxy->WebhookData		 = WebhookData;
	Proxy->WebhookToken		 = WebhookToken;

	return Proxy;
}

void UExecuteWebhookProxy::Activate()
{
	FDiscordExecuteWebhook Callback;
	if (OnWebhookExecuted.IsBound() || OnError.IsBound())
	{
		Callback = FDiscordExecuteWebhook::CreateUObject(this, &UExecuteWebhookProxy::OnResponse);
	}
	FDiscordWebhookLibrary::ExecuteWebhook(WebhookId, WebhookToken, WebhookData, Callback);
}

void UExecuteWebhookProxy::OnResponse(const bool bSuccess)
{
	(bSuccess ? OnWebhookExecuted : OnError).Broadcast();
	SetReadyToDestroy();
}
