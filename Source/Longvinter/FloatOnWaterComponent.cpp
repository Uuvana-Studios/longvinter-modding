// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "FloatOnWaterComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values for this component's properties
UFloatOnWaterComponent::UFloatOnWaterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool UFloatOnWaterComponent::FloatOnWater(float DeltaSeconds, float TraceLength, float Multiplier, TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes, USceneComponent* TraceComponent, UStaticMeshComponent* Cube)
{
    if (!TraceComponent) return false;

    AActor* Owner = GetOwner();
    if (!Owner) return false;

    FVector TraceStart = TraceComponent->GetComponentLocation();
    FVector TraceEnd = TraceStart - FVector(0, 0, TraceLength);

    FHitResult HitResult;
    bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, TraceStart, TraceEnd, ObjectTypes);

    DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, -1, 0, 1.f);

    if (Cube)
    {
        FVector ForceToAdd;

        if (bHit)
        {
            float Distance = (HitResult.Location - TraceStart).Size();
            float Alpha = Distance / TraceLength;
            float Mass = Cube->GetMass();
            float LerpValue = FMath::Lerp(Mass * 1000.0f, 0.0f, Alpha);

            float ForceMultiplier = (DeltaSeconds > 0.1f) ? FMath::Clamp(0.8f - DeltaSeconds , 0.0f, 1.0f) : 1.0f;
            ForceToAdd = HitResult.ImpactNormal * LerpValue * ForceMultiplier * Multiplier;
        }
        else
        {
            FVector Gravity = FVector(0, 0, -980);
            ForceToAdd = Cube->GetMass() * 900 * Gravity * HitResult.Distance;
        }

        Cube->AddForceAtLocation(ForceToAdd, TraceStart, NAME_None);
        return bHit;
    }
    return false;
}




// Called when the game starts
void UFloatOnWaterComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UFloatOnWaterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

