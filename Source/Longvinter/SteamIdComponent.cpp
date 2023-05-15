// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "SteamIdComponent.h"
#include "OnlineSubsystemUtils.h"
#include <OnlineSubsystemUtils/Public/OnlineSubsystemUtils.h>

// Sets default values for this component's properties
USteamIdComponent::USteamIdComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USteamIdComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}



// Called every frame
void USteamIdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}