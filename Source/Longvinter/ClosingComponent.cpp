// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "ClosingComponent.h"

// Sets default values for this component's properties
UClosingComponent::UClosingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.


	// ...
}

void UClosingComponent::ShutDown()
{
	GIsRequestingExit = true;
}



// Called when the game starts
void UClosingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UClosingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
