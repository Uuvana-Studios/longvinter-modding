// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DiscordCore.h"

namespace FDiscordManagerUtils
{
	template<class T>
	void CopyStringToBuffer(const FString& From, T&To)
	{
		ANSICHAR* Utf8 = TCHAR_TO_UTF8(*From);
		FMemory::Memzero(To);
		FMemory::Memcpy(To, Utf8, FMath::Min<int32>(static_cast<int32>(sizeof(To)), TCString<ANSICHAR>::Strlen(Utf8)));
	}

	void DISCORDCORE_API DiscordResultCallback_AutoDelete(void* Data, FRawDiscord::EDiscordResult Result);
};

#define DISCORD_CALL_RAW_CHECKED(Module, Func, ...) do { \
	if (DiscordCore.IsValid() && DiscordCore->App. Module ) \
	{ \
		DiscordCore->App. Module -> Func (DiscordCore->App. Module , __VA_ARGS__); \
	} \
} while(0)

#define DISCORD_CALL_RAW_CHECKED_Ret(Module, Func, ...) ( \
	DiscordCore.IsValid() && DiscordCore->App. Module ? \
	DiscordCore->App. Module -> Func (DiscordCore->App. Module , __VA_ARGS__) : FRawDiscord::EDiscordResult::InvalidManager \
)

#define ACHIEVEMENT_CALL_RAW_CHECKED( Func, ...)	DISCORD_CALL_RAW_CHECKED(achievements,	Func, __VA_ARGS__)
#define ACTIVITY_CALL_RAW_CHECKED(	  Func, ...)	DISCORD_CALL_RAW_CHECKED(activities,	Func, __VA_ARGS__)
#define APPLICATION_CALL_RAW_CHECKED( Func, ...)	DISCORD_CALL_RAW_CHECKED(application,	Func, __VA_ARGS__)
#define NETWORK_CALL_RAW_CHECKED(	  Func, ...)	DISCORD_CALL_RAW_CHECKED(network,		Func, __VA_ARGS__)
#define NETWORK_CALL_RAW_CHECKED_Ret( Func, ...)	DISCORD_CALL_RAW_CHECKED_Ret(network,	Func, __VA_ARGS__)
#define VOICE_CALL_RAW_CHECKED(		  Func, ...)	DISCORD_CALL_RAW_CHECKED(voice,			Func, __VA_ARGS__)
#define RELATIONSHIP_CALL_RAW_CHECKED(Func, ...)	DISCORD_CALL_RAW_CHECKED(relationships,	Func, __VA_ARGS__)
#define STORAGE_CALL_RAW_CHECKED(	  Func, ...)	DISCORD_CALL_RAW_CHECKED(storage,		Func, __VA_ARGS__)
#define STORE_CALL_RAW_CHECKED(		  Func, ...)	DISCORD_CALL_RAW_CHECKED(store,			Func, __VA_ARGS__)
#define USER_CALL_RAW_CHECKED(		  Func, ...)	DISCORD_CALL_RAW_CHECKED(users,			Func, __VA_ARGS__)
#define IMAGE_CALL_RAW_CHECKED(		  Func, ...)	DISCORD_CALL_RAW_CHECKED(image,			Func, __VA_ARGS__)
#define OVERLAY_CALL_RAW_CHECKED(	  Func, ...)	DISCORD_CALL_RAW_CHECKED(overlay,		Func, __VA_ARGS__)
#define LOBBY_CALL_RAW_CHECKED(		  Func, ...)	DISCORD_CALL_RAW_CHECKED(lobbies,		Func, __VA_ARGS__)
#define LOBBY_CALL_RAW_CHECKED_Ret(	  Func, ...)	DISCORD_CALL_RAW_CHECKED_Ret(lobbies,	Func, __VA_ARGS__)
