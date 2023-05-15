// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UIWSInteractorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), hideCategories = (Cooking, Input, Replication, Tags, "Component Replication"))
class UIWS_API UUIWSInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIWSInteractorComponent();

	/**Enable/disable water ripples. (this just enables or disables custom depth on all child components - no need to use the component if you want to set those values yourself elsewhere)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS")
	bool bShouldInteract = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition = "bShouldInteract"), Category = "UIWS")
	bool bEnableInteractiveStateSwitching = true;

	/**Any actor velocity lower than this will disable interaction
	This solves any unwated/unrealistic ripples caused by an object being permanently in water.
	Not ideal for skeletal meshes as you may want their animations to impact water despite no actor movement.
	Will extend in the future.
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(EditCondition = "bEnableInteractiveStateSwitching"), Category = "UIWS")
	float MinInteractionVelocity = 50.0f;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UPROPERTY()
	TArray<UActorComponent*> Statics;
	UPROPERTY()
	TArray<UActorComponent*> Skels;

	void EnableInteraction();
	void DisableInteraction();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**Called to update list of static and skeletal mesh comps.  Call it whenever you add another component if you need want this component to control it's interactivity state*/	
	UFUNCTION(BlueprintCallable, Category = "UIWS")
	void UpdateComponentList();
};
