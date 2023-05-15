// Copyright 2018 Elliot Gray. All Rights Reserved.

#include "UIWSCapture.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/BoxComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialParameterCollection.h"
#include "UObject/ConstructorHelpers.h"
#include "UIWS.h"

//DECLARE_CYCLE_STAT(TEXT("UIWS/WaterCapture"), STAT_WaterCapture, STATGROUP_UIWS);
// Sets default values
AUIWSCapture::AUIWSCapture()
{

	SetReplicates(false);
	//get material param collection
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> MPC_BodiesAsset(TEXT("/UIWS/MPC_UIWSWaterBodies.MPC_UIWSWaterBodies"));
	if (MPC_BodiesAsset.Succeeded())
	{
		MPC_UIWSWaterBodies = MPC_BodiesAsset.Object;
	}
	////get water static mesh
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterSM(TEXT("/UIWS/Materials/Simulation/Meshes/UIWSMesh16x16fbx.UIWSMesh16x16fbx"));
	//if (WaterSM.Succeeded())
	//{
	//	WaterMeshSM = WaterSM.Object;
	//	//WaterMeshComp->SetStaticMesh(WaterSM.Object);
	//	//WaterMeshComp->SetMobility(EComponentMobility::Static);
	//}

	//get heighsim material
	//static ConstructorHelpers::FObjectFinder<UMaterialInterface> DTP(TEXT("/UIWS/Materials/Capture/M_UIWSDrawToPersistent.M_UIWSDrawToPersistent"));
	//if (DTP.Succeeded())
	//{
	//	DrawToPMat = DTP.Object;
	//}

	//get heighsim material
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PP_Depth(TEXT("/UIWS/Materials/Capture/PP_UIWSDepthCheck.PP_UIWSDepthCheck"));
	if (PP_Depth.Succeeded())
	{
		//DrawToPMat = DTP.Object;
		DrawToPMat = PP_Depth.Object;
	}

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(SceneRoot);

	PPVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("PPVolume"));
	PPVolume->SetupAttachment(SceneRoot);
	PPVolume->SetBoxExtent(FVector(1,1,1));

	/** Kind of lame solution to scene capture getting hecka messed up by post process volumes.
		May need to extend this if you're doing anything funky in your own game*/
	PostProcessComp = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComp"));
	PostProcessComp->SetupAttachment(PPVolume);
	PostProcessComp->BlendRadius = 0;
	PostProcessComp->BlendWeight = 1000;
	PostProcessComp->bUnbound = false;
#if ENGINE_MINOR_VERSION <23
	PostProcessComp->Settings.bOverride_DepthOfFieldMethod = 1;
	PostProcessComp->Settings.DepthOfFieldMethod = EDepthOfFieldMethod::DOFM_BokehDOF;
	PostProcessComp->Settings.bOverride_DepthOfFieldMaxBokehSize = 1;
	PostProcessComp->Settings.DepthOfFieldMaxBokehSize = 0;
#endif
#if ENGINE_MINOR_VERSION >22
	PostProcessComp->Settings.bOverride_DepthOfFieldFocalDistance = true;
	PostProcessComp->Settings.DepthOfFieldFocalDistance = 0;
#endif

	PostProcessComp->Settings.bOverride_DepthOfFieldFstop = 1;
	PostProcessComp->Settings.bOverride_DepthOfFieldFocalDistance = 1;
	PostProcessComp->Settings.DepthOfFieldFstop = 4;
	PostProcessComp->Settings.DepthOfFieldFocalDistance = 1000;
	PostProcessComp->Settings.bOverride_MotionBlurAmount = 1;
	PostProcessComp->Settings.MotionBlurAmount = 0;
	PostProcessComp->Settings.bOverride_MotionBlurMax = 1;
	PostProcessComp->Settings.MotionBlurMax = 0;

	/** Setup the scene capture and disable most rendering features because we just want the custom depth.  
		With all these disabled the capture is extremely cheap */
	SceneCaptureComp = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture Comp"));
	SceneCaptureComp->SetupAttachment(SceneRoot);
	SceneCaptureComp->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComp->SetWorldRotation(FRotator(-90,90,0));
	SceneCaptureComp->AddOrUpdateBlendable(PP_Depth.Object, 1);
	SceneCaptureComp->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	SceneCaptureComp->ShowFlags.SetDynamicShadows(false);
	SceneCaptureComp->ShowFlags.SetAntiAliasing(false);
	SceneCaptureComp->ShowFlags.SetAmbientOcclusion(false);
#if ENGINE_MINOR_VERSION<24
	SceneCaptureComp->ShowFlags.SetAtmosphericFog(false);
#endif
	SceneCaptureComp->ShowFlags.SetBloom(false);
	SceneCaptureComp->ShowFlags.SetDirectLighting(false);
	SceneCaptureComp->ShowFlags.SetInstancedFoliage(false);
	SceneCaptureComp->ShowFlags.SetFog(false);
	SceneCaptureComp->ShowFlags.SetAmbientCubemap(false);
	SceneCaptureComp->ShowFlags.SetDecals(false);
	SceneCaptureComp->ShowFlags.SetLighting(false);
	SceneCaptureComp->ShowFlags.SetTranslucency(false);
	SceneCaptureComp->bCaptureOnMovement = false;
	SceneCaptureComp->bCaptureEveryFrame = true;


	//SMCOMP = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SMCOMP"));
	//SMCOMP->SetupAttachment(RootComponent);
	//SMCOMP->SetStaticMesh(WaterMeshSM);
}

// Called when the game starts or when spawned
void AUIWSCapture::BeginPlay()
{
	Super::BeginPlay();
	FLinearColor ColPos = UKismetMaterialLibrary::GetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"));
	FVector PlayerLoc = FVector(ColPos.R,ColPos.G,ColPos.B);
	SetActorLocation(FVector(PlayerLoc.X, PlayerLoc.Y, GetActorLocation().Z));
}

void AUIWSCapture::MoveCapture()
{
	FLinearColor ColPos = UKismetMaterialLibrary::GetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"));
	FVector PlayerLoc = FVector(ColPos.R, ColPos.G, ColPos.B);
	SetActorLocation(FVector(PlayerLoc.X, PlayerLoc.Y, GetActorLocation().Z));


	//MoveCapturePixel();
}

void AUIWSCapture::MoveCapturePixel()
{
	FLinearColor ColPos = UKismetMaterialLibrary::GetVectorParameterValue(this, MPC_UIWSWaterBodies, TEXT("playerpos"));
	FVector PlayerLoc = FVector(ColPos.R, ColPos.G, ColPos.B);
	FVector MoveAmount;
	MoveAmount = SnapPixelWorldSize(PlayerLoc.X, PlayerLoc.Y, SceneCaptureComp->OrthoWidth*(1/256));
	AddActorWorldOffset(FVector(MoveAmount.X, MoveAmount.Y, 0));
}

void AUIWSCapture::DrawToPersistent()
{
	DrawMID->SetVectorParameterValue(TEXT("Offset"), FLinearColor(moveoffset/SceneCaptureComp->OrthoWidth));
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RTPersistent, DrawMID);

}

FVector AUIWSCapture::SnapPixelWorldSize(float PX, float PY, float PWS)
{
	FVector VecOut;
	VecOut = FVector(FMath::FloorToInt(PX/PWS), FMath::FloorToInt(PY/PWS), 0);
	VecOut = VecOut*PWS;
	return VecOut;
}

// Called every frame
void AUIWSCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//SCOPE_CYCLE_COUNTER(STAT_WaterCapture);
	//SceneCaptureComp->CaptureScene();
	MoveCapture();
//	DrawToPersistent();
}

UTextureRenderTarget2D* AUIWSCapture::SetupCapture(float EdgeTestDepth, int32 RTRes)
{
	DynamicDrawToPMat = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, DrawToPMat);
	//RTPersistent = UKismetRenderingLibrary::CreateRenderTarget2D(this);
	RTCapture = UKismetRenderingLibrary::CreateRenderTarget2D(this, RTRes, RTRes);

	SceneCaptureComp->TextureTarget = RTCapture;
	SceneCaptureComp->OrthoWidth = UKismetMaterialLibrary::GetScalarParameterValue(this, MPC_UIWSWaterBodies, TEXT("InteractiveDistance"));
	
	//bp originally set mpc_capture ortho width here.  replace references?  don't think it's actually used 

	//DrawMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, DrawToPMat);
	//DrawMID->SetTextureParameterValue(TEXT("RTCapture"), RTCapture);
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RTCapture);
	//UKismetRenderingLibrary::ClearRenderTarget2D(this, RTPersistent);
	return RTCapture;
}

