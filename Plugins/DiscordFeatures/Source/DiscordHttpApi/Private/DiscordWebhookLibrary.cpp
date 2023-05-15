// Copy

#include "DiscordWebhookLibrary.h"

#include "DiscordHttpApi.h"

#include "Http.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

#include "Serialization/JsonSerializer.h"
#include "Policies/CondensedJsonPrintPolicy.h"

DECLARE_LOG_CATEGORY_CLASS(LogDiscordWebhook, Log, All);

static constexpr int32	 DiscordWebhookNameMaxLength	= 80;
static const	 FString DiscordWebhookUrlRoot			= TEXT("https://discord.com/api");

namespace NWebhookHelper
{
	FString									MakeUrl				(const FString& RelativeRoute);
	TSharedPtr<class FJsonObject>			ParseJson			(const FString& Json);
	TArray<TSharedPtr<class FJsonValue>>	ParseJsonArray		(const FString& Json);
	TArray<EDiscordWebhookUserFlag>			MakeUserFlags		(const int64& Flags);
	void									JsonObjectToWebhook	(const TSharedPtr<class FJsonObject>& Object, FDiscordWebhook&		OutWebhook);
	void									JsonObjectToUser	(const TSharedPtr<class FJsonObject>& Object, FDiscordWebhookUser&	OutUser);
	FString									BuildJsonForModify	(const FString& Name, const FString& Avatar, const int64& ChannelId);

	template<class T>
	void SendWebhookRequest(const FString& Url, const FString& Verb, const FString& BodyData, const bool bAuthenticated, const T& Callback);
}

template<class T>
void NWebhookHelper::SendWebhookRequest(const FString& Url, const FString& Verb, const FString& BodyData, const bool bAuthenticated, const T& Callback)
{
	const FHttpRequestRef Request = bAuthenticated ? FDiscordHttpApiModule::Get().CreateAuthorizedRequest() : FHttpModule::Get().CreateRequest();

	Request->SetURL(MakeUrl(Url));
	Request->SetVerb(Verb);

	if (!BodyData.IsEmpty())
	{
		Request->SetContentAsString(BodyData);
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	}

	Request->OnProcessRequestComplete().BindLambda(Callback);

	ensure(Request->ProcessRequest());
}

FString NWebhookHelper::BuildJsonForModify(const FString& Name, const FString& Avatar, const int64& ChannelId)
{
	FString Content = TEXT("{");

	if (!Name.IsEmpty())
	{
		Content += FString::Printf(TEXT("\"name\":\"%s\","), *Name);
	}

	if (!Avatar.IsEmpty())
	{
		Content += FString::Printf(TEXT("\"avatar\":\"%s\","), *Avatar);
	}

	if (ChannelId != 0)
	{
		Content += FString::Printf(TEXT("\"channel_id\":%d,"), ChannelId);
	}

	if (Content.Len() <= 1)
	{
		return TEXT("");
	}
	
	Content[Content.Len() - 1] = TEXT('}');

	return Content;
}

FString NWebhookHelper::MakeUrl(const FString& RelativeRoute)
{
	return DiscordWebhookUrlRoot / RelativeRoute;
}

TSharedPtr<FJsonObject> NWebhookHelper::ParseJson(const FString& Json)
{
	TSharedPtr<FJsonObject> Object;

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Json);
	FJsonSerializer::Deserialize(Reader, Object);

	return Object;
}

TArray<TSharedPtr<FJsonValue>> NWebhookHelper::ParseJsonArray(const FString& Json)
{
	TArray<TSharedPtr<FJsonValue>> Objects;

	TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(Json);
	FJsonSerializer::Deserialize(Reader, Objects);

	return Objects;
}

TArray<EDiscordWebhookUserFlag> NWebhookHelper::MakeUserFlags(const int64& Flags)
{
	TArray<EDiscordWebhookUserFlag> FlagsArr;

	if (Flags == 0)
	{
		FlagsArr.Push(EDiscordWebhookUserFlag::None);
		return FlagsArr;
	}
	
	const auto TestUserFlag = [&](const int64& Expr, const EDiscordWebhookUserFlag& Val) -> void
	{
		if (Flags & ((int64)1 << Expr))
		{
			FlagsArr.Push(Val);
		}
	};

	TestUserFlag(0x00, EDiscordWebhookUserFlag::DiscordEmployee);
	TestUserFlag(0x01, EDiscordWebhookUserFlag::PartneredServerOwner);
	TestUserFlag(0x02, EDiscordWebhookUserFlag::HypeSquadEvents);
	TestUserFlag(0x03, EDiscordWebhookUserFlag::BugHunterLevel1);
	TestUserFlag(0x06, EDiscordWebhookUserFlag::HouseBravery);
	TestUserFlag(0x07, EDiscordWebhookUserFlag::HouseBrilliance);
	TestUserFlag(0x08, EDiscordWebhookUserFlag::HouseBalance);
	TestUserFlag(0x09, EDiscordWebhookUserFlag::EarlySupporter);
	TestUserFlag(0x0A, EDiscordWebhookUserFlag::TeamUser);
	TestUserFlag(0x0C, EDiscordWebhookUserFlag::System);
	TestUserFlag(0x0E, EDiscordWebhookUserFlag::BugHunterLevel2);
	TestUserFlag(0x10, EDiscordWebhookUserFlag::VerifiedBot);
	TestUserFlag(0x11, EDiscordWebhookUserFlag::EarlyVerifiedBotDeveloper);

	return FlagsArr;
}

#define ENSURE_FIELD(Expr, What, Miss) ensureMsgf(Expr, TEXT(#What) TEXT(" is missing the field \"") TEXT(#Miss) TEXT("\" which is a mandatory field."));

void NWebhookHelper::JsonObjectToWebhook(const TSharedPtr<class FJsonObject>& Object, FDiscordWebhook& OutWebhook)
{
	int32 WebhookType = 0;

	TSharedPtr<FJsonObject> UserObject;
	const TSharedPtr<FJsonObject>* WebhookUserPtr = &UserObject;

	ENSURE_FIELD(Object->TryGetNumberField(TEXT("id"),			OutWebhook.Id),			"Webhook object", "id");
	ENSURE_FIELD(Object->TryGetNumberField(TEXT("type"),		WebhookType),			"Webhook object", "type");
				 Object->TryGetNumberField(TEXT("guild_id"),	OutWebhook.GuildId);
	ENSURE_FIELD(Object->TryGetNumberField(TEXT("channel_id"),	OutWebhook.ChannelId),	"Webhook object", "channel_id");
				 Object->TryGetObjectField(TEXT("user"),		WebhookUserPtr);
				 Object->TryGetStringField(TEXT("name"),		OutWebhook.Name);
				 Object->TryGetStringField(TEXT("avatar"),		OutWebhook.Avatar);
				 Object->TryGetStringField(TEXT("token"),		OutWebhook.Token);

	if (UserObject.IsValid())
	{
		JsonObjectToUser(UserObject, OutWebhook.User);
	}

	OutWebhook.Type = static_cast<EDiscordWebhookType>(WebhookType);
}

void NWebhookHelper::JsonObjectToUser(const TSharedPtr<FJsonObject>& Object, FDiscordWebhookUser& OutUser)
{
	int64 UserFlags = -1;
	int64 UserPublicFlags = -1;
	int32 PremiumType = 0;

	ENSURE_FIELD(Object->TryGetNumberField(TEXT("id"),				OutUser.Id),			"Webhook User Data", "id");
	ENSURE_FIELD(Object->TryGetStringField(TEXT("username"),		OutUser.Username),		"Webhook User Data", "username");
	ENSURE_FIELD(Object->TryGetStringField(TEXT("discriminator"),	OutUser.Discriminator), "Webhook User Data", "discriminator");
	ENSURE_FIELD(Object->TryGetStringField(TEXT("avatar"),			OutUser.Avatar),		"Webhook User Data", "avatar");
				 Object->TryGetBoolField  (TEXT("bot"),				OutUser.bBot);
				 Object->TryGetBoolField  (TEXT("system"),			OutUser.bSystem);
				 Object->TryGetBoolField  (TEXT("mfa_enabled"),		OutUser.bMfaEnabled);
				 Object->TryGetStringField(TEXT("locale"),			OutUser.Locale);
				 Object->TryGetBoolField  (TEXT("verified"),		OutUser.bVerified);
				 Object->TryGetStringField(TEXT("email"),			OutUser.Email);
				 Object->TryGetNumberField(TEXT("premium_type"),	PremiumType);

	if (Object->TryGetNumberField(TEXT("flags"), UserFlags))
	{
		OutUser.Flags = MakeUserFlags(UserFlags);
	}

	if (Object->TryGetNumberField(TEXT("public_flags"), UserPublicFlags))
	{
		OutUser.PublicFlags = MakeUserFlags(UserPublicFlags);
	}
}

#undef ENSURE_FIELD

void FDiscordWebhookLibrary::CreateWebhook(const FString& Name, const FString& ImageData, const int64& ChannelId, const FDiscordCreateWebhook& OnWebhookCreated)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Webhook creation request: { Name: %s, ChannelId: %d }."), *Name, ChannelId);

	if (Name.Len() > DiscordWebhookNameMaxLength)
	{
		UE_LOG(LogDiscordWebhook, Warning, TEXT("Webhook name exceed the max number of characters (%d). Webhook name will be truncated."), DiscordWebhookNameMaxLength);
	}

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/channels/%lld/webhooks"), ChannelId),
		TEXT("POST"),
		FString::Printf(TEXT("{\"name\":\"%s\",\"avatar\":\"%s\"}"), *Name, *ImageData),
		true,
		[OnWebhookCreated](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			FDiscordWebhook Webhook;

			const bool bCreateSuccess = bSuccess && [&]() -> bool
			{
				TSharedPtr<FJsonObject> RootObject = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!RootObject.IsValid())
				{
					UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to create Webhook. Server response: %s"), *Response->GetContentAsString());
					return false;
				}

				{
					int32 Code = -1;
					FString Message;
					const bool bHasErrors = RootObject->TryGetNumberField(TEXT("code"), Code) && RootObject->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to create Webhook. Server message: %s. Code: %d."), *Message, Code);
						return false;
					}
				}

				NWebhookHelper::JsonObjectToWebhook(RootObject, Webhook);

				UE_LOG(LogDiscordWebhook, Log, TEXT("New webhook with name \"%s\" created."), *Webhook.Name);

				return true;
			}();

			if (!bSuccess)
			{
				UE_LOG(LogDiscordWebhook, Error, TEXT("Webhook server didn't respond."));
			}

			OnWebhookCreated.ExecuteIfBound(bCreateSuccess, Webhook);
		}
	);
}

void FDiscordWebhookLibrary::GetChannelWebhooks(const int64& ChannelId, const FDiscordGetWebhooks& OnChannelWebhooksReceived)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server Webhooks for the Channel %d."), ChannelId);

	NWebhookHelper::SendWebhookRequest
	(	
		FString::Printf(TEXT("/channels/%lld/webhooks"), ChannelId),
		TEXT("GET"),
		TEXT(""),
		true,
		[OnChannelWebhooksReceived](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			TArray<FDiscordWebhook> Webhooks;

			const bool bGetSucces = bSuccess && [&]() -> bool
			{
				const FString ResponseContent = Response->GetContentAsString();

				// We expect an array. Something else means an error occured.
				if (ResponseContent.IsEmpty() || ResponseContent[0] != TEXT('['))
				{
					TSharedPtr<FJsonObject> Object = NWebhookHelper::ParseJson(ResponseContent);

					int32	Code = -1;
					FString Message;

					const bool bHasErrors = Object->TryGetNumberField(TEXT("code"), Code) | Object->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Channel Webhooks. Server message: %s. Code: %d."), *Message, Code);
					}
					else
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Server sent invalid data for Guild Webhooks: %s."), *ResponseContent);
					}

					return false;
				}

				TArray<TSharedPtr<FJsonValue>> WebhooksValues = NWebhookHelper::ParseJsonArray(ResponseContent);

				for (const TSharedPtr<FJsonValue>& WebhookValue : WebhooksValues)
				{
					if (WebhookValue->Type != EJson::Object)
					{
						continue;
					}

					FDiscordWebhook Webhook;

					NWebhookHelper::JsonObjectToWebhook(WebhookValue->AsObject(), Webhook);

					Webhooks.Push(Webhook);
				}

				UE_LOG(LogDiscordWebhook, Log, TEXT("Received %d Webhook(s)."), Webhooks.Num());

				return true;
			}();

			OnChannelWebhooksReceived.ExecuteIfBound(bGetSucces, Webhooks);
		}
	);
}

void FDiscordWebhookLibrary::GetGuildWebhooks(const int64& ChannelId, const FDiscordGetWebhooks& OnChannelWebhooksReceived)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server Webhooks for the Guild %d."), ChannelId);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/guilds/%lld/webhooks"), ChannelId),
		TEXT("GET"),
		TEXT(""),
		true,
		[OnChannelWebhooksReceived](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			TArray<FDiscordWebhook> Webhooks;

			const bool bGetSucces = bSuccess && [&]() -> bool
			{
				const FString ResponseContent = Response->GetContentAsString();

				// We expect an array. Something else means an error occured.
				if (ResponseContent.IsEmpty() || ResponseContent[0] != TEXT('['))
				{
					TSharedPtr<FJsonObject> Object = NWebhookHelper::ParseJson(ResponseContent);

					int32	Code = -1;
					FString Message;

					const bool bHasErrors = Object->TryGetNumberField(TEXT("code"), Code) && Object->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Guild Webhooks. Server message: %s. Code: %d."), *Message, Code);
					}
					else
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Server sent invalid data for Guild Webhooks: %s."), *ResponseContent);
					}

					return false;
				}

				TArray<TSharedPtr<FJsonValue>> WebhooksValues = NWebhookHelper::ParseJsonArray(Response->GetContentAsString());

				for (const TSharedPtr<FJsonValue>& WebhookValue : WebhooksValues)
				{
					if (WebhookValue->Type != EJson::Object)
					{
						continue;
					}

					FDiscordWebhook Webhook;

					NWebhookHelper::JsonObjectToWebhook(WebhookValue->AsObject(), Webhook);

					Webhooks.Push(Webhook);
				}

				UE_LOG(LogDiscordWebhook, Log, TEXT("Received %d Webhook(s)."), Webhooks.Num());

				return true;
			}();

			OnChannelWebhooksReceived.ExecuteIfBound(bGetSucces, Webhooks);
		}
	);
}

void FDiscordWebhookLibrary::GetWebhook(const int64& WebhookId, const FDiscordGetWebhook& OnWebhookReceived)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server Webhook with ID %lld."), WebhookId);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld"), WebhookId),
		TEXT("GET"),
		TEXT(""),
		true,
		[OnWebhookReceived](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			FDiscordWebhook Webhook;

			const bool bCreateSuccess = bSuccess && [&]() -> bool
			{
				TSharedPtr<FJsonObject> RootObject = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!RootObject.IsValid())
				{
					UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Webhook. Server response: %s"), *Response->GetContentAsString());
					return false;
				}

				{
					int32 Code = -1;
					FString Message;
					const bool bHasErrors = RootObject->TryGetNumberField(TEXT("code"), Code) && RootObject->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Webhook. Server message: %s. Code: %d."), *Message, Code);
						return false;
					}
				}

				NWebhookHelper::JsonObjectToWebhook(RootObject, Webhook);

				UE_LOG(LogDiscordWebhook, Log, TEXT("Webhook with name \"%s\" received."), *Webhook.Name);

				return true;
			}();

			if (!bSuccess)
			{
				UE_LOG(LogDiscordWebhook, Error, TEXT("Webhook server didn't respond."));
			}

			OnWebhookReceived.ExecuteIfBound(bCreateSuccess, Webhook);
		}
	);
}

void FDiscordWebhookLibrary::GetWebhookWithToken(const int64& WebhookId, const FString& Token, const FDiscordGetWebhook& OnWebhookReceived)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server Webhook with ID %d using an authorization token."), WebhookId);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld/%s"), WebhookId, *Token),
		TEXT("GET"),
		TEXT(""),
		false,
		[OnWebhookReceived](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			FDiscordWebhook Webhook;

			const bool bCreateSuccess = bSuccess && [&]() -> bool
			{
				TSharedPtr<FJsonObject> RootObject = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!RootObject.IsValid())
				{
					UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Webhook. Server response: %s"), *Response->GetContentAsString());
					return false;
				}

				{
					int32 Code = -1;
					FString Message;
					const bool bHasErrors = RootObject->TryGetNumberField(TEXT("code"), Code) && RootObject->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to get Webhook. Server message: %s. Code: %d."), *Message, Code);
						return false;
					}
				}

				NWebhookHelper::JsonObjectToWebhook(RootObject, Webhook);

				UE_LOG(LogDiscordWebhook, Log, TEXT("Webhook with name \"%s\" received."), *Webhook.Name);

				return true;
			}();

			if (!bSuccess)
			{
				UE_LOG(LogDiscordWebhook, Error, TEXT("Webhook server didn't respond."));
			}

			OnWebhookReceived.ExecuteIfBound(bCreateSuccess, Webhook);
		}
	);
}

void FDiscordWebhookLibrary::ModifyWebhook(const int64 WebhookId, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId, const FDiscordGetWebhook& OnWebhookUpdated)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server to update Webhook with ID %d."), WebhookId);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld"), WebhookId),
		TEXT("PATCH"),
		NWebhookHelper::BuildJsonForModify(NewName, NewAvatar, NewChannelId),
		true,
		[OnWebhookUpdated](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			FDiscordWebhook Webhook;

			const bool bCreateSuccess = bSuccess && [&]() -> bool
			{
				TSharedPtr<FJsonObject> RootObject = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!RootObject.IsValid())
				{
					UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to update Webhook. Server response: %s"), *Response->GetContentAsString());
					return false;
				}

				{
					int32 Code = -1;
					FString Message;
					const bool bHasErrors = RootObject->TryGetNumberField(TEXT("code"), Code) && RootObject->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to update Webhook. Server message: %s. Code: %d."), *Message, Code);
						return false;
					}
				}

				NWebhookHelper::JsonObjectToWebhook(RootObject, Webhook);

				UE_LOG(LogDiscordWebhook, Log, TEXT("Webhook with name \"%s\" updated."), *Webhook.Name);

				return true;
			}();

			if (!bSuccess)
			{
				UE_LOG(LogDiscordWebhook, Error, TEXT("Webhook server didn't respond."));
			}

			OnWebhookUpdated.ExecuteIfBound(bCreateSuccess, Webhook);
		}
	);
}

void FDiscordWebhookLibrary::ModifyWebhookWithToken(const int64 WebhookId, const FString& Token, const FString& NewName, const FString& NewAvatar, const int64 NewChannelId, const FDiscordGetWebhook& OnWebhookUpdated)
{
	UE_LOG(LogDiscordWebhook, Log, TEXT("Asking Discord Server to update Webhook with ID %d using an authorization token."), WebhookId);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld/%s"), WebhookId, *Token),
		TEXT("PATCH"),
		NWebhookHelper::BuildJsonForModify(NewName, NewAvatar, NewChannelId),
		false,
		[OnWebhookUpdated](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			FDiscordWebhook Webhook;

			const bool bCreateSuccess = bSuccess && [&]() -> bool
			{
				TSharedPtr<FJsonObject> RootObject = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!RootObject.IsValid())
				{
					UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to update Webhook. Server response: %s"), *Response->GetContentAsString());
					return false;
				}

				{
					int32 Code = -1;
					FString Message;
					const bool bHasErrors = RootObject->TryGetNumberField(TEXT("code"), Code) && RootObject->TryGetStringField(TEXT("message"), Message);

					if (bHasErrors)
					{
						UE_LOG(LogDiscordWebhook, Error, TEXT("Failed to update Webhook. Server message: %s. Code: %d."), *Message, Code);
						return false;
					}
				}

				NWebhookHelper::JsonObjectToWebhook(RootObject, Webhook);

				UE_LOG(LogDiscordWebhook, Log, TEXT("Webhook with name \"%s\" updated."), *Webhook.Name);

				return true;
			}();

			if (!bSuccess)
			{
				UE_LOG(LogDiscordWebhook, Error, TEXT("Webhook server didn't respond."));
			}

			OnWebhookUpdated.ExecuteIfBound(bCreateSuccess, Webhook);
		}
	);
}

void FDiscordWebhookLibrary::DeleteWebhook(const int64 WebhookId, const FDiscordDeleteWebhook& OnWebhookDeleted)
{
	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld"), WebhookId),
		TEXT("DELETE"),
		TEXT(""),
		true,
		[OnWebhookDeleted](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			const bool bParseSuccess = bSuccess && [&]() -> bool
			{
				const TSharedPtr<FJsonObject> Object = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!Object.IsValid())
				{
					return false;
				}

				if (Response->GetResponseCode() != 204)
				{
					UE_LOG(LogDiscordWebhook, Warning, TEXT("Failed to delete Webhook. Code: %d. Response: %s."), Response->GetResponseCode(), *Response->GetContentAsString());
					return false;
				}

				return true;
			}();

			OnWebhookDeleted.ExecuteIfBound(bParseSuccess);
		}
	);
}

void FDiscordWebhookLibrary::DeleteWebhookWithToken(const int64 WebhookId, const FString& Token, const FDiscordDeleteWebhook& OnWebhookDeleted)
{
	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld/%s"), WebhookId, *Token),
		TEXT("DELETE"),
		TEXT(""),
		true,
		[OnWebhookDeleted](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			const bool bParseSuccess = bSuccess && [&]() -> bool
			{
				const TSharedPtr<FJsonObject> Object = NWebhookHelper::ParseJson(Response->GetContentAsString());

				if (!Object.IsValid())
				{
					return false;
				}

				if (Response->GetResponseCode() != 204)
				{
					UE_LOG(LogDiscordWebhook, Warning, TEXT("Failed to delete Webhook. Code: %d. Response: %s."), Response->GetResponseCode(), *Response->GetContentAsString());
					return false;
				}

				return true;
			}();

			OnWebhookDeleted.ExecuteIfBound(bParseSuccess);
		}
	);
}

void FDiscordWebhookLibrary::ExecuteWebhook(const int64 WebhookId, const FString& WebhookToken, const FDiscordWebhookData& WebhookData, const FDiscordExecuteWebhook& OnWebhookExecuted)
{
	TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();

	if (!WebhookData.Content .IsEmpty()) RootObject->SetStringField(TEXT("content"),  WebhookData.Content);
	if (!WebhookData.Username.IsEmpty()) RootObject->SetStringField(TEXT("username"), WebhookData.Username);
	if (!WebhookData.Avatar  .IsEmpty()) RootObject->SetStringField(TEXT("username"), WebhookData.Avatar);
	if (!WebhookData.File	 .IsEmpty()) RootObject->SetStringField(TEXT("file"),	  WebhookData.File);

	RootObject->SetBoolField(TEXT("tts"), WebhookData.bTts);

	TArray<TSharedPtr<FJsonValue> > Embeds;

	for (const FDiscordWebhookEmbed& Embed : WebhookData.Embeds)
	{
		TSharedPtr<FJsonObject> EmbedObject = MakeShared<FJsonObject>();

		if (!Embed.Title	  .IsEmpty()) EmbedObject->SetStringField(TEXT("title"),		Embed.Title);
		if (!Embed.Type		  .IsEmpty()) EmbedObject->SetStringField(TEXT("type"),			Embed.Type);
		if (!Embed.Description.IsEmpty()) EmbedObject->SetStringField(TEXT("description"),  Embed.Description);
		if (!Embed.Url		  .IsEmpty()) EmbedObject->SetStringField(TEXT("url"),			Embed.Url);
		if (!Embed.Timestamp  .IsEmpty()) EmbedObject->SetStringField(TEXT("timestamp"),	Embed.Timestamp);
		
		EmbedObject->SetNumberField(TEXT("color"),	Embed.Color);

		{
			TSharedPtr<FJsonObject> Footer = MakeShared<FJsonObject>();

			if (Embed.Footer.Text		 .IsEmpty())	Footer->SetStringField(TEXT("text"),			Embed.Footer.Text);
			if (Embed.Footer.IconUrl	 .IsEmpty())	Footer->SetStringField(TEXT("icon_url"),		Embed.Footer.IconUrl);
			if (Embed.Footer.ProxyIconUrl.IsEmpty())	Footer->SetStringField(TEXT("proxy_icon_url"),	Embed.Footer.ProxyIconUrl);

			if (Footer->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("footer"), Footer);
			}
		}

		{
			TSharedPtr<FJsonObject> Image = MakeShared<FJsonObject>();

			if (!Embed.Image.Url		.IsEmpty())	Image->SetStringField(TEXT("url"),			Embed.Image.Url);
			if (!Embed.Image.ProxyUrl.IsEmpty())	Image->SetStringField(TEXT("proxy_url"),	Embed.Image.ProxyUrl);
			if (Embed.Image.Height > 0)				Image->SetNumberField(TEXT("height"),		Embed.Image.Height);
			if (Embed.Image.Width  > 0)				Image->SetNumberField(TEXT("width"),		Embed.Image.Width);

			if (Image->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("image"), Image);
			}
		}

		{
			TSharedPtr<FJsonObject> Thumbnail = MakeShared<FJsonObject>();

			if (!Embed.Thumbnail.Url	 .IsEmpty()) Thumbnail->SetStringField(TEXT("url"),			Embed.Image.Url);
			if (!Embed.Thumbnail.ProxyUrl.IsEmpty()) Thumbnail->SetStringField(TEXT("proxy_url"),	Embed.Image.ProxyUrl);
			if (Embed.Thumbnail.Height > 0)			 Thumbnail->SetNumberField(TEXT("height"),		Embed.Image.Height);
			if (Embed.Thumbnail.Width  > 0)			 Thumbnail->SetNumberField(TEXT("width"),		Embed.Image.Width);

			if (Thumbnail->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("thumbnail"), Thumbnail);
			}
		}

		{
			TSharedPtr<FJsonObject> Video = MakeShared<FJsonObject>();
			
			if (!Embed.Video.Url.IsEmpty()) Video->SetStringField(TEXT("url"),			Embed.Video.Url);
			if (Embed.Video.Height > 0)		Video->SetNumberField(TEXT("height"),		Embed.Video.Height);
			if (Embed.Video.Width  > 0)		Video->SetNumberField(TEXT("width"),		Embed.Video.Width);

			if (Video->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("video"), Video);
			}
		}

		{
			TSharedPtr<FJsonObject> Provider = MakeShared<FJsonObject>();

			if (!Embed.Provider.Url .IsEmpty()) Provider->SetStringField(TEXT("url"),	Embed.Provider.Url);
			if (!Embed.Provider.Name.IsEmpty()) Provider->SetStringField(TEXT("name"),	Embed.Provider.Name);

			if (Provider->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("provider"), Provider);
			}
		}

		{
			TSharedPtr<FJsonObject> Author = MakeShared<FJsonObject>();

			if (!Embed.Author.IconUrl	  .IsEmpty()) Author->SetStringField(TEXT("icon_url"),		 Embed.Author.IconUrl);
			if (!Embed.Author.Name		  .IsEmpty()) Author->SetStringField(TEXT("name"),			 Embed.Author.Name);
			if (!Embed.Author.ProxyIconUrl.IsEmpty()) Author->SetStringField(TEXT("proxy_icon_url"), Embed.Author.ProxyIconUrl);
			if (!Embed.Author.Url		  .IsEmpty()) Author->SetStringField(TEXT("url"),			 Embed.Author.Url);

			if (Author->Values.Num() > 0)
			{
				EmbedObject->SetObjectField(TEXT("author"), Author);
			}
		}

		{
			TArray<TSharedPtr<FJsonValue> > Fields;

			for (const FDiscordWebhookEmbedField& Field : Embed.Field)
			{
				TSharedPtr<FJsonObject> FieldObject = MakeShared<FJsonObject>();

				FieldObject->SetStringField(TEXT("name"),   Field.Name);
				FieldObject->SetStringField(TEXT("value"),  Field.Value);
				FieldObject->SetBoolField  (TEXT("inline"), Field.bInline);

				Fields.Push(MakeShared<FJsonValueObject>(FieldObject));
			}

			EmbedObject->SetArrayField(TEXT("fields"), Fields);
		}
		
		Embeds.Push(MakeShared<FJsonValueObject>(EmbedObject));
	}

	RootObject->SetArrayField(TEXT("embeds"), Embeds);

	TSharedPtr<FJsonObject> AllowedMentions = MakeShared<FJsonObject>();

	{
		TArray<TSharedPtr<FJsonValue>> Parse;

		for (const EDiscordAllowedMentionTypes& Mention : WebhookData.AllowedMentions.Parse)
		{
			FString MentionStr;

			switch (Mention)
			{
			case EDiscordAllowedMentionTypes::EveryoneMentions:
				MentionStr = TEXT("everyone");
				break;
			case EDiscordAllowedMentionTypes::RoleMentions:
				MentionStr = TEXT("roles");
				break;
			case EDiscordAllowedMentionTypes::UserMentions:
				MentionStr = TEXT("users");
				break;
			default:
				break;
			}

			Parse.Push(MakeShared<FJsonValueString>(MentionStr));
		}
		
		AllowedMentions->SetArrayField(TEXT("parse"), Parse);
	}

	{
		TArray<TSharedPtr<FJsonValue>> Roles;

		for (const int64& Snowflake : WebhookData.AllowedMentions.Roles)
		{
			Roles.Push(MakeShared<FJsonValueNumber>(Snowflake));
		}

		AllowedMentions->SetArrayField(TEXT("roles"), Roles);
	}

	{
		TArray<TSharedPtr<FJsonValue>> Users;

		for (const int64& Snowflake : WebhookData.AllowedMentions.Users)
		{
			Users.Push(MakeShared<FJsonValueNumber>(Snowflake));
		}

		AllowedMentions->SetArrayField(TEXT("roles"), Users);
	}

	RootObject->SetObjectField(TEXT("allowed_mentions"), AllowedMentions);

	FString StringifiedJson;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR> > > Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&StringifiedJson);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	NWebhookHelper::SendWebhookRequest
	(
		FString::Printf(TEXT("/webhooks/%lld/%s?wait=%s"), WebhookId, *WebhookToken, OnWebhookExecuted.IsBound() ? TEXT("true") : TEXT("false")),
		TEXT("POST"),
		StringifiedJson,
		true,
		[OnWebhookExecuted](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess) -> void
		{
			const bool bExecuteSuccess = bSuccess && [&]()->bool
			{
				TSharedPtr<FJsonObject> Object = NWebhookHelper::ParseJson(Response->GetContentAsString());

				int32 Code = -1;
				FString Message;
				if (Object->TryGetStringField(TEXT("message"), Message) && Object->TryGetNumberField(TEXT("code"), Code))
				{
					UE_LOG(LogDiscordWebhook, Warning, TEXT("Failed to execute Webhook. Code: %d. Response: %s."), Code, *Message);
					return false;
				}
				return true;
			}();

			OnWebhookExecuted.ExecuteIfBound(bExecuteSuccess);
		}
	);
}

