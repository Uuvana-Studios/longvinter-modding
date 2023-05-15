// Copyright Pandores Marketplace 2021. All Rights Reserved.

#include "DiscordVoiceManager.h"
#include "DiscordManagerUtils.h"

/* static */ UDiscordVoiceManager* UDiscordVoiceManager::GetVoiceManager(UDiscordCore* const DiscordCore)
{
	if (!DiscordCore)
	{
		return nullptr;
	}

	if (DiscordCore->VoiceManager)
	{
		return Cast<UDiscordVoiceManager>(DiscordCore->VoiceManager);
	}

	UDiscordVoiceManager* const Manager = NewObject<UDiscordVoiceManager>();

	Manager->DiscordCore = DiscordCore;

	DiscordCore->VoiceManager = Manager;
	DiscordCore->VoiceOnSettingsUpdate.BindUObject(Manager, &UDiscordVoiceManager::OnVoiceSettingsUpdateInternal);

	return Manager;
}

FDiscordInputMode UDiscordVoiceManager::GetInputMode()
{
	FRawDiscord::DiscordInputMode RawMode;
	FMemory::Memzero(RawMode);

	VOICE_CALL_RAW_CHECKED(get_input_mode, &RawMode);

	FDiscordInputMode Mode;

	Mode.Shortcut = UTF8_TO_TCHAR(RawMode.shortcut);
	Mode.Type	  = static_cast<EDiscordInputModeType>(RawMode.type);

	return Mode;
}

void UDiscordVoiceManager::SetInputMode(const FDiscordInputMode& NewInputMode, const FDiscordResultCallback& OnResult)
{
	const auto Callback = [](void* Data, FRawDiscord::EDiscordResult Result)
	{
		FDiscordResultCallback* const Cb = (FDiscordResultCallback*)Data;

		Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result));

		delete Cb;
	};

	FRawDiscord::DiscordInputMode Raw;
	FMemory::Memzero(Raw);

	Raw.type = static_cast<FRawDiscord::EDiscordInputModeType>(NewInputMode.Type);
	FDiscordManagerUtils::CopyStringToBuffer(NewInputMode.Shortcut, Raw.shortcut);

	VOICE_CALL_RAW_CHECKED(set_input_mode, Raw, new FDiscordResultCallback(OnResult), Callback);
}

bool UDiscordVoiceManager::IsSelfMute()
{
	bool bMute = false;
	VOICE_CALL_RAW_CHECKED(is_self_mute, &bMute);
	return bMute;
}

void UDiscordVoiceManager::SetSelfMute(const bool bMute)
{
	VOICE_CALL_RAW_CHECKED(set_self_mute, bMute);
}

bool UDiscordVoiceManager::IsSelfDeaf()
{
	bool bDeaf = false;
	VOICE_CALL_RAW_CHECKED(is_self_deaf, &bDeaf);
	return bDeaf;
}

void UDiscordVoiceManager::SetSelfDeaf(const bool bDeaf)
{
	VOICE_CALL_RAW_CHECKED(set_self_deaf, bDeaf);
}

bool UDiscordVoiceManager::IsLocalMute(const int64 UserId)
{
	bool bMute = false;
	VOICE_CALL_RAW_CHECKED(is_local_mute, UserId, &bMute);
	return bMute;
}

void UDiscordVoiceManager::SetLocalMute(const int64 UserId, const bool bMute)
{
	VOICE_CALL_RAW_CHECKED(set_local_mute, UserId, bMute);
}

uint8 UDiscordVoiceManager::GetLocalVolume(const int64 UserId)
{
	uint8 Volume = 0;
	VOICE_CALL_RAW_CHECKED(get_local_volume, UserId, &Volume);
	return Volume;
}

void UDiscordVoiceManager::SetLocalVolume(const int64 UserId, const uint8 Volume)
{
	VOICE_CALL_RAW_CHECKED(set_local_volume, UserId, Volume);
}

void UDiscordVoiceManager::OnVoiceSettingsUpdateInternal()
{
	OnVoiceSettingsUpdate.Broadcast();
}
