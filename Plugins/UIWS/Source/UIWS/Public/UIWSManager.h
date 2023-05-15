// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIWSManager.generated.h"

class AUIWSWaterBody;
class UMaterialParameterCollection;
class UPostProcessComponent;
// One of these per level.  If using level streaming, only one manager in the persistent level is required.  Non needed in sublevels.
UCLASS(hideCategories = (Rendering, Input, Actor, Cooking))
class UIWS_API AUIWSManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUIWSManager();

	/** Enable to update light info for the current level.  Just for ease of us in the editor*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Update")
	bool UpdateLightInfo = false;

	/** This post process volume is used so that the underwater volumes blend correctly in game.  It shouldn't conflict with other volumes, but if you're getting strange results uncheck this uproperty*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Simulation", AdvancedDisplay)
	bool bShouldSpawnGlobalPostProcess = true;

	/** Enable to allow caustic strength and direction to be updated in real-time in game.  Useful if you update your directional light as part of a time of day system*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Simulation")
	bool ContinuousLightUpdate = false;

	/** If true, water sim will center on pawn.  If disabled sim will center on camera.  If pawn is invalid (ie dead or not a APawn) sim centers on camera */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Simulation")
	bool CenterSimOnPawn = true;

	/** Adjusts water ripple panner for wind speed*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Water Params")
	FVector WindVector = FVector(1);

	/** Adjusts water ripple motion chaos for wind turbulence*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Water Params")
	float WindTurbulence = 3;

	/** Brightness multiplier applied to caustics.  Set this per level if you need specific overrides*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS: Tweaks")
	float CausticBrightnessMult = 1.0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void InitBodies(bool bUpdate);

	void ClearMPCs();

	void UpdateLightMPCVals();
	UPROPERTY()
	UMaterialParameterCollection* MPC_UIWSWaterBodies;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void UpdateManagedBodies(bool bUpdate);

	virtual void RequestPriority(AUIWSWaterBody* RequestingBody);


	virtual void OnConstruction(const FTransform & Transform) override;

	virtual bool ShouldTickIfViewportsOnly() const override;

	virtual void UpdateRegistration(bool Register, AUIWSWaterBody* body);

	UPROPERTY()
	TArray<TWeakObjectPtr<AUIWSWaterBody>> ManagedWaterBodies;

	/** This post process volume is used so that the underwater volumes blend correctly in game.  It shouldn't conflict with other volumes, but if you're getting strange results uncheck SpawnPostProcess uproperty on this actor*/
	UPROPERTY(BlueprintReadOnly, Category = "UIWS")
	UPostProcessComponent* PostProcessComp;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	UPROPERTY()
	AUIWSWaterBody* CurrentPriorityBody;

	UPROPERTY()
	AUIWSWaterBody* LastPriorityBody;
};
