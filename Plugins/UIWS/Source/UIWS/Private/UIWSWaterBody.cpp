// Copyright 2018 Elliot Gray. All Rights Reserved.

#include "UIWSWaterBody.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/Canvas.h"
#include "UIWSCapture.h"
#include "UIWSManager.h"
#include "Components/BoxComponent.h"
#include "Components/PostProcessComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/SceneCaptureComponent2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/DamageType.h"
#include "Kismet/KismetStringLibrary.h"
#include "UIWS.h"

//DECLARE_CYCLE_STAT(TEXT("UIWS/Automatic Interaction"), STAT_AutoInteraction, STATGROUP_UIWS);
//DECLARE_CYCLE_STAT(TEXT("UIWS/Manual Interaction"), STAT_ManualInteraction, STATGROUP_UIWS);
//DECLARE_CYCLE_STAT(TEXT("UIWS/WaterBody"), STAT_WaterBody, STATGROUP_UIWS);

// Sets default values
AUIWSWaterBody::AUIWSWaterBody()
{	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(false);
	// Box comp that might come in handy later for more aggressive culling/lodding
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationCollision"));
	SetRootComponent(BoxComp);
	BoxComp->SetVisibility(false, false);
	
	/*
	WaterMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComp"));
	WaterMeshComp->SetupAttachment(RootComponent);
	//WaterMeshComp->SetVisibility(false);
	//WaterMeshComp->SetHiddenInGame(true);

	WaterMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	WaterMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	WaterMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	WaterMeshComp->SetWorldScale3D(FVector(WaterMeshComp->GetComponentTransform().GetScale3D().X, WaterMeshComp->GetComponentTransform().GetScale3D().Y, 1));

	*/
	/** Setup Post Process Volume*/
	WaterVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("Post Process Volume"));
	WaterVolume->SetupAttachment(RootComponent);
	WaterVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	WaterVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	WaterVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	/** Setup Post Process comp*/
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("Post Process Comp"));
	PostProcessComp->SetupAttachment(WaterVolume);

	/** Setup the post process settings for underwater visuals*/
	PostProcessComp->Activate();
	PostProcessComp->bUnbound = false;
	PostProcessComp->BlendRadius = 20;
	PostProcessComp->Settings.bOverride_DepthOfFieldFstop = 1;
	PostProcessComp->Settings.bOverride_DepthOfFieldFocalDistance = 1;
	PostProcessComp->Settings.DepthOfFieldFstop = 1.0;
	PostProcessComp->Settings.DepthOfFieldFocalDistance = 50;

#if ENGINE_MINOR_VERSION <23
	PostProcessComp->Settings.bOverride_DepthOfFieldMethod = 1;
	PostProcessComp->Settings.DepthOfFieldMethod = EDepthOfFieldMethod::DOFM_CircleDOF;
#endif
#if ENGINE_MINOR_VERSION >22
	PostProcessComp->Settings.bOverride_DepthOfFieldFocalDistance = true;
	PostProcessComp->Settings.DepthOfFieldFocalDistance = 50;
#endif

	//Location = GetActorLocation();
	//Scale = GetActorScale();

	// Box comp that might come in handy later for more agressive culling/lodding
	//BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationCollision"));
	//BoxComp->SetupAttachment(WaterMeshComp);
	//BoxComp->SetBoxExtent(FVector (1000,1000,100));
	//BoxComp->SetRelativeLocation(FVector(500,500,0));
	//BoxComp->SetHiddenInGame(false);
	//BoxComp->SetVisibility(true);

	//get all the content refs we need
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MPC_BodiesAsset(TEXT("/UIWS/MPC_UIWSWaterBodies.MPC_UIWSWaterBodies"));
	if (MPC_BodiesAsset.Succeeded())
	{
		MPC_UIWSWaterBodies = MPC_BodiesAsset.Object;
	}
	//get water static mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterSM(TEXT("/UIWS/Materials/Simulation/Meshes/UIWSMesh16x16fbx.UIWSMesh16x16fbx"));
	if (WaterSM.Succeeded())
	{
		WaterMeshSM = WaterSM.Object;
		//WaterMeshComp->SetStaticMesh(WaterSM.Object);
		//WaterMeshComp->SetMobility(EComponentMobility::Static);
	}

	//get water material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WaterMaterialLod1(TEXT("/UIWS/Materials/Simulation/UIWSWaterMaterialInst_LOD1.UIWSWaterMaterialInst_LOD1"));//transparent water mat
	if (WaterMaterialLod1.Succeeded())
	{
		WaterMatLOD1 = WaterMaterialLod1.Object;
	}

	//get water material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WaterMaterial(TEXT("/UIWS/Materials/Simulation/UIWSWaterMaterial_Inst.UIWSWaterMaterial_Inst"));//transparent water mat
	if (WaterMaterial.Succeeded())
	{
		WaterMat = WaterMaterial.Object;
	}
	/*
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> WaterMaterial(TEXT("/UIWS/Materials/Simulation/UIWSWaterMaterial_Opaque.UIWSWaterMaterial_Opaque")); //opaque water mat
	if (WaterMaterial.Succeeded())
	{
	WaterMat= WaterMaterial.Object;
	}
	*/
	//get force splat material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> SplatMat(TEXT("/UIWS/Materials/Simulation/UIWSForceSplat.UIWSForceSplat"));
	if (SplatMat.Succeeded())
	{
		ForceSplatMat = SplatMat.Object;
	}

	//get manual force splat material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ManSplatMat(TEXT("/UIWS/Materials/Simulation/UIWSManualSplat.UIWSManualSplat"));
	if (ManSplatMat.Succeeded())
	{
		ManForceSplatMat = ManSplatMat.Object;
	}

	//get heighsim material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> HSMat(TEXT("/UIWS/Materials/Simulation/UIWSHeightSim.UIWSHeightSim"));
	if (HSMat.Succeeded())
	{
		HeightSimMat = HSMat.Object;
	}

	//get computenormal material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> CNMat(TEXT("/UIWS/Materials/Simulation/UIWSComputeNormal.UIWSComputeNormal"));
	if (CNMat.Succeeded())
	{
		ComputeNormalMat = CNMat.Object;
	}

	//get global rts
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> gh0(TEXT("/UIWS/Materials/Simulation/UIWSHeight0.UIWSHeight0"));
	if (gh0.Succeeded())
	{
		globalheight0 = gh0.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gh0");
	}

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> gh1(TEXT("/UIWS/Materials/Simulation/UIWSHeight1.UIWSHeight1"));
	if (gh1.Succeeded())
	{
		globalheight1 = gh1.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gh1");
	}

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> gh2(TEXT("/UIWS/Materials/Simulation/UIWSHeight2.UIWSHeight2"));
	if (gh2.Succeeded())
	{
		globalheight2 = gh2.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gh2");
	}
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> gn(TEXT("/UIWS/Materials/Simulation/UIWSHeightfieldNormal.UIWSHeightfieldNormal"));
	if (gn.Succeeded())
	{
		globalnormal = gn.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gn");
	}

	//get the underwater post process material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnderWaterPPMat(TEXT("/UIWS/Materials/Simulation/PP_UnderWater.PP_UnderWater"));
	if (UnderWaterPPMat.Succeeded())
	{
		PPUnderWaterMat = UnderWaterPPMat.Object;

	}
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ds(TEXT("/UIWS/ParticleEffects/DefaultSplash.DefaultSplash"));
	if (ds.Succeeded())
	{
		DefaultSplashEffect = ds.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gn");
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> ief(TEXT("/UIWS/ParticleEffects/P_MeshSplash.P_MeshSplash"));
	if (ief.Succeeded())
	{
		InteractionEffect = ief.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gn");
	}

	static ConstructorHelpers::FObjectFinder<UParticleSystem> def(TEXT("/UIWS/ParticleEffects/P_MeshSplashHit.P_MeshSplashHit"));
	if (def.Succeeded())
	{
		DamageEffect = def.Object;
		//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Found gn");
	}


}

void AUIWSWaterBody::AddToMPC()
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "AddtoMPC()");
	if (WaterBodyNum != 0 && WaterBodyNum <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(GetActorLocation()));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(GetActorScale3D() * 1000));

		FString ThisBodyRotName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Rot";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(GetActorRotation().Euler()));
	}
}

// Called when the game starts or when spawned
void AUIWSWaterBody::BeginPlay()
{
	Super::BeginPlay();
	/** Binding to check when player character hits the water surface.  Used to request body priority switch and make the currently stood in body the one affecting interactive caustics*/
#if !WITH_EDITOR
	CreateMeshSurface();
	TArray<UActorComponent*> StaticComps = GetComponentsByClass(UStaticMeshComponent::StaticClass());
	for (auto &comp : StaticComps)
	{
		UStaticMeshComponent* sm = Cast<UStaticMeshComponent>(comp);
		if (sm)
		{
			sm->SetMaterial(0, WaterMID);
			sm->SetMaterial(1, WaterMIDLOD1);
		}
	}
#endif
	//Find Manager
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AUIWSManager::StaticClass(), FoundActors);

	if (FoundActors.Num() < 1)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "No UIWS Manager found, have you added one to the persistent level?");
	}
	else
	{
		AUIWSManager* Manager = Cast<AUIWSManager>(FoundActors[0]);
		if (Manager)
		{
			MyManager = Manager;
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Something went wrong.  Couldn't find a UIWS water manager");
		}
	}
	
	RegisterWithManager();
	if(GetWorld()->GetNetMode() == NM_DedicatedServer || GetGameInstance()->GetWorldContext()->RunAsDedicated)
	{
		bIsInteractive = false;
	}
	if(bIsInteractive)
	{
		WaterVolume->OnComponentBeginOverlap.AddDynamic(this, &AUIWSWaterBody::OnWaterOverlap);
		PPUnderWaterMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, PPUnderWaterMat);
		PostProcessComp->AddOrUpdateBlendable(PPUnderWaterMID);

		if(bLimitTickRate)
		{
			ChangeBodyTickRate(TickRate);
		}
		SetWaterVisualParams();
		//ApplyInteractivityForces();
		//PropagateRipples(0.016);

		if(UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (WaterVolume->IsOverlappingActor(UGameplayStatics::GetPlayerPawn(this, 0)))
			{
				RequestPriorityManual();
			}
		}
	}

	ForceSplatInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, ForceSplatMat);
	HeightSimInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, HeightSimMat);
	ManualSplatInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, ManForceSplatMat);
	ComputeNormalInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, ComputeNormalMat);
}

void AUIWSWaterBody::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (MyManager.IsValid())
	{
		MyManager->UpdateRegistration(false, this);
	}

}


void AUIWSWaterBody::ApplyInteractivityForces()
{

	ForceSplatInst->SetTextureParameterValue(TEXT("RTPersistentIn"), myCaptureRT);
	
	//Get player position in interactive UV space
	FLinearColor UVLC = UKismetMaterialLibrary::GetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"));;
	FVector WPVec = FVector(UVLC.R, UVLC.G, 0);
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, WorldPosToRelativeUV(WPVec).ToString());
	ForceSplatInst->SetVectorParameterValue(TEXT("ForcePosition"), FLinearColor(WorldPosToRelativeUV(WPVec)));
	ForceSplatInst->SetScalarParameterValue(TEXT("ForceStrength"), 1);
	if(GetHeightRT(iHeightState)!=nullptr)
	{

		//badUKismetRenderingLibrary::DrawMaterialToRenderTarget(this, GetHeightRT(iHeightState), ForceSplatInst);
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, GetHeightRT(iHeightState), Canvas, Size, Context);
		Canvas->K2_DrawMaterial(ForceSplatInst, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), 0.0f, FVector2D(0, 0));
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
	}

}

void AUIWSWaterBody::PropagateRipples(float inDeltaTime)
{
	if(bTieSimToFPS)
	{
		iHeightState = (iHeightState + 1) % 3;
		if (GetLastHeightRT(iHeightState, 1) != nullptr)
		{
			HeightSimInst->SetTextureParameterValue(TEXT("PreviousHeight1"), GetLastHeightRT(iHeightState, 1));
		}
		if (GetLastHeightRT(iHeightState, 2) != nullptr)
		{
			HeightSimInst->SetTextureParameterValue(TEXT("PreviousHeight2"), GetLastHeightRT(iHeightState, 2));
		}
	
		if (GetHeightRT(iHeightState) != nullptr)
		{
			UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, GetHeightRT(iHeightState), Canvas, Size, Context);
			Canvas->K2_DrawMaterial(HeightSimInst, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), 0.0f, FVector2D(0, 0));
			UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
		}
	}
	else
	{
		//propagate ripples as per requested simulation rate
		fTimeAccumulator = fTimeAccumulator + inDeltaTime;
		float ac1 = 1 / FMath::Min(fUpdateRate, 120.0f);

		while (fTimeAccumulator >= ac1)
		{
			fTimeAccumulator = fTimeAccumulator - ac1;
			iHeightState = (iHeightState + 1) % 3;
			//UMaterialInstanceDynamic* HeightSimInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, HeightSimMat);
			if (GetLastHeightRT(iHeightState, 1) != nullptr)
			{
				HeightSimInst->SetTextureParameterValue(TEXT("PreviousHeight1"), GetLastHeightRT(iHeightState, 1));
			}
			if (GetLastHeightRT(iHeightState, 2) != nullptr)
			{
				HeightSimInst->SetTextureParameterValue(TEXT("PreviousHeight2"), GetLastHeightRT(iHeightState, 2));
			}

			if (GetHeightRT(iHeightState) != nullptr)
			{
				UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, GetHeightRT(iHeightState), Canvas, Size, Context);
				Canvas->K2_DrawMaterial(HeightSimInst, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), 0.0f, FVector2D(0, 0));
				UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
			}

		}
	}
	
	//compute normals once per frame
	if(GetHeightRT(iHeightState)!=nullptr)
	{
		//UMaterialInstanceDynamic* ComputeNormalInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, ComputeNormalMat);
		ComputeNormalInst->SetTextureParameterValue(TEXT("Heightfield"), GetHeightRT(iHeightState));
		//badUKismetRenderingLibrary::DrawMaterialToRenderTarget(this, activenormal, ComputeNormalInst);

		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, activenormal, Canvas, Size, Context);
		Canvas->K2_DrawMaterial(ComputeNormalInst, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), 0.0f, FVector2D(0, 0));
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);


		WaterMID->SetTextureParameterValue(TEXT("Heightfield"), GetHeightRT(iHeightState));
		WaterMIDLOD1->SetTextureParameterValue(TEXT("Heightfield"), GetHeightRT(iHeightState));

	}

	WaterMID->SetTextureParameterValue(TEXT("HeightfieldNormal"), activenormal);
	WaterMIDLOD1->SetTextureParameterValue(TEXT("HeightfieldNormal"), activenormal);

	if(GetHeightRT(iHeightState) == nullptr)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "HEIGHT RT IS NULL");
	}
	if (activenormal == nullptr)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "ACTIVENORMAL RT IS NULL");
	}
	if (GetLastHeightRT(iHeightState,1) == nullptr)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "HEIGHT RT LAST1 IS NULL");
	}
	if (GetLastHeightRT(iHeightState,2) == nullptr)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "HEIGHT RT LAST2 IS NULL");
	}
}

float AUIWSWaterBody::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageDealer)
{
	UDamageType const* const DamageTypeCDO = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();

	//Handle Point damage
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		FPointDamageEvent* const PDEvent = (FPointDamageEvent*)&DamageEvent;
		if(InteractOnDamage)
		{
			PointDamageSplashAtlocation(PDEvent->HitInfo.ImpactPoint, DamageAmount);
		}
	}
	//Handle radial Damage
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		FRadialDamageEvent* const PDEvent = (FRadialDamageEvent*)&DamageEvent;
		if (InteractOnDamage)
		{
			RadialDamageSplashAtlocation(PDEvent->Origin, DamageAmount);
		}
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageDealer);
}

void AUIWSWaterBody::SplashAtlocation(FVector SplashLoc, FVector SplashVelocity, float SplashStrengthPercent)
{
	//SCOPE_CYCLE_COUNTER(STAT_ManualInteraction);
	UParticleSystem* SysToSpawn;
	if (InteractionEffect)
	{
		SysToSpawn = InteractionEffect;
	}
	else
	{
		SysToSpawn = DefaultSplashEffect;
	}

	UParticleSystemComponent* Splash = UGameplayStatics::SpawnEmitterAtLocation(this, SysToSpawn, SplashLoc, FRotator::ZeroRotator, true);
	Splash->SetRenderCustomDepth(true);
	Splash->SetCustomDepthStencilValue(1);
	Splash->SetWorldScale3D(FVector(UKismetMathLibrary::Lerp(InteractionEffectScaleMin, InteractionEffectScaleMax, SplashStrengthPercent)));
	Splash->SetVectorParameter(TEXT("SplashVelocity"), SplashVelocity);
	SplashedAtLocation(SplashLoc, SplashVelocity, SplashStrengthPercent);
	//Splash->SetWorldScale3D(FVector(SplashStrengthPercent));
}

void AUIWSWaterBody::PointDamageSplashAtlocation(FVector SplashLoc, float DamageAmount /*= 1.0f*/)
{
	//SCOPE_CYCLE_COUNTER(STAT_ManualInteraction);
	UParticleSystem* SysToSpawn;
	if(DamageEffect)
	{
		SysToSpawn = DamageEffect;
	}
	else
	{
		SysToSpawn = DefaultSplashEffect;
	}
	//Render Ripples
	float RippleStrengthScaled = FMath::GetMappedRangeValueClamped(FVector2D(5, 10), FVector2D(0, DamageEffectScaleMax), DamageAmount);
	float RippleSizeScaled = FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(0, DamageEffectScaleMax), DamageAmount);
	ApplyForceAtLocation(RippleStrengthScaled, RippleSizeScaled, SplashLoc);

	//Zero the splash spawn location incase of radial damage origin
	//SplashLoc.Z = GetActorLocation().Z;
	float ParticleScaleMult = FMath::GetMappedRangeValueClamped(FVector2D(0, DamageScale), FVector2D(DamageEffectScaleMin, DamageEffectScaleMax), DamageAmount);
	//Spawn particle
	if (bEnableParticleOnDamage)
	{
		UParticleSystemComponent* Splash = UGameplayStatics::SpawnEmitterAtLocation(this, SysToSpawn, SplashLoc, FRotator::ZeroRotator, true);
		Splash->SetRenderCustomDepth(true);
		Splash->SetCustomDepthStencilValue(1);
		Splash->SetWorldScale3D(FVector(ParticleScaleMult));
	}
	OnPointDamageEffect(SplashLoc, RippleStrengthScaled, RippleSizeScaled, DamageAmount);

}

void AUIWSWaterBody::RadialDamageSplashAtlocation(FVector SplashLoc, float DamageAmount /*= 100.0f*/)
{
	//SCOPE_CYCLE_COUNTER(STAT_ManualInteraction);
	UParticleSystem* SysToSpawn;
	if (DamageEffect)
	{
		SysToSpawn = DamageEffect;
	}
	else
	{
		SysToSpawn = DefaultSplashEffect;
	}
	//Render Ripples
	float RippleStrengthScaled = FMath::GetMappedRangeValueClamped(FVector2D(5, 10), FVector2D(0, DamageEffectScaleMax), DamageAmount);
	float RippleSizeScaled = FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(0, DamageEffectScaleMax), DamageAmount);
	ApplyForceAtLocation(RippleStrengthScaled, RippleSizeScaled, SplashLoc);

	//Zero the splash spawn location incase of radial damage origin
	SplashLoc.Z = GetActorLocation().Z;
	float ParticleScaleMult = FMath::GetMappedRangeValueClamped(FVector2D(0, DamageScale), FVector2D(DamageEffectScaleMin, DamageEffectScaleMax), DamageAmount);
	//Spawn particle
	if (bEnableParticleOnDamage)
	{
		UParticleSystemComponent* Splash = UGameplayStatics::SpawnEmitterAtLocation(this, SysToSpawn, SplashLoc, FRotator::ZeroRotator, true);
		Splash->SetWorldScale3D(FVector(ParticleScaleMult));
	}
	OnRadialDamageEffect(SplashLoc, RippleStrengthScaled, RippleSizeScaled, DamageAmount);
}

FVector AUIWSWaterBody::WorldPosToRelativeUV(FVector WorldPos)
{
	float IntDistance = UKismetMaterialLibrary::GetScalarParameterValue(this,MPC_UIWSWaterBodies, TEXT("InteractiveDistance"));
	
	float x = (UKismetMathLibrary::GenericPercent_FloatFloat(WorldPos.X + (IntDistance / 2), IntDistance)) / -IntDistance;
	float y = (UKismetMathLibrary::GenericPercent_FloatFloat(WorldPos.Y + (IntDistance / 2), IntDistance)) / IntDistance;
	return FVector(x, y, 0);
}

UTextureRenderTarget2D* AUIWSWaterBody::SetupCaptureCPP()
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "SetupCaptureCPP()");
	FActorSpawnParameters SpawnParams;
	FRotator SpawnRot;
	SpawnParams.Owner = this;
	SpawnRot.Pitch = GetActorRotation().Pitch;
	SpawnRot.Roll = GetActorRotation().Roll;
	SpawnRot.Yaw = 0.0f;
	myCaptureActor = GetWorld()->SpawnActor<AUIWSCapture>(GetActorLocation()+FVector(0,0,CaptureOffset), SpawnRot, SpawnParams);
	return myCaptureActor->SetupCapture(EdgeDepth, CaptureRes);
}

void AUIWSWaterBody::InitializeWaterMaterial(bool bUpdate)
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "InitializeWaterMaterial() Update = " + UKismetStringLibrary::Conv_BoolToString(bUpdate));
	if(!bUpdate)
	{

		UMaterialInterface* ThisWaterMat;
		UMaterialInterface* ThisDistantMat;
		if(OverrideWaterMaterials == true && CustomWaterMaterial!=nullptr)
		{
			ThisWaterMat = CustomWaterMaterial;
			if(CustomWaterMaterialDistant!=nullptr)
			{
				ThisDistantMat = CustomWaterMaterialDistant;
			}else
			{
				ThisDistantMat = CustomWaterMaterial;
			}
		}
		else
		{
			ThisWaterMat = WaterMat;
			ThisDistantMat = WaterMatLOD1;
		}
		WaterMID = UMaterialInstanceDynamic::Create(ThisWaterMat, this);
		WaterMIDLOD1 = UMaterialInstanceDynamic::Create(ThisDistantMat, this);
		/*
		for (auto &XMesh : XMeshes)
		{
			XMesh->SetMaterial(0, WaterMID);;
			XMesh->SetMaterial(1, WaterMIDLOD1);
		}

		for (auto &YMesh : YMeshes)
		{
			YMesh->SetMaterial(0, WaterMID);;
			YMesh->SetMaterial(1, WaterMIDLOD1);
		}
		*/
		//WaterMeshComp->SetMaterial(0,WaterMID);
		//WaterMeshComp->SetMaterial(1, WaterMIDLOD1);

		TArray<UActorComponent*> StaticComps = GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (auto &comp : StaticComps)
		{
			UStaticMeshComponent* sm = Cast<UStaticMeshComponent>(comp);
			if (sm)
			{
				sm->SetMaterial(0, WaterMID);
				sm->SetMaterial(1, WaterMIDLOD1);
			}
		}

		SetWaterVisualParams();

	}
	//WaterMID->SetScalarParameterValue(TEXT("MyHeight(Needs Replacement)"), GetActorLocation().Z); //this is dumb change this functionality in the shader 
}

void AUIWSWaterBody::InitializeRenderTargets(bool bUpdate)
{

	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "InitializeRenderTargets() Update = " + UKismetStringLibrary::Conv_BoolToString(bUpdate));
	if(bGeneratesInteractiveCaustics == true)
	{
		//Clear global render targets ready for use
		UKismetRenderingLibrary::ClearRenderTarget2D(this, globalheight0);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, globalheight1);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, globalheight2);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, globalnormal);
		if(HeightSimInst)
			HeightSimInst->SetScalarParameterValue(TEXT("SupportsReflection"), bSupportsEdgeReflection);
		if(bUpdate)
		{
			//if this is an update (ie, not first init) we need to copy the local ripple rt's to the global so the existing ripples persist.  This is pretty heavy so only persisting for the new primary body and letting the others just reset

			UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, globalheight0, Canvas, Size, Context);
			Canvas->K2_DrawTexture(localheight0, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White, EBlendMode::BLEND_Opaque);
			UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
	
			UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, globalheight1, Canvas, Size, Context);
			Canvas->K2_DrawTexture(localheight1, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White, EBlendMode::BLEND_Opaque);
			UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);

			UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, globalheight2, Canvas, Size, Context);
			Canvas->K2_DrawTexture(localheight2, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White, EBlendMode::BLEND_Opaque);
			UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);

			UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, globalnormal, Canvas, Size, Context);
			Canvas->K2_DrawTexture(localnormal, FVector2D(0, 0), Size, FVector2D(0, 0), FVector2D(1, 1), FLinearColor::White, EBlendMode::BLEND_Opaque);
			UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, Context);
		}
		activeheight0 = globalheight0;
		activeheight1 = globalheight1;
		activeheight2 = globalheight2;
		activenormal = globalnormal;
	}
	else
	{
		//create local render targets if they don't already exist
		if(localheight0 ==nullptr)
		{
			localheight0 = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimResMin, SimResMin);
		}
		if (localheight1 == nullptr)
		{
			localheight1 = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimResMin, SimResMin);
		}
		if (localheight2 == nullptr)
		{
			localheight2 = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimResMin, SimResMin);
		}
		if (localnormal == nullptr)
		{
			localnormal = UKismetRenderingLibrary::CreateRenderTarget2D(this, SimResMin, SimResMin);
		}
		UKismetRenderingLibrary::ClearRenderTarget2D(this, localheight0);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, localheight1);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, localheight2);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, localnormal);
		if (HeightSimInst)
			HeightSimInst->SetScalarParameterValue(TEXT("SupportsReflection"), 0);
		activeheight0 = localheight0;
		activeheight1 = localheight1;
		activeheight2 = localheight2;
		activenormal = localnormal;
	}

	WaterMID->SetTextureParameterValue(TEXT("HeightfieldNormal"),activenormal);
	//Only spawn and configure a scene capture if we're initializing for the first time

	/** Platform specific interaction type overrides*/
#if PLATFORM_SWITCH
	if(!bDisableAutomaticInteraction)
	{
		bDisableAutomaticInteraction = bDisableAutomaticInteractionNintendoSwitch;
	}
#endif
#if PLATFORM_XBOXONE
	if (!bDisableAutomaticInteraction)
	{
		bDisableAutomaticInteraction = bDisableAutomaticInteractionXbox;
	}
#endif
#if PLATFORM_PS4
	if (!bDisableAutomaticInteraction)
	{
		bDisableAutomaticInteraction = bDisableAutomaticInteractionPS4;
	}
#endif
#if PLATFORM_ANDROID
	if (!bDisableAutomaticInteraction)
	{
		bDisableAutomaticInteraction = bDisableAutomaticInteractionAndroid;
	}
#endif
#if PLATFORM_IOS
	if (!bDisableAutomaticInteraction)
	{
		bDisableAutomaticInteraction = bDisableAutomaticInteractionIOS;
	}
#endif

	if(!bUpdate && !bDisableAutomaticInteraction)
	{
		myCaptureRT = SetupCaptureCPP();
	}

}

void AUIWSWaterBody::ChangeBodyTickRate(float fNewTickRate)
{
	if(fNewTickRate == 0)
	{
		SetActorTickInterval(0);
		if(myCaptureActor)
		{
			myCaptureActor->SetActorTickInterval(0);
		}

	}
	else
	{
		SetActorTickInterval(1 / fNewTickRate);
		if (myCaptureActor)
		{
			myCaptureActor->SetActorTickInterval(1 / fNewTickRate);
		}

	}
}

UTextureRenderTarget2D* AUIWSWaterBody::GetHeightRT(int IndexIn)
{
	if(IndexIn == 0)
	{
		return activeheight0;
	}
	else if (IndexIn == 1)
	{
		return activeheight1;
	}
	else
	{
		return activeheight2;
	}
}

UTextureRenderTarget2D* AUIWSWaterBody::GetLastHeightRT(int CurrentHeightIndex, int NumFramesOld)
{
	return GetHeightRT((CurrentHeightIndex - NumFramesOld + 3)%3);
}



void AUIWSWaterBody::OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	////SCOPE_CYCLE_COUNTER(STAT_ManualInteraction);

	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "OnWaterOverlap()");
	if(OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
	{
		//CheckIfCulled();
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "Pawn overlapped me oof");
		if(MyManager.IsValid())
		{
			MyManager->RequestPriority(this);
		}else
		{
			UE_LOG(LogTemp, Warning, TEXT("Manager pointer was invalid when RequestPriorityManual() was called"));
		}
	
	}
	if(bEnableParticleOnCollision)
	{
		FVector loc = OtherActor->GetActorLocation();
		loc.Z = GetActorLocation().Z;
		FVector vel = OtherActor->GetVelocity();
		vel.Z = -vel.Z;
		SplashAtlocation(loc, vel, 1);
	}

}

void AUIWSWaterBody::RegisterWithManager()
{
	//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Water body Registering with Manager");
	if (MyManager.IsValid())
	{
		MyManager->UpdateRegistration(true, this);
		InitializeWaterMaterial(false);
		InitializeRenderTargets(false);
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "Body failed to register with manager as MyManager variable was null");
	}

}

void AUIWSWaterBody::UnRegisterWithManager()
{
	//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Water body Un-Registering with Manager");
	if (MyManager.IsValid())
	{
		MyManager->UpdateRegistration(false, this);
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "Body failed to Un-register with manager as MyManager variable was null");
	}
}

// Called every frame
void AUIWSWaterBody::Tick(float DeltaTime)
{
	//SCOPE_CYCLE_COUNTER(STAT_WaterBody);
	Super::Tick(DeltaTime);
	bLowFps = DeltaTime > 0.2f;

	if(bIsInteractive)
	{
		/*Unfortunately edge bounce introduces artefacting at low fps.  Unavoidable cost of entry*/
		if (bSupportsEdgeReflection && HeightSimInst)
		{
			if (bLowFps)
				HeightSimInst->SetScalarParameterValue(TEXT("SupportsReflection"), 0);
			else
				HeightSimInst->SetScalarParameterValue(TEXT("SupportsReflection"), 1);
		}
		ApplyInteractivityForces();
		PropagateRipples(DeltaTime);
	}
}


#if WITH_EDITOR
void AUIWSWaterBody::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{

	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "PostEditChangeProperty()");
	Super::PostEditChangeProperty(PropertyChangedEvent);
	//bIsInteractive = true;
	SetWaterVisualParams();
	//if(Location != FVector(1333333.3, 13333.33, 1333333.335))
	//{
	//SetActorLocation(Location);
	//SetActorScale3D(Scale);
	//}

}

void AUIWSWaterBody::PostEditMove(bool bFinished)
{
	//CreateMeshSurface();
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "PostEditMove()");
	Super::PostEditMove(bFinished);
	//SetActorRotation(FRotator::ZeroRotator);

	//Location = GetActorLocation();
	//Scale = GetActorScale();

	if (WaterBodyNum != 0 && WaterBodyNum <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(GetActorLocation()));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(GetActorScale3D() * 1000));


		FString ThisBodyRotName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Rot";
		//Set my sale in param collection
		
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(FVector(GetActorRotation().Roll, GetActorRotation().Pitch, GetActorRotation().Yaw)));

	}
}

void AUIWSWaterBody::EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
	Super::EditorApplyRotation(DeltaRotation, bAltDown, bShiftDown, bCtrlDown);
	FRotator ClearedRotation;
	//ClearedRotation.Pitch = FMath::Clamp(GetActorRotation().Pitch, -15.0f, 15.0f);
	//ClearedRotation.Roll = FMath::Clamp(GetActorRotation().Roll,-15.0f,15.0f);
	ClearedRotation.Pitch = 0;
	ClearedRotation.Roll = 0;
	ClearedRotation.Yaw = GetActorRotation().Yaw;
	SetActorRotation(ClearedRotation);

	if (WaterBodyNum != 0 && WaterBodyNum <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(GetActorLocation()));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(GetActorScale3D() * 1000));

		FString ThisBodyRotName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Rot";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyRotName), FLinearColor(GetActorRotation().Euler()));

	}
}
#endif

void AUIWSWaterBody::BeginDestroy()
{
	//Clean up my reference in the MPC
	if (WaterBodyNum != 0 && WaterBodyNum <= UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("SupportedBodyCount")))
	{
		FString ThisBodyPosName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Position";
		//Set my position in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyPosName), FLinearColor(FVector(0)));

		FString ThisBodyScaleName = "WaterBody" + FString::FromInt(WaterBodyNum) + "_Scale";
		//Set my sale in param collection
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC_UIWSWaterBodies, FName(*ThisBodyScaleName), FLinearColor(FVector(0)));
	}
	Super::BeginDestroy();
}

void AUIWSWaterBody::SetWaterVisualParams()
{
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, "SetWaterVisualParams()");
	if(WaterMID)
	{
		WaterMID->SetVectorParameterValue(TEXT("WaterColor"), WaterColor);
		WaterMID->SetVectorParameterValue(TEXT("DeepWater"), DeepOpacity);
		WaterMID->SetVectorParameterValue(TEXT("ShallowWater"), ShallowOpacity);
		WaterMID->SetScalarParameterValue(TEXT("Water Depth"), DepthTransitionDistance);
		WaterMID->SetScalarParameterValue(TEXT("Shallow EdgeFadeDistance"), ShallowEdgeFadeDistance);
		WaterMID->SetScalarParameterValue(TEXT("ShallowEdgeFadePower"), ShallowEdgeFadePower);

		WaterMIDLOD1->SetVectorParameterValue(TEXT("WaterColor"), WaterColor);
		WaterMIDLOD1->SetVectorParameterValue(TEXT("DeepWater"), DeepOpacity);
		WaterMIDLOD1->SetVectorParameterValue(TEXT("ShallowWater"), ShallowOpacity);
		WaterMIDLOD1->SetScalarParameterValue(TEXT("Water Depth"), DepthTransitionDistance);
		WaterMIDLOD1->SetScalarParameterValue(TEXT("Shallow EdgeFadeDistance"), ShallowEdgeFadeDistance);
		WaterMIDLOD1->SetScalarParameterValue(TEXT("ShallowEdgeFadePower"), ShallowEdgeFadePower);
	}


	if(PPUnderWaterMID)
	{
		PPUnderWaterMID->SetVectorParameterValue(TEXT("UnderWaterTint"), WaterColor);
		PPUnderWaterMID->SetVectorParameterValue(TEXT("MyBodyPos"), FLinearColor(GetActorLocation()));
		PPUnderWaterMID->SetVectorParameterValue(TEXT("MyBodyScale"), FLinearColor(GetActorScale3D() * 1000));
		PPUnderWaterMID->SetScalarParameterValue(TEXT("MaxUnderwaterBrightness"), UnderWaterBrightnessMult);
	}

	if(WaterVolume)
	{
		WaterVolume->SetRelativeScale3D(FVector(1, 1, 1));
		WaterVolume->SetBoxExtent(FVector(500, 500, 500));
		FVector rot = FVector(500 * GetActorScale3D().X, 500 * GetActorScale3D().Y,0);
		//FVector NewPos = rot.RotateAngleAxis(GetActorRotation().Yaw,FVector(0,0,1));
		FVector NewPos = rot.RotateAngleAxis(GetActorRotation().Roll, FVector(1, 0, 0));
		NewPos = NewPos.RotateAngleAxis(GetActorRotation().Pitch, FVector(0, 1, 0));
		NewPos = NewPos.RotateAngleAxis(GetActorRotation().Yaw, FVector(0, 0, 1));

		//WaterVolume->SetWorldLocation(GetActorLocation() + FVector(500 * GetActorScale3D().X, 500 * GetActorScale3D().Y, -(WaterVolume->GetScaledBoxExtent().Z)));
		WaterVolume->SetWorldLocation(GetActorLocation() + FVector(NewPos.X, NewPos.Y, -(WaterVolume->GetScaledBoxExtent().Z)));
		//WaterVolume->SetRelativeLocation(FVector::ZeroVector);
		//WaterVolume->AddRelativeLocation(FVector(500*GetActorScale3D().X, 500 * GetActorScale3D().Y, -1000 * GetActorScale3D().Y));
	}
	
}

void AUIWSWaterBody::OnConstruction(const FTransform & Transform)
{
	
	Super::OnConstruction(Transform);

	



	InitializeWaterMaterial(false);
	CreateMeshSurface();

}

void AUIWSWaterBody::CheckIfCulled()
{
	//if (GEngine) //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Checking if culled");
	//do fps culling tasks
	//If the body is culled do the sim at a low framerate.  Will still do the caustic sim etc and be ready to go when it becomes visible but this should help performance
	//Stops the scene capture from rendering if the body isn't in view
	bool isVisible =true;// GetWorld()->GetTimeSeconds() - WaterMeshComp->LastRenderTimeOnScreen <= 0.5f;
	if (!isVisible)
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Orange, "Culling a body");
		ChangeBodyTickRate(UKismetMathLibrary::RandomFloatInRange(1, 2));
		myCaptureActor->SceneCaptureComp->SetComponentTickInterval(1 / UKismetMathLibrary::RandomFloatInRange(9,11));
		myCaptureActor->SceneCaptureComp->SetHiddenInGame(true);

	}
	else if (bLimitTickRate)
	{
		ChangeBodyTickRate(TickRate);
		myCaptureActor->SceneCaptureComp->SetComponentTickInterval(1 / TickRate);
		//myCaptureActor->SetActorHiddenInGame(false);
		//myCaptureRT = SetupCaptureCPP();
		myCaptureActor->SceneCaptureComp->SetHiddenInGame(false);
	}
	else
	{
		ChangeBodyTickRate(0.0f);
		myCaptureActor->SceneCaptureComp->SetComponentTickInterval(1 / 10);
		//myCaptureActor->SetActorHiddenInGame(false);
		//myCaptureRT = SetupCaptureCPP();
		myCaptureActor->SceneCaptureComp->SetHiddenInGame(false);
	}
}

void AUIWSWaterBody::CreateMeshSurface()
{
	/** Get rid of the old chunks*/
	for (auto &XMesh : XMeshes)
	{
		if(XMesh)
			XMesh->DestroyComponent();
	}
	XMeshes.Empty();

	for (auto &YMesh : YMeshes)
	{
		if(YMesh)
			YMesh->DestroyComponent();
	}
	YMeshes.Empty();
	CurrentYArray.Empty();

	InitializeWaterMaterial(false);

	UStaticMeshComponent* firstMesh = CreateSurfaceComponent();
	XMeshes.Add(firstMesh);
	/** Determine how many chunks we want to break the mesh up into, if any*/
	int Scalex = FMath::TruncToInt(GetActorScale3D().X);
	int chunksx = Scalex / FMath::Clamp(MaxTileScale,1,1000);
	if (chunksx > 0)
	{
		for (int i = 1; i < chunksx; i++)
		{
			UStaticMeshComponent* mesh = CreateSurfaceComponent();
			XMeshes.Add(mesh);
			mesh->SetWorldLocation(XMeshes[i - 1]->GetSocketLocation(TEXT("Extent_X")));
		}
	}

	/** Correctly place and scale each chunk*/
	for (auto &XMesh : XMeshes)
	{
		float Divisor = XMeshes.Num();
		float meshscaleX = XMesh->GetRelativeTransform().GetScale3D().X / Divisor;
		//float meshscaley = XMesh->GetRelativeTransform().GetScale3D().Y;
		XMesh->SetRelativeScale3D(FVector(meshscaleX, 1, 1));
		int arrayloc = XMeshes.Find(XMesh);
		if (arrayloc != 0)
		{
			XMesh->SetWorldLocation(XMeshes[arrayloc - 1]->GetSocketLocation(TEXT("Extent_X")));
		}
	}


	/** Spawn all the Y axis chunks*/
	for (auto &XMesh : XMeshes)
	{
		CurrentYArray.Empty();
		CurrentYArray.Add(XMesh);
		int Scaley = FMath::TruncToInt(GetActorScale3D().Y);
		int chunksy = Scaley / FMath::Clamp(MaxTileScale, 1, 1000);

		if (chunksy > 0)
		{
			for (int i = 1; i < chunksy; i++)
			{
				UStaticMeshComponent* mesh = CreateSurfaceComponent();
				CurrentYArray.Add(mesh);
				YMeshes.Add(mesh);
				mesh->SetWorldLocation(CurrentYArray[i - 1]->GetSocketLocation(TEXT("Extent_Y")));
			}


			/** Set Y Axis meshes scale*/
			for (auto &YMesh : CurrentYArray)
			{
				float Divisor = CurrentYArray.Num();
				float meshscaleY = YMesh->GetRelativeTransform().GetScale3D().Y / Divisor;
				float meshscalex = XMesh->GetRelativeTransform().GetScale3D().X;
				YMesh->SetRelativeScale3D(FVector(meshscalex, meshscaleY, 1));
				int arrayloc = CurrentYArray.Find(YMesh);
				if (arrayloc != 0)
				{
					YMesh->SetWorldLocation(CurrentYArray[arrayloc - 1]->GetSocketLocation(TEXT("Extent_Y")));
				}
			}
		}
	}


}

UStaticMeshComponent* AUIWSWaterBody::CreateSurfaceComponent()
{
	UStaticMeshComponent* comp = NewObject<UStaticMeshComponent>(this);
	comp->RegisterComponent();
	comp->SetStaticMesh(WaterMeshSM);
	comp->SetMaterial(0, WaterMID);
	comp->SetMaterial(1, WaterMIDLOD1);
	comp->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	comp->SetRelativeLocation(FVector(0, 0, 0));
	comp->SetRelativeScale3D(FVector(1, 1, 1));
	comp->SetCollisionResponseToAllChannels(ECR_Ignore);
	comp->LightmapType = ELightmapType::ForceSurface;
	comp->SetMobility(EComponentMobility::Movable);
	//comp->SetMaterial(0,MID)
	//comp->SetMaterial(0, WaterMID);
	//comp->SetMaterial(1, WaterMIDLOD1);
	//WaterMeshComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	comp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	/** For Joe so he stops whining */
	comp->bCastDynamicShadow = false;
	comp->bCastVolumetricTranslucentShadow = false;
	comp->bCastFarShadow = false;
	comp->bCastStaticShadow = false;
	return comp;
}

void AUIWSWaterBody::ApplyForceAtLocation(float fStrength, float fSizePercent, FVector HitLocation, bool bWithEffect)
{
	//SCOPE_CYCLE_COUNTER(STAT_ManualInteraction);
	FLinearColor UVLC = UKismetMaterialLibrary::GetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"));;
	float IntDistance = UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("InteractiveDistance"));
	FVector WPVec = FVector(UVLC.R, UVLC.G, 0);

	//if the ripple is within interactive bounds draw it
	if(HitLocation.X>WPVec.X-(IntDistance-400)/2 && HitLocation.X<WPVec.X + (IntDistance - 400) / 2 && HitLocation.Y>WPVec.Y - (IntDistance - 400) / 2 && HitLocation.Y<WPVec.Y + (IntDistance - 400) / 2)
	{
		//UMaterialInstanceDynamic* ManualSplatInst = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, ManForceSplatMat);
		FVector HitVec = HitLocation;
		FVector UVVec = WorldPosToRelativeUV(HitVec);
		ManualSplatInst->SetVectorParameterValue(TEXT("ForcePosition"), FLinearColor(UVVec));
		ManualSplatInst->SetScalarParameterValue(TEXT("ForceSizePercent"), fSizePercent);
		ManualSplatInst->SetScalarParameterValue(TEXT("ForceStrength"), fStrength);

		//		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, GetHeightRT(iHeightState), ManualSplatInst);
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this, GetHeightRT(iHeightState), CanvasMan, SizeMan, ContextMan);
		CanvasMan->K2_DrawMaterial(ManualSplatInst, FVector2D(0, 0), SizeMan, FVector2D(0, 0), FVector2D(1, 1), 0.0f, FVector2D(0, 0));
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, ContextMan);
	}
	

	if(bWithEffect)
	{
		SplashAtlocation(HitLocation, FVector(0,0,100), fStrength);
	}
	ForceAppliedAtLocation(HitLocation, fStrength);
}

void AUIWSWaterBody::RequestPriorityManual()
{
	if(MyManager.IsValid())
	{
		MyManager->RequestPriority(this);
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, "RequesPriorityManual()");
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Manager pointer was invalid when RequestPriorityManual() was called"));
	}

}

