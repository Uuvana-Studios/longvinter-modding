// Copyright 2018 Elliot Gray. All Rights Reserved.

#include "UIWSManager.h"
#include "UIWSWaterBody.h"

#include "Runtime/Launch/Resources/Version.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInterface.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/Engine.h"
#include "UIWS.h"

//DECLARE_CYCLE_STAT(TEXT("UIWS/WaterManager"), STAT_WaterManager, STATGROUP_UIWS);

// Sets default values
AUIWSManager::AUIWSManager()
{
	
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PPComp"));
	PostProcessComp->Priority = -1.0f;
	PostProcessComp->bUnbound = false;
	SetReplicates(false);


	//get material param collection
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MPC_BodiesAsset(TEXT("/UIWS/MPC_UIWSWaterBodies.MPC_UIWSWaterBodies"));
	if (MPC_BodiesAsset.Succeeded())
	{
		MPC_UIWSWaterBodies = MPC_BodiesAsset.Object;
	}

	//get post process mat
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnderWaterPPGlobal(TEXT("/UIWS/Materials/Simulation/PP_UnderWater_Instinstn.PP_UnderWater_Instinstn"));
	if (UnderWaterPPGlobal.Succeeded())
	{
		PostProcessComp->AddOrUpdateBlendable(UnderWaterPPGlobal.Object);
	}


}

// Called when the game starts or when spawned
void AUIWSManager::BeginPlay()
{
	Super::BeginPlay();
	//get array of bodies
	//UpdateManagedBodies(false);

	//prep bodies for tick
#if ENGINE_MINOR_VERSION >22
	PostProcessComp->Settings.DepthOfFieldFocalDistance = 0;
#endif
	UpdateLightMPCVals();
	//PostProcessComp->Settings.bOverride_MotionBlurAmount = 1;
	//PostProcessComp->Settings.MotionBlurAmount = 0;
	//PostProcessComp->Settings.bOverride_MotionBlurMax = 1;
	//PostProcessComp->Settings.MotionBlurMax = 0;
	if(bShouldSpawnGlobalPostProcess)
	{
		PostProcessComp->bUnbound = true;
		//PostProcessComp->Settings.bOverride_DepthOfFieldFstop = 1;
		//PostProcessComp->Settings.bOverride_DepthOfFieldMethod = 1;
		//PostProcessComp->Settings.bOverride_DepthOfFieldMethod = 2;
		//PostProcessComp->Settings.DepthOfFieldMaxBokehSize = 0;

#if ENGINE_MINOR_VERSION >22
		PostProcessComp->Settings.bOverride_DepthOfFieldFocalDistance = true;
		PostProcessComp->Settings.DepthOfFieldFocalDistance = 100000.0;
#endif
	}
	else
	{
		PostProcessComp->DestroyComponent(true);
	}
}

void AUIWSManager::InitBodies(bool bUpdate)
{
	for (auto &WaterBody : ManagedWaterBodies)
	{
		AUIWSWaterBody* bod = Cast<AUIWSWaterBody>(WaterBody);
		if (bod)
		{

			bod->InitializeWaterMaterial(bUpdate);
			bod->InitializeRenderTargets(bUpdate);
			bod->MyManager = this;
		}
	}
}

void AUIWSManager::ClearMPCs()
{
	for (int i = 1; i<=6;i++)
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(i) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(FVector(0,0,0)));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(i) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(FVector(0, 0, 0)));

		FString ThisBodyRotName = "WaterBody" + FString::FromInt(i) + "_Rot";
		//Set my sale in param collection

		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(FVector(0, 0, 0)));
	}
}

void AUIWSManager::UpdateLightMPCVals()
{
	TArray<AActor*> DirectionalLights;
	UGameplayStatics::GetAllActorsOfClass(this, ADirectionalLight::StaticClass(), DirectionalLights);
	if (DirectionalLights.Num() > 0)
	{
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found a light");
		ADirectionalLight* DirectionalLight = Cast<ADirectionalLight>(DirectionalLights[0]);
		if (DirectionalLight)
		{
			//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "light valid");
			UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("DirectionalLightIntensity"), DirectionalLight->GetLightComponent()->Intensity);
			//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString::SanitizeFloat(DirectionalLight->GetLightComponent()->Intensity));
			UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("DirectionalLightVector"), FLinearColor(DirectionalLight->GetActorForwardVector()));
		}
	}
	else
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("DirectionalLightIntensity"), 0);
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("DirectionalLightVector"), FLinearColor(FVector(0,0,1)));
	}

	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("WindChoppyness"), WindTurbulence);
	UKismetMaterialLibrary::SetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("CausticBrightness"), CausticBrightnessMult);
	UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("WindVector"), WindVector);
}

// Called every frame
void AUIWSManager::Tick(float DeltaTime)
{
	//SCOPE_CYCLE_COUNTER(STAT_WaterManager);
	Super::Tick(DeltaTime);

	//update player position in the mpc.  if there's a valid pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if(PlayerPawn && CenterSimOnPawn)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"), FLinearColor(PlayerPawn->GetActorLocation()));
	}else if(APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Pawn was invalid.  Centering on camera");
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"), FLinearColor(PC->PlayerCameraManager->GetCameraLocation()));
	}else
	{
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Camera was invalid, not updating sim center position");
		UE_LOG(LogTemp, Warning, TEXT("UIWS Manager failed to find a pawn OR camera to center water simulation on.  UIWSManager.CPP Line 112"));
	}
	

	if(ContinuousLightUpdate)
	{
		UpdateLightMPCVals();
	}
}

void AUIWSManager::UpdateManagedBodies(bool bUpdate)
{
	//clear MPC refs
	for (int i = 0; i < UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")); i++)
	{
		//ckear mpc entries for any stored bodies
		if (i != 0 && i <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
		{
			FString ThisBodyPosName = "WaterBody" + FString::FromInt(i) + "_Position";
			//Set my position in param collection
			UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(FVector(0, 0, -200000)));

			FString ThisBodyScaleName = "WaterBody" + FString::FromInt(i) + "_Scale";
			//Set my sale in param collection
			UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(FVector::ZeroVector));

			FString ThisBodyRotName = "WaterBody" + FString::FromInt(i) + "_Rot";
			//Set my sale in param collection

			UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(FVector(0, 0, 0)));
		}
	}
	//Recreate MPC refs
	for (auto &WaterBody : ManagedWaterBodies)
	{
		if(WaterBody.IsValid() && WaterBody->bIsInteractive)
		{
			int Index;
			ManagedWaterBodies.Find(WaterBody, Index);
			WaterBody->WaterBodyNum = Index+1;

			//Tell the water body to add it's position and scale values to the water body param collection
			WaterBody->AddToMPC();
		}
	}
	
}

void AUIWSManager::RequestPriority(AUIWSWaterBody* RequestingBody)
{
	if (RequestingBody == CurrentPriorityBody)
	{
		//no need to update if the body we're splashin in is already the priority body
		return;
	}
	LastPriorityBody = CurrentPriorityBody;
	CurrentPriorityBody = RequestingBody;
	for (auto &WaterBody : ManagedWaterBodies)
	{
		AUIWSWaterBody* bod = Cast<AUIWSWaterBody>(WaterBody);
		if (bod)
		{
			if(bod == RequestingBody)
			{
				bod->bGeneratesInteractiveCaustics = true;
			}
			else
			{
				bod->bGeneratesInteractiveCaustics = false;
			}
		}
	}
	ManagedWaterBodies.Remove(RequestingBody);
	ManagedWaterBodies.Insert(RequestingBody, 0);

	//Swap out interactive caustic render targets
	InitBodies(true);

	//Make sure the interactive caustics are being displayed at the correct place in the world based on the MPC values
	UpdateManagedBodies(true);
}

void AUIWSManager::OnConstruction(const FTransform & Transform)
{
	UpdateLightMPCVals();

}

bool AUIWSManager::ShouldTickIfViewportsOnly() const
{
	return false;
}

void AUIWSManager::UpdateRegistration(bool Register, AUIWSWaterBody * body)
{
	//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Manager Updating Registration");

	if (Register)
	{
		ManagedWaterBodies.Add(body);
	}
	else
	{
		ManagedWaterBodies.Remove(body);
	}

	UpdateManagedBodies(true);
}

#if WITH_EDITOR
void AUIWSManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateManagedBodies(true);
	UpdateLightInfo = false;
	UpdateLightMPCVals();
}
#endif