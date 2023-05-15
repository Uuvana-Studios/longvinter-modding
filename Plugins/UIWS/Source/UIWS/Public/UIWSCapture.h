// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UIWSCapture.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UPostProcessComponent;
class UBoxComponent;
class UMaterialParameterCollection;
class UStaticMeshComponent;
UCLASS(NotPlaceable)
class UIWS_API AUIWSCapture : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUIWSCapture();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UFUNCTION(BlueprintCallable)
	UTextureRenderTarget2D* SetupCapture(float EdgeTestDepth, int32 RTRes = 256);
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS")
	USceneCaptureComponent2D* SceneCaptureComp;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintReadOnly, Category = "UIWS")
	UBoxComponent* PPVolume;


	//UStaticMesh* WaterMeshSM;

	//UStaticMeshComponent* SMCOMP;
protected:

	UPROPERTY()
	USceneComponent* SceneRoot; 

	UPROPERTY()
	UMaterialInstanceDynamic* DrawMID;
	UPROPERTY()
	FVector moveoffset = FVector(0);

	UPROPERTY()
	UTextureRenderTarget2D* RTPersistent;
	UPROPERTY()
	UTextureRenderTarget2D* RTCapture;

	UPROPERTY()
	UMaterialParameterCollection* MPC_UIWSWaterBodies;
	UPROPERTY()
	UMaterialInterface* DrawToPMat;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicDrawToPMat;

private:	
	//Move capture to player location at surface
	virtual void MoveCapture();
	//Alternate function, not in use, that moves the capture 1 ortho pixel at a time.  Unnessecary for water needs
	void MoveCapturePixel();
	//Draw the capture clamped depth to the persistent render target
	virtual void DrawToPersistent();

	FVector SnapPixelWorldSize(float PX, float PY, float PWS);

	UPROPERTY()
	UPostProcessComponent* PostProcessComp;
};
