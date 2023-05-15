// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ServerHostingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LONGVINTER_API UServerHostingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UServerHostingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "")
	void StartServer();

	UFUNCTION(BlueprintCallable, Category = "")
	void WipeServer();

	UFUNCTION(BlueprintCallable, Category = "")
	void SaveServerVariables(FString ServerName, FString MaxPlayers, FString Password, FString ServerTag, FString Community, FString Admins, FString PVP, FString Decay, FString MaxTents, FString Public, FString CoopPlay, FString CoopSpawn);

	UFUNCTION(BlueprintCallable, Category = "")
	void LoadServerVariables(FString& ServerName, FString& MaxPlayers, FString& Password, FString& ServerTag, FString& Community, FString& Admins, FString& PVP, FString& Decay, FString& MaxTents, FString& Public, FString& CoopPlay, FString& CoopSpawn);
};
