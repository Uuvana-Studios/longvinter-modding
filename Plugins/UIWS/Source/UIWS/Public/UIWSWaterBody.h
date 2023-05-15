// Copyright 2018 Elliot Gray. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "UIWSWaterBody.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UTextureRenderTarget2D;
class UMaterialParameterCollectionInstance;
class UMaterialParameterCollection;
class AUIWSManager;
class AUIWSCapture;
class UBoxComponent;
class UPostProcessComponent;
class USceneCaptureComponent2D;

UCLASS(hideCategories = (Cooking, Input, Replication, "Actor Tick"))
class UIWS_API AUIWSWaterBody : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AUIWSWaterBody();

	virtual void AddToMPC();

	//UFUNCTION(BlueprintImplementableEvent)
	//UTextureRenderTarget2D* SetupCapture();

	UTextureRenderTarget2D* SetupCaptureCPP();

	//UFUNCTION(BlueprintCallable)
	void InitializeWaterMaterial(bool bUpdate);
	void InitializeRenderTargets(bool bUpdate);

	void ChangeBodyTickRate(float fNewTickRate);

	UPROPERTY()
	TWeakObjectPtr<AUIWSManager> MyManager = nullptr;



	bool nolocationset = false;
	bool bLowFps = false;

	/** Override base actor take damage functionality to apply effects and ripples automatically.  Calls parent functionality*/
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageDealer) override;

	/** Spawns a particle effect at a location*/
	UFUNCTION(BlueprintCallable, Category = "UIWS Functions")
	virtual void SplashAtlocation(FVector SplashLoc, FVector SplachVelocity = FVector(0,0,100), float SplashStrengthPercent = 1.0f);

	/** ripples water and spawns particle effect at location proportional to damage amount*/
	UFUNCTION(BlueprintCallable, Category = "UIWS Functions")
	virtual void PointDamageSplashAtlocation(FVector SplashLoc, float DamageAmount = 100.0f);

	/** ripples water and spawns particle effect at location proportional to damage amount*/
	UFUNCTION(BlueprintCallable, Category = "UIWS Functions")
	virtual void RadialDamageSplashAtlocation(FVector SplashLoc, float DamageAmount = 100.0f);

	/**	Called when any water interaction causes a particle splash (manual, automatic, collision, etc.  Excluding damage events*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UIWS Events")
	void SplashedAtLocation(FVector SplashLoc, FVector SplashVelocity, float Strength = 1);

	/**	Called when ripples are created manually at a location.  Doesn't trigger for automatically detected interactions based on custom depth*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UIWS Events")
	void ForceAppliedAtLocation(FVector RippleLoc, float Strength = 1);

	/**	Event that triggers when point damage effects would be applied.  Will trigger even if automatic effects are disabled for ease of extension*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UIWS Events")
	void OnPointDamageEffect(FVector RippleLocation, float RippleStrengthScaled, float RippleSizeScaled, float DamageAmount = 1);

	/**	Event that triggers when point damage effects would be applied.  Will trigger even if automatic effects are disabled for ease of extension*/
	UFUNCTION(BlueprintImplementableEvent, Category = "UIWS Events")
	void OnRadialDamageEffect(FVector RippleLocation, float RippleStrengthScaled, float RippleSizeScaled, float DamageAmount = 1);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the game starts or when spawned
	virtual void EndPlay(EEndPlayReason::Type  EndPlayReason) override;
	
	/**	Applies input for the ripple sim*/
	void ApplyInteractivityForces();

	/**	Calculates the ripple sim*/
	void PropagateRipples(float inDeltaTime);
	
	/**	Convert world position to player relative 'UV' space.  Utility.*/
	FVector WorldPosToRelativeUV(FVector WorldPos);

	/**	Get the current sim step height render target*/
	UTextureRenderTarget2D* GetHeightRT(int IndexIn);

	/**	Get the last sim step height render target*/
	UTextureRenderTarget2D* GetLastHeightRT(int CurrentHeightIndex, int NumFramesOld);

	/** Bound to any collision with water surface*/
	UFUNCTION()
	virtual void OnWaterOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	//void OnWaterSurfaceOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, FHitResult &SweepResult);
	UPROPERTY()
	UBoxComponent* BoxComp;
	UPROPERTY()
	AUIWSCapture* myCaptureActor;


	virtual void CreateMeshSurface();

	UStaticMeshComponent* CreateSurfaceComponent();
	UPROPERTY()
	TArray<UStaticMeshComponent*> XMeshes;
	UPROPERTY()
	TArray<UStaticMeshComponent*> YMeshes;
	UPROPERTY()
	TArray<UStaticMeshComponent*> CurrentYArray;
	UPROPERTY()
	TArray<UStaticMeshComponent*> WaterSurfaceMeshes;

	//UStaticMesh* WaterMeshSM;


	void RegisterWithManager();

	void UnRegisterWithManager();


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	UMaterialInterface* WaterMat;
	UMaterialInterface* WaterMatLOD1;
	/** Enable if you want to override the water material with your own.  Simply add MF_UIWS to your material to get access dynamic interaction*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "UIWS Material")
	bool OverrideWaterMaterials = false;
	/** Material for Lod 0 and 1*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "OverrideWaterMaterials"), Category = "UIWS Material")
	UMaterialInterface* CustomWaterMaterial = nullptr;
	/**Optional material for Lod 2.  If left blank CustomWaterMaterial will be used*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "OverrideWaterMaterials"), Category = "UIWS Material")
	UMaterialInterface* CustomWaterMaterialDistant = nullptr;


	/** Only one body can have this enabled at a time.  Enabling will disable on all other bodies.  Setting modified at runtime (eventually this will be 100% transparent) */
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Water Simulation")
	bool bGeneratesInteractiveCaustics = false;	
	
	//UPROPERTY()
	bool F = false;
	
	/** Disable ripple interaction if you want*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation")
	bool bIsInteractive = true;

	/** Enable this to only support manual ripple creation.  Saves cost of rendering a scene capture each frame and drawing that to the interactivity which is too expensive on low end.  Overrides platform specific options.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Advanced Performance Options", AdvancedDisplay)
	bool bDisableAutomaticInteraction = false;

	/** Disable Auto Interaction on Xbox*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "!BDisableAutomaticInteraction"), Category = "UIWS Water Simulation\|Advanced Performance Options\|Platform Specific")
	bool bDisableAutomaticInteractionXbox = false;

	/** Disable Auto Interaction on PS4*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "!BDisableAutomaticInteraction"), Category = "UIWS Water Simulation\|Advanced Performance Options\|Platform Specific")
	bool bDisableAutomaticInteractionPS4 = false;

	/** Disable Auto Interaction on Nintendo Switch*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "!BDisableAutomaticInteraction"), Category = "UIWS Water Simulation\|Advanced Performance Options\|Platform Specific")
	bool bDisableAutomaticInteractionNintendoSwitch = true;

	/** Disable Auto Interaction on Android*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "!BDisableAutomaticInteraction"), Category = "UIWS Water Simulation\|Advanced Performance Options\|Platform Specific")
	bool bDisableAutomaticInteractionAndroid = true;

	/** Disable Auto Interaction on IOS*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "!BDisableAutomaticInteraction"), Category = "UIWS Water Simulation\|Advanced Performance Options\|Platform Specific")
	bool bDisableAutomaticInteractionIOS = true;

	/** Resolution of this water's automatic interaction scene capture.  Lower res is usually better quality.  Don't recommend higher values*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Advanced Performance Options")
	int32 CaptureRes = 256;

	/**Higher values lower the vertex count per meter.  Recommend adjusting alongisde the lod ratios in static mesh UIWSMesh16x16fbx */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Advanced Performance Options")
	int MaxTileScale = 3;

	/** Simulation render target resolution for body when it is the priority (ie, nearest, colliding body)*/
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation")
	//int32 iSimResMax = 1024;

	/** Simulation render target resolution for body when it is not the priority body.  To change the res of priority (up close) water go to UIWSContent>Materials>Simulation and modify the height and normal rts*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Advanced Performance Options")
	int32 SimResMin = 512;

	/** Highly not recommended.  Only enable if you're desperate for better performance and are willing to faff with sim settings to get it to look ok around target res*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Advanced Performance Options")
	bool bTieSimToFPS = false;

	/** Whether or not to use default plugin damage handling.  Disable if you don't want to use unreal engine damage systems.  To manually ripple the body call ApplyForceManual() from c++ or bp*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation", meta = (EditCondition = "bIsInteractive"))
	bool InteractOnDamage = true;
	/** Enable this to manually set tick rate.  Seems to have a negligible performance effect but might be useful of low spec hardware*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation", meta = (EditCondition = "bIsInteractive"))
	bool bLimitTickRate = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation", meta = (EditCondition = "bLimitTickRate"))
	float TickRate = 60;

	/** When using the automatic damage handling provided by the plugin, particle effects are automatically scaled between their smallest for a damage of 1, and their biggest for this amount.
	*	You might set this to be the maximum damage they expect a weapon to deal in your game.  Any damage dealt over this value will result in the same, maximum scaled damage effect.
	*	To override default damage functionality override in C++ or BP DamageSplashAtLocation()
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation")
	float DamageScale = 100;
	/** Whether the body supports ripple reflections.  Only supported on flat bodies with auto interaction enabled (not supported on rivers)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Ripple Reflection")
	bool bSupportsEdgeReflection = true;
	/*Depth we capture water edge intersections over.  If you have extremely shallow water you may want to lower this value, at cost of edge consistency*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Ripple Reflection")
	float EdgeDepth = 30.0f;
	/*Alternatively to edge depth, if you have shallow water you can increase this value to retain strong edge (for ripple bounce on thin objects) and capture interactions this distance above the water)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation\|Ripple Reflection")
	float CaptureOffset = 30.0f;
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Water Simulation")
	float VolumeDepth = 100;
	/** Primary water body color*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	FLinearColor WaterColor = FLinearColor(0.111649,0.125, 0.047573,1);
	/** Color for deepest water*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	FLinearColor DeepOpacity = FLinearColor(0,0,0,0);
	/** Color for shallowest water*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	FLinearColor ShallowOpacity = FLinearColor(1,1,1,0);
	/** The distance over which water color will transition from deep to shallow*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	float DepthTransitionDistance = 149.5;
	/** Multiply the water color by this amount to darken or brighten the underwater post process.*/
	//UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	float UnderWaterBrightnessMult = 0.5f;

	/** Distance over which to apply edge fade to water edges*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	float ShallowEdgeFadeDistance = 61.2;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Body Settings")
	float ShallowEdgeFadePower = 1;

	/** Whether or not to spawn particle effects when recieving damage.  If disabled water will still ripple*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles")
	bool bEnableParticleOnDamage = true;

	/** Still experimental and barebones.  It works but doesn't look great for most overlaps.*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles")
	bool bEnableParticleOnCollision = true;


	/** When enabled, you can replace default interaction and damage particle effects as well as the damage effect min and max scale (scaled with damage amount) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles")
	bool bOverrideParticleSettings = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	float InteractionEffectScaleMin = 0.3f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	float InteractionEffectScaleMax = 0.8f;
	/** Effect spawned on interaction (ie, footsteps, splashing, etc)*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	UParticleSystem* InteractionEffect;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	float DamageEffectScaleMin = 0.4f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	float DamageEffectScaleMax = 1.0f;
	/** Effect spawned when damage is taken (ie Mortal, bullet hit) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Interaction Particles\|Particle Overrides", meta = (EditCondition = "bOverrideParticleSettings"))
	UParticleSystem* DamageEffect;


	/** Apply force manually to the water body.  Location in world space.
	Strength Clamped beteween -10 and 10.  Percent should be between 0 and 1
	Can modify these clamps in material: UIWSForceSplatManual
	*/
	UFUNCTION(BlueprintCallable, Category = "UIWS Functions")
	virtual void ApplyForceAtLocation(float fStrength, float fSizePercent, FVector HitLocation, bool bWithEffect = false);

	/** Function you can call if you need to manually request priority for a water body (making it the high res + interactive caustic casting body).  
	Only one body can have priority at a time, and by default a body requests priority on overlap with player pawn 0*/
	UFUNCTION(BlueprintCallable, Category = "UIWS Functions")
	void RequestPriorityManual();

	//Under the hood variable used by the manager to keep track of this body and which caustics it contributes to
	int WaterBodyNum = 0;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void EditorApplyRotation(const FRotator& DeltaRotation, bool bAltDown, bool bShiftDown, bool bCtrlDown) override;
#endif
	virtual void BeginDestroy() override;

	/** Initialize all the visual params for the material and scaling etc.  This is called at all relevant times, so feel free to override and extend as required*/
	virtual void SetWaterVisualParams();


private:
/** Sim Variables*/
	int iHeightState = 0;
	float fTimeAccumulator = 0.0f;
	float fInteractivityDistance;
	float fUpdateRate = 60.0f;
	FVector LastInteractivityCenter;/*The last frame position of the player we center interactivity on */
	
	UPROPERTY()
	UStaticMeshComponent* WaterMeshComp;

	
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> hellmats;


	UPROPERTY()
	UTextureRenderTarget2D* activeheight0;
	UPROPERTY()
	UTextureRenderTarget2D* activeheight1;
	UPROPERTY()
	UTextureRenderTarget2D* activeheight2;
	UPROPERTY()
	UTextureRenderTarget2D* activenormal;

	UPROPERTY()
	UTextureRenderTarget2D* localheight0;
	UPROPERTY()
	UTextureRenderTarget2D* localheight1;
	UPROPERTY()
	UTextureRenderTarget2D* localheight2;
	UPROPERTY()
	UTextureRenderTarget2D* localnormal;

	UPROPERTY()
	UTextureRenderTarget2D* globalheight0;
	UPROPERTY()
	UTextureRenderTarget2D* globalheight1;
	UPROPERTY()
	UTextureRenderTarget2D* globalheight2;
	UPROPERTY()
	UTextureRenderTarget2D* globalnormal;

	UPROPERTY()
	UTextureRenderTarget2D* myCaptureRT;

	UPROPERTY()
	UMaterialInterface* ForceSplatMat;
	UPROPERTY()
	UMaterialInterface* ManForceSplatMat;
	UPROPERTY()
	UMaterialInterface* HeightSimMat;
	UPROPERTY()
	UMaterialInterface* ComputeNormalMat;



	UPROPERTY()
	UMaterialInterface* PPUnderWaterMat;

	UPROPERTY()
	UMaterialInstanceDynamic* PPUnderWaterMID;
	
	UPROPERTY()
	UParticleSystem* DefaultSplashEffect;


	FTimerHandle CullingCheckHandle;

	



	virtual void OnConstruction(const FTransform & Transform) override;

	virtual void CheckIfCulled();

protected:

	UPROPERTY()
	UMaterialInstanceDynamic* WaterMID;

	UPROPERTY()
	UMaterialInstanceDynamic* WaterMIDLOD1;
	UPROPERTY()
	UStaticMesh* WaterMeshSM;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "UIWS Water Simulation", AdvancedDisplay)
	UBoxComponent* WaterVolume;
	UPostProcessComponent* PostProcessComp;
	UMaterialParameterCollection* MPC_UIWSWaterBodies;

private:
	UPROPERTY()
	UMaterialInstanceDynamic* HeightSimInst;
	UPROPERTY()
	UMaterialInstanceDynamic* ForceSplatInst;
	UPROPERTY()
	UMaterialInstanceDynamic* ManualSplatInst;
	UPROPERTY()
	UMaterialInstanceDynamic* ComputeNormalInst;
	UPROPERTY()
	UCanvas* Canvas;
	UPROPERTY()
	FVector2D Size;
	UPROPERTY()
	FDrawToRenderTargetContext Context;


	UPROPERTY()
	UCanvas* CanvasMan;
	UPROPERTY()
	FVector2D SizeMan;
	UPROPERTY()
	FDrawToRenderTargetContext ContextMan;
};
