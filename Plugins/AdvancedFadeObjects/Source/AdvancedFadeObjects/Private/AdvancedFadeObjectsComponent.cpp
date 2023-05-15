// Copyright 2020-2021 Fly Dream Dev. All Rights Reserved. 

#include "AdvancedFadeObjectsComponent.h"
#include "TimerManager.h"
#include "Engine.h"

#include "Kismet/GameplayStatics.h"


UAdvancedFadeObjectsComponent::UAdvancedFadeObjectsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// Add first collision type
	objectTypes.Add(ECC_WorldStatic);
}

// Called when the game starts
void UAdvancedFadeObjectsComponent::BeginPlay()
{
	Super::BeginPlay();

	traceDelegate.BindUObject(this, &UAdvancedFadeObjectsComponent::OnTraceCompleted);

	GetOwner()->GetWorld()->GetTimerManager().SetTimer(timerHandle_AddObjectsTimer, this, &UAdvancedFadeObjectsComponent::AddObjectsToHideTimer,
	                                                   addObjectInterval, true);
	GetOwner()->GetWorld()->GetTimerManager().SetTimer(timerHandle_ObjectComputeTimer, this, &UAdvancedFadeObjectsComponent::FadeObjWorkerTimer,
	                                                   calcFadeInterval, true);

	GetOwner()->GetWorld()->GetTimerManager().PauseTimer(timerHandle_ObjectComputeTimer);
	GetOwner()->GetWorld()->GetTimerManager().PauseTimer(timerHandle_AddObjectsTimer);

	SetActivate(bIsActivate);
}

// Called every frame
void UAdvancedFadeObjectsComponent::TickComponent(const float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAdvancedFadeObjectsComponent::AddObjectsToHideTimer()
{
	UGameplayStatics::GetAllActorsOfClass(this, playerClass, characterArray);

	if (lastTraceHandle._Data.FrameNumber != 0)
	{
		FTraceDatum OutData;
		if (GetWorld()->QueryTraceData(lastTraceHandle, OutData))
		{
			// Clear out handle so next tick we don't enter
			lastTraceHandle._Data.FrameNumber = 0;
			// trace is finished, do stuff with results
			DoWorkWithTraceResults(OutData);
		}
	}

	if (bIsWantsTrace)
	{
		lastTraceHandle = RequestTrace();
		bIsWantsTrace = false;
	}

	if (bIsEnabled)
	{
		for (AActor* currentActor_ : characterArray)
		{
			const FVector traceStart_ = GEngine->GetFirstLocalPlayerController(GetOwner()->GetWorld())->PlayerCameraManager->GetCameraLocation();
			const FVector traceEnd_ = currentActor_->GetActorLocation();
			FVector traceLength_ = traceStart_ - traceEnd_;
			const FQuat actorQuat_ = currentActor_->GetActorQuat();

			if (traceLength_.Size() < workDistance)
			{
				FCollisionQueryParams traceParams_(TEXT("FadeObjectsTrace"), bIsTraceComplex, GetOwner());

				traceParams_.AddIgnoredActors(actorsIgnore);
				traceParams_.bReturnPhysicalMaterial = false;
				// Not tracing complex uses the rough collision instead making tiny objects easier to select.
				traceParams_.bTraceComplex = bIsTraceComplex;

				TArray<FHitResult> hitArray_;
				TArray<TEnumAsByte<EObjectTypeQuery>> traceObjectTypes_;

				// Convert ECollisionChannel to ObjectType
				for (int i = 0; i < objectTypes.Num(); ++i)
				{
					traceObjectTypes_.Add(UEngineTypes::ConvertToObjectType(objectTypes[i].GetValue()));
				}

				FTraceDelegate* traceDelegate_ = &traceDelegate;

				GetOwner()->GetWorld()->AsyncSweepByObjectType(EAsyncTraceType::Multi, traceStart_, traceEnd_, actorQuat_, traceObjectTypes_,
				                                               FCollisionShape::MakeCapsule(capsuleRadius, capsuleHalfHeight), traceParams_, traceDelegate_);
			}
		}
	}
}

void UAdvancedFadeObjectsComponent::FadeObjWorkerTimer()
{
	if (fadeObjects.Num() > 0)
	{
		// For loop all fade objects
		for (int i = 0; i < fadeObjects.Num(); ++i)
		{
			// Index of iteration 
			int fnID_ = i;

			float adaptiveFade_;

			if (fnID_ == fadeObjects.Num())
			{
				adaptiveFade_ = nearObjectFade;
			}
			else
			{
				adaptiveFade_ = farObjectFade;
			}

			// For loop fadeMID array
			for (int t = 0; t < fadeObjects[i].fadeMID.Num(); ++t)
			{
				float targetF_;

				const float currentF = fadeObjects[i].fadeCurrent;

				if (fadeObjects[i].bToHide)
				{
					targetF_ = adaptiveFade_;
				}
				else
				{
					targetF_ = 1.0f;
				}

				const float newFade_ = FMath::FInterpConstantTo(currentF, targetF_, GetOwner()->GetWorld()->GetDeltaSeconds(), fadeRate);

				fadeObjects[i].fadeMID[t]->SetScalarParameterValue("Fade", newFade_);

				currentFade = newFade_;

				fadeObjects[i].SetFadeAndHide(newFade_, fadeObjects[i].bToHide);
			}
			// remove index in array
			if (currentFade == 1.0f)
			{
				for (int bmi = 0; bmi < fadeObjects[fnID_].baseMatInterface.Num(); ++bmi)
				{
					fadeObjects[fnID_].primitiveComp->SetMaterial(bmi, fadeObjects[fnID_].baseMatInterface[bmi]);
				}

				fadeObjects[fnID_].primitiveComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
				fadeObjects.RemoveAt(fnID_);
				fadeObjectsTemp.RemoveAt(fnID_);
			}
		}
	}
}

void UAdvancedFadeObjectsComponent::SetActivate(bool setActivate)
{
	bIsActivate = setActivate;

	if (!bIsActivate)
	{
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(timerHandle_ObjectComputeTimer);
		GetOwner()->GetWorld()->GetTimerManager().PauseTimer(timerHandle_AddObjectsTimer);
	}
	else
	{
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(timerHandle_ObjectComputeTimer);
		GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(timerHandle_AddObjectsTimer);
	}
}

void UAdvancedFadeObjectsComponent::SetWantsTrace()
{
	// don't allow overlapping traces here.
	if (!GetWorld()->IsTraceHandleValid(lastTraceHandle, false))
	{
		bIsWantsTrace = true;
	}
}

FTraceHandle UAdvancedFadeObjectsComponent::RequestTrace()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return FTraceHandle();

	auto Channel = UEngineTypes::ConvertToCollisionChannel(myTraceType);

	TArray<AActor*> ActorsToIgnore;

	FCollisionQueryParams traceParams_(TEXT("FadeObjectsTrace"), bIsTraceComplex, GetOwner());

	traceParams_.AddIgnoredActors(actorsIgnore);
	traceParams_.bReturnPhysicalMaterial = false;
	// Not tracing complex uses the rough collision instead making tiny objects easier to select.
	traceParams_.bTraceComplex = bIsTraceComplex;

	auto Params = traceParams_;
	auto Start = FVector::ZeroVector;
	auto End = FVector(1000.f);
	return World->AsyncLineTraceByChannel(EAsyncTraceType::Single,
	                                      Start, End,
	                                      Channel,
	                                      Params,
	                                      FCollisionResponseParams::DefaultResponseParam,
	                                      &traceDelegate);
}

void UAdvancedFadeObjectsComponent::OnTraceCompleted(const FTraceHandle& setHandle, FTraceDatum& setData)
{
	//	ensure(setHandle == lastTraceHandle);
	DoWorkWithTraceResults(setData);
	lastTraceHandle._Data.FrameNumber = 0; // reset it
}

void UAdvancedFadeObjectsComponent::DoWorkWithTraceResults(const FTraceDatum& setTraceData)
{
	for (int hA = 0; hA < setTraceData.OutHits.Num(); ++hA)
	{
		if (setTraceData.OutHits[hA].bBlockingHit && IsValid(setTraceData.OutHits[hA].GetComponent()) && !fadeObjectsHit.Contains(setTraceData.OutHits[hA].GetComponent()))
		{
			fadeObjectsHit.AddUnique(setTraceData.OutHits[hA].GetComponent());
		}
	}

	// Make fade array after complete GetAllActorsOfClass loop
	for (int fO = 0; fO < fadeObjectsHit.Num(); ++fO)
	{
		// If not contains this component in fadeObjectsTemp
		if (!fadeObjectsTemp.Contains(fadeObjectsHit[fO]))
		{
			TArray<UMaterialInterface*> lBaseMaterials_;
			TArray<UMaterialInstanceDynamic*> lMidMaterials_;

			lBaseMaterials_.Empty();
			lMidMaterials_.Empty();

			fadeObjectsTemp.AddUnique(fadeObjectsHit[fO]);

			// For loop all materials ID in object
			for (int nM = 0; nM < fadeObjectsHit[fO]->GetNumMaterials(); ++nM)
			{
				lMidMaterials_.Add(UMaterialInstanceDynamic::Create(fadeMaterial, fadeObjectsHit[fO]));
				lBaseMaterials_.Add(fadeObjectsHit[fO]->GetMaterial(nM));

				// Set new material on object
				fadeObjectsHit[fO]->SetMaterial(nM, lMidMaterials_.Last());
			}
			// Create new fade object in array of objects to fade
			FFadeObjStruct newObject_;
			newObject_.NewElement(fadeObjectsHit[fO], lBaseMaterials_, lMidMaterials_, immediatelyFade, true);
			// Add object to array
			fadeObjects.Add(newObject_);

			// Set collision on Primitive Component
			fadeObjectsHit[fO]->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		}
	}

	// Set hide to visible true if contains
	for (int fOT = 0; fOT < fadeObjectsTemp.Num(); ++fOT)
	{
		if (!fadeObjectsHit.Contains(fadeObjectsTemp[fOT]))
		{
			fadeObjects[fOT].SetHideOnly(false);
		}
	}

	// Clear array
	fadeObjectsHit.Empty();
}

void UAdvancedFadeObjectsComponent::SetEnable(bool setEnable)
{
	bIsEnabled = setEnable;
}
