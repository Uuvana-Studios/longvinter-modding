// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Materials/MaterialInterface.h"
#include "WorldCollision.h"

#include "AdvancedFadeObjectsComponent.generated.h"

USTRUCT()
struct FFadeObjStruct
{
	GENERATED_USTRUCT_BODY()

    UPROPERTY()
	UPrimitiveComponent* primitiveComp;

	UPROPERTY()
	TArray<UMaterialInterface*> baseMatInterface;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> fadeMID;

	UPROPERTY()
	float fadeCurrent;

	UPROPERTY()
	bool bToHide;

	void NewElement(UPrimitiveComponent* newComponent, TArray<UMaterialInterface*> newBaseMat,
                    TArray<UMaterialInstanceDynamic*> newMID, float currentFade, bool bHide)
	{
		primitiveComp = newComponent;
		baseMatInterface = newBaseMat;
		fadeMID = newMID;
		fadeCurrent = currentFade;
		bToHide = bHide;
	}

	void SetHideOnly(bool hide)
	{
		bToHide = hide;
	}

	void SetFadeAndHide(float newFade, bool newHide)
	{
		fadeCurrent = newFade;
		bToHide = newHide;
	}

	//For Destroy
	void Destroy()
	{
		primitiveComp = nullptr;
	}

	//Constructor
	FFadeObjStruct()
	{
		primitiveComp = nullptr;
		fadeCurrent = 0;
		bToHide = true;
	}
};

UCLASS(Blueprintable)
class ADVANCEDFADEOBJECTS_API UAdvancedFadeObjectsComponent : public UActorComponent
{
		GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UAdvancedFadeObjectsComponent();

	// Called every frame
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	void BeginPlay() override;

	// Check objects between camera manager and character and add to array for fade
	void AddObjectsToHideTimer();

	// Fade worker.
	void FadeObjWorkerTimer();

	// Enable or disable fade object worker
	UFUNCTION(BlueprintCallable, Category = "Fade Objects")
	void SetEnable(bool setEnable);

	// Pause or unpause fade object worker
	UFUNCTION(BlueprintCallable, Category = "Fade Objects")
    void SetActivate(bool setActivate);

	void SetWantsTrace();

	FTraceHandle RequestTrace();
	void OnTraceCompleted(const FTraceHandle& setHandle, FTraceDatum& setData);
	void DoWorkWithTraceResults(const FTraceDatum& setTraceData);

	TEnumAsByte<ETraceTypeQuery> myTraceType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Objects")
	TArray<AActor*> actorsIgnore;

	FTraceHandle lastTraceHandle;
	FTraceDelegate traceDelegate;
	uint32 bIsWantsTrace : 1;
	

private:

	TArray<FFadeObjStruct> fadeObjects;

	// Some worker timer
	FTimerHandle timerHandle_ObjectComputeTimer;
	FTimerHandle timerHandle_AddObjectsTimer;
	FTimerHandle timerHandle_AddPlayersTimer;

	// Temp variable
	float currentFade;

	// Now ID
	int32 fadeNowID;

	// Primitive components temp variable
	TArray<UPrimitiveComponent*> fadeObjectsTemp;

	// Primitive components temp variable
	TArray<UPrimitiveComponent*> fadeObjectsHit;

	// Translucent material for fade object
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	UMaterialInstance* fadeMaterial;

	// Enable or disable fade object worker
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsEnabled = true;

	// Pause or unpause fade object worker
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsActivate = true;

	// This can reduce performance.
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	bool bIsTraceComplex = false;

	// Timer interval
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float addObjectInterval = 0.1f;
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float calcFadeInterval = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float workDistance = 5000.0f;
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float nearCameraRadius = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	TSubclassOf<AActor> playerClass;

	// Check trace block by this
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	TArray<TEnumAsByte<ECollisionChannel>> objectTypes;

	// Rate fade increment
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float fadeRate = 10.0f;

	// Trace object size
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float capsuleHalfHeight = 88.0f;
	// Trace object size
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float capsuleRadius = 34.0f;

	// All characters array (maybe you control ( > 1 ) characters)
	TArray<AActor*> characterArray;

	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float nearObjectFade = 0.3;
	// Fade near and close parameters
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float farObjectFade = 0.1;

	// Instance fade
	UPROPERTY(EditAnywhere, Category = "Fade Objects")
	float immediatelyFade = 0.5f;
	
};