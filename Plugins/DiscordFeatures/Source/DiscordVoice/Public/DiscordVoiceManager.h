// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordVoiceManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDiscordOnVoiceSettingsUpdate);

UENUM(BlueprintType)
enum class EDiscordInputModeType : uint8
{
	VoiceActivity = 0,
	PushToTalk,
};

USTRUCT(BlueprintType)
struct FDiscordInputMode
{
	GENERATED_BODY()
public:
	// Set either VAD or PTT as the voice input mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Voice")
	EDiscordInputModeType Type;

	// The PTT hotkey for the user
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord|Voice")
	FString Shortcut;
};

UCLASS()
class DISCORDVOICE_API UDiscordVoiceManager : public UDiscordManager
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FDiscordOnVoiceSettingsUpdate OnVoiceSettingsUpdate;

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice", BlueprintPure, meta = (CompactNodeTitle = "VOICE MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Voice Manager") UDiscordVoiceManager* GetVoiceManager(UDiscordCore* DiscordCore);

	// Get the current voice input mode for the user.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Voice")
	UPARAM(DisplayName = "Input Mode") FDiscordInputMode GetInputMode();

	void SetInputMode(const FDiscordInputMode& NewInputMode, const FDiscordResultCallback& OnResult);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Voice")
	UPARAM(DisplayName = "Is Self Mute") bool IsSelfMute();

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice")
	void SetSelfMute(const bool bMute);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Voice")
	UPARAM(DisplayName = "Is Self Deaf") bool IsSelfDeaf();

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice")
	void SetSelfDeaf(const bool bDeaf);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Voice")
	UPARAM(DisplayName = "Is Mute") bool IsLocalMute(const int64 UserId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice")
	void SetLocalMute(const int64 UserId, const bool bMute);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Voice")
	UPARAM(DisplayName = "Volume") uint8 GetLocalVolume(const int64 UserId);

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice")
	void SetLocalVolume(const int64 UserId, const uint8 Volume);

private:
	UFUNCTION()
	void OnVoiceSettingsUpdateInternal();

};

