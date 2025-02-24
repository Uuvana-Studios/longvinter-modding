// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "RoadSpawningComponent.h"

// Sets default values for this component's properties
URoadSpawningComponent::URoadSpawningComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URoadSpawningComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void URoadSpawningComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void URoadSpawningComponent::GenerateRoadAlongSpline(USplineComponent* Spline, TSubclassOf<AActor> RoadActorClass, float RoadSize, TArray<AActor*>& OutSpawnedRoads)
{
    if (!Spline || !RoadActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateRoadAlongSpline: Invalid Spline or RoadActorClass"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("GenerateRoadAlongSpline: Unable to get World"));
        return;
    }

    OutSpawnedRoads.Empty();

    float SplineLength = Spline->GetSplineLength();
    int32 NumSegments = FMath::CeilToInt(SplineLength / RoadSize);

    UE_LOG(LogTemp, Log, TEXT("GenerateRoadAlongSpline: Spline Length: %f, Number of Segments: %d"), SplineLength, NumSegments);

    float CurrentDistance = 0.0f;

    for (int32 i = 0; i < NumSegments; ++i)
    {
        FVector Location = Spline->GetLocationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);
        FRotator Rotation = Spline->GetRotationAtDistanceAlongSpline(CurrentDistance, ESplineCoordinateSpace::World);

        // Spawn the road actor
        AActor* RoadActor = World->SpawnActor<AActor>(RoadActorClass, Location, Rotation);
        if (RoadActor)
        {
            OutSpawnedRoads.Add(RoadActor);

            UE_LOG(LogTemp, Log, TEXT("Spawned Road Actor at Location: %s, Rotation: %s"),
                *Location.ToString(), *Rotation.ToString());

            // Draw debug box to visualize the spawned actor
            DrawDebugBox(World, Location, FVector(50, 50, 10), FColor::Green, false, 5.0f);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn Road Actor at Location: %s"), *Location.ToString());
        }

        // Move to the next segment
        CurrentDistance += RoadSize;
    }

    UE_LOG(LogTemp, Log, TEXT("GenerateRoadAlongSpline: Finished spawning %d road actors"), OutSpawnedRoads.Num());
}

void URoadSpawningComponent::GenerateRandomWidthPattern(TArray<int32>& OutPattern, int32 Length)
{
    OutPattern.Empty(Length);

    for (int32 i = 0; i < Length; ++i)
    {
        int32 Width = FMath::RandRange(1, 3);
        OutPattern.Add(Width);
    }

    // Convert the integer array to a string array for logging
    TArray<FString> WidthStrings;
    for (int32 Width : OutPattern)
    {
        WidthStrings.Add(FString::FromInt(Width));
    }

    UE_LOG(LogTemp, Log, TEXT("Generated Width Pattern: %s"), *FString::Join(WidthStrings, TEXT(", ")));
}