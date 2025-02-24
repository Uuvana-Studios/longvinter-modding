﻿// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UGC_Workshop_Component.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LONGVINTER_API UUGC_Workshop_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUGC_Workshop_Component();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "")
	void GetInstalledMods(TArray<FString> &Mods, TArray<FString> &SteamMods);

	UFUNCTION(BlueprintCallable, Category = "")
	void copyMods(bool CopyRootOnly, bool SkipMount);

	UFUNCTION(BlueprintCallable, Category = "")
	bool CheckModFolderSync(bool Server, FString& hash1_s, FString& hash2_s);
};
