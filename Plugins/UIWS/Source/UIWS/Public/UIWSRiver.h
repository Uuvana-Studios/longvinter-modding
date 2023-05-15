// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UIWSWaterBody.h"
#include "UIWSRiver.generated.h"


class USplineMeshComponent;
class USplineComponent;
/**
 * 
 */
UCLASS()
class UIWS_API AUIWSRiver : public AUIWSWaterBody
{
	GENERATED_BODY()
	
	AUIWSRiver();

	TArray<USplineMeshComponent*> SplineMeshComps;
	
	virtual void CreateMeshSurface() override;
	
	virtual USplineMeshComponent* CreateSplineMeshComp();


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;

#endif

protected:
	void AddToMPC() override;
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWSRiver", AdvancedDisplay)
	USplineComponent* SplineComp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWSRiver")
	bool AllowCameraUnder = false;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
