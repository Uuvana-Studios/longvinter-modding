// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "UMG_Navigation_Component.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LONGVINTER_API UUMG_Navigation_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUMG_Navigation_Component();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "")
	void SetupNavigationKeys(bool KeyNavigation, bool TabNavigation, bool AnalogNavigation);

	UFUNCTION(BlueprintCallable, Category = "")
	void SetFocusBrush(FSlateBrush brush);
};
