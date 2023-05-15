// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordCore.h"
#include "DiscordManager.h"
#include "DiscordOverlayManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDiscordOverlayToggleEvent, const bool, bLocked);

UCLASS()
class DISCORDOVERLAY_API UDiscordOverlayManager : public UDiscordManager
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FDiscordOverlayToggleEvent OnOverlayToggle;

public:
	UFUNCTION(BlueprintCallable, Category = "Discord|Overlay", BlueprintPure, meta = (CompactNodeTitle = "OVERLAY MANAGER", BlueprintAutoCast))
	static UPARAM(DisplayName = "Overlay Manager") UDiscordOverlayManager* GetOverlayManager(UDiscordCore* DiscordCore);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Overlay")
	UPARAM(DisplayName = "Enabled") bool IsEnabled();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Discord|Overlay")
	UPARAM(DisplayName = "Locked") bool IsLocked();

	void SetLocked(const bool bLocked, const FDiscordResultCallback& Callback);

	void OpenActivityInvite(const EDiscordActivityActionType Type, const FDiscordResultCallback& Callback);

	void OpenGuildInvite(const FString& Code, const FDiscordResultCallback& Callback);

	void OpenVoiceSettings(const FDiscordResultCallback& Callback);

private:
	void OnOverlayToggleInternal(const bool bLocked);

};
