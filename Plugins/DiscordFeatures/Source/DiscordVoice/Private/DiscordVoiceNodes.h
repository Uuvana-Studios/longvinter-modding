// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "DiscordVoiceManager.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "DiscordVoiceNodes.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetInputModeEvent, EDiscordResult, Result);

UCLASS()
class UVoiceSetInputModeProxy : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FSetInputModeEvent OnInputModeSet;

	UPROPERTY(BlueprintAssignable)
	FSetInputModeEvent OnError;

	virtual void Activate()
	{
		if (!Manager)
		{
			FFrame::KismetExecutionMessage(TEXT("Set input mode failed. Passed nullptr for Manager."), ELogVerbosity::Error);
			OnResultInternal(EDiscordResult::InternalError);
			return;
		}

		Manager->SetInputMode(InputMode, FDiscordResultCallback::CreateUObject(this, &UVoiceSetInputModeProxy::OnResultInternal));
	}

	UFUNCTION(BlueprintCallable, Category = "Discord|Voice", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Set Input Mode"))
	static UVoiceSetInputModeProxy* SetInputMode(UDiscordVoiceManager* VoiceManager, const FDiscordInputMode& NewInputMode)
	{
		UVoiceSetInputModeProxy* const Proxy = NewObject<UVoiceSetInputModeProxy>();

		Proxy->Manager   = VoiceManager;
		Proxy->InputMode = NewInputMode;

		return Proxy;
	}

private:
	UPROPERTY()
	UDiscordVoiceManager* Manager;

	FDiscordInputMode InputMode;

	void OnResultInternal(EDiscordResult Result)
	{
		(Result == EDiscordResult::Ok ? OnInputModeSet : OnError)
			.Broadcast(Result);
		SetReadyToDestroy();
	}
};

