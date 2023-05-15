// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordOverlayManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordOverlayNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOverlayResult, const EDiscordResult, Result);

UCLASS(Abstract)
class UBaseOverlayAsyncProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FOverlayResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FOverlayResult OnError;

protected:
	UPROPERTY()
	UDiscordOverlayManager* Manager;

	FDiscordResultCallback CreateCallback();

	void OnResult(EDiscordResult Result);

};

UCLASS()
class USetLockedProxy : public UBaseOverlayAsyncProxy
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Overlay", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Set Locked"))
	static USetLockedProxy* SetLocked(UDiscordOverlayManager* OverlayManager, const bool bLocked);

	virtual void Activate();

private:
	bool bLocked;
};

UCLASS()
class UOpenActivityInviteProxy : public UBaseOverlayAsyncProxy
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "Discord|Overlay", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Open Activity Invite"))
	static UOpenActivityInviteProxy* OpenActivityInvite(UDiscordOverlayManager* OverlayManager, const EDiscordActivityActionType Type);

	virtual void Activate();

private:
	EDiscordActivityActionType Type;
};

UCLASS()
class UOpenGuildInviteProxy : public UBaseOverlayAsyncProxy
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Overlay", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Open Guild Invite"))
	static UOpenGuildInviteProxy* OpenGuildInvite(UDiscordOverlayManager* OverlayManager, const FString& Code);

	virtual void Activate();

private:
	FString Code;
};

UCLASS()
class UOpenVoiceSettingsProxy : public UBaseOverlayAsyncProxy
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Discord|Overlay", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Open Voice Settings"))
	static UOpenVoiceSettingsProxy* OpenVoiceSettings(UDiscordOverlayManager* OverlayManager);

	virtual void Activate();

private:
	FString Code;
};




