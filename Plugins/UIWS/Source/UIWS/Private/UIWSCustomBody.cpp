// Copyright 2018 Elliot Gray. All Rights Reserved.

#include "UIWSCustomBody.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Components/PostProcessComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"



AUIWSCustomBody::AUIWSCustomBody()
{
	CustomMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CustomMeshComp"));
	CustomMeshComp->SetupAttachment(RootComponent);
	PostProcessComp->bEnabled = false;
	WaterVolume->SetVisibility(false);
	WaterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	bDisableAutomaticInteraction = true;

	CustomWaterVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Custom Post Process Volume"));
	CustomWaterVolume->SetupAttachment(RootComponent);
	CustomWaterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	//CustomWaterVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	//CustomWaterVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CustomWaterVolume->SetRelativeScale3D(FVector(1, 1, 1));
	CustomWaterVolume->SetRelativeLocation(FVector(0,0,-500));
	CustomWaterVolume->SetBoxExtent(FVector(500, 500, 500));
	CustomWaterVolume->SetVisibility(false);
	PostProcessComp->SetupAttachment(CustomWaterVolume);

	//PostProcessComp->AttachToComponent(CustomWaterVolume, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	//PostProcessComp->AttachToComponent(CustomWaterVolume);
}

void AUIWSCustomBody::CreateMeshSurface()
{
	InitializeWaterMaterial(false);
	CustomMeshComp->SetStaticMesh(CustomStaticMesh);
	CustomMeshComp->SetMaterial(0, WaterMID);
	CustomMeshComp->SetMaterial(1, WaterMIDLOD1);
	CustomMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CustomMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	if (AllowCameraUnder == false)
	{
		CustomWaterVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block);
	}
	CustomMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CustomMeshComp->bCastDynamicShadow = false;
	CustomMeshComp->bCastVolumetricTranslucentShadow = false;
	CustomMeshComp->bCastFarShadow = false;
	CustomMeshComp->bCastStaticShadow = false;
}





void AUIWSCustomBody::OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	Super::OnWaterOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);;
}


void AUIWSCustomBody::BeginPlay()
{
	Super::BeginPlay();
	if (bIsInteractive)
	{
		CustomWaterVolume->OnComponentBeginOverlap.AddDynamic(this, &AUIWSCustomBody::OnWaterOverlap);


		if (UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (CustomWaterVolume->IsOverlappingActor(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				RequestPriorityManual();
			}
		}
	}
}

void AUIWSCustomBody::AddToMPC()
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Orange, "AddtoMPC()");
	if (WaterBodyNum != 0 && WaterBodyNum <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(GetActorLocation()));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(GetActorScale3D() * 0));

		FString ThisBodyRotName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Rot";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(GetActorRotation().Euler()));
	}
}


#if WITH_EDITOR
void AUIWSCustomBody::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (DisablePostProcessVolume)
	{
		PostProcessComp->bEnabled = false;
		CustomWaterVolume->SetVisibility(false);
	}
	else
	{
		PostProcessComp->bEnabled = false;
		CustomWaterVolume->SetVisibility(true);
	}
}


void AUIWSCustomBody::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);
	CreateMeshSurface();
}
#endif