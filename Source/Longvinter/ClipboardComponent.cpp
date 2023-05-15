// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "ClipboardComponent.h"

// Sets default values for this component's properties
UClipboardComponent::UClipboardComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UClipboardComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UClipboardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UClipboardComponent::CopyToClipboard(FString text) {
	FPlatformMisc::ClipboardCopy(*text);
}