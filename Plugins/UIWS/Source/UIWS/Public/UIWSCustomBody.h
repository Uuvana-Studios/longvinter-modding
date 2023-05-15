// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIWSWaterBody.h"
#include "UIWSCustomBody.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class UIWS_API AUIWSCustomBody : public AUIWSWaterBody
{
	GENERATED_BODY()

	AUIWSCustomBody();

	virtual void CreateMeshSurface() override;
	

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UIWS Custom Body")
	UStaticMesh* CustomStaticMesh = WaterMeshSM;
	UPROPERTY()
	UStaticMeshComponent* CustomMeshComp;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DisablePostProcessVolume = true;

	virtual void BeginPlay() override;

	virtual void AddToMPC() override;

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Custom Body", AdvancedDisplay)
	UBoxComponent* CustomWaterVolume;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Custom Body")
	bool AllowCameraUnder = false;

	virtual void OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) override;
};
