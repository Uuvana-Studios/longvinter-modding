// Copyright Sam Bonifacio. All Rights Reserved.

#include "InputMappingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Utility/InputMappingUtils.h"
#include "Misc/AutoSettingsInputLogs.h"
#include "AutoSettingsPlayer.h"
#include "ConfigUtils.h"
#include "Misc/AutoSettingsInputProjectConfig.h"

static FAutoConsoleCommand DumpPlayersCommand(
	TEXT("AutoSettings.Input.DumpPlayers"),
	TEXT("Dumps registered player controllers to log with their input mappings"),
	FConsoleCommandDelegate::CreateStatic(UInputMappingManager::DumpPlayers),
	ECVF_Default);

static FAutoConsoleCommand TestLayoutMergeCommand(
    TEXT("AutoSettings.Input.TestLayoutMerge"),
    TEXT("Run a layout merge and log each step"),
    FConsoleCommandDelegate::CreateStatic(UInputMappingManager::TestLayoutMerge),
    ECVF_Default);

static TAutoConsoleVariable<int32> CVarDebugMode(
	TEXT("AutoSettings.Input.Debug"),
	0,
	TEXT("Dump all input mappings whenever they are modified"),
	ECVF_RenderThreadSafe);


UInputMappingManager* UInputMappingManager::Singleton = nullptr;

UInputMappingManager::UInputMappingManager()
{
	if(HasAllFlags(RF_ClassDefaultObject))
	{
		FConfigUtils::MigrateConfigPropertiesFromSection(this, TEXT("/Script/AutoSettings.InputMappingManager"));
	}
}

UInputMappingManager* UInputMappingManager::Get()
{
	if (!Singleton)
	{
		Singleton = NewObject<UInputMappingManager>();
		Singleton->AddToRoot();
	}

	return Singleton;
}

TArray<FInputMappingPreset> UInputMappingManager::GetDefaultInputPresets()
{
	return GetDefault<UAutoSettingsInputProjectConfig>()->GetInputPresets();
}

FPlayerInputMappings UInputMappingManager::GetPlayerInputMappingsStatic(APlayerController * Player)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Get Player Input Mappings"))
	{
		return FPlayerInputMappings();
	}
	
	return Get()->FindPlayerInputMappings(Player);
}

void UInputMappingManager::SetPlayerInputPresetStatic(APlayerController * Player, FInputMappingPreset Preset)
{
	Get()->SetPlayerInputPreset(Player, Preset);
}

void UInputMappingManager::SetPlayerInputPresetByTag(APlayerController* Player, FGameplayTag PresetTag)
{
	Get()->SetPlayerInputPreset(Player, PresetTag);
}

void UInputMappingManager::SetPlayerKeyGroupStatic(APlayerController * Player, FGameplayTag KeyGroup)
{
	Get()->SetPlayerKeyGroup(Player, KeyGroup);
}

void UInputMappingManager::AddPlayerActionOverrideStatic(APlayerController * Player, const FInputActionKeyMapping & NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	Get()->AddPlayerActionOverride(Player, NewMapping, MappingGroup, bAnyKeyGroup);
}

void UInputMappingManager::AddPlayerAxisOverrideStatic(APlayerController * Player, const FInputAxisKeyMapping & NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	Get()->AddPlayerAxisOverride(Player, NewMapping, MappingGroup, bAnyKeyGroup);
}

bool UInputMappingManager::InitializePlayerInputOverridesStatic(APlayerController * Player)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Initialize Player Input Overrides"))
	{
		return false;
	}

	if (!ensure(Player->PlayerInput))
	{
		return false;
	}

	Get()->RegisterPlayerController(Player);

	return true;
}

FInputActionKeyMapping UInputMappingManager::GetPlayerActionMappingStatic(APlayerController* Player, FName ActionName,
	int32 MappingGroup)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Get Player Action Mapping"))
	{
		return FInputActionKeyMapping();
	}
	
	return Get()->GetPlayerActionMapping(Player, ActionName, MappingGroup, FGameplayTag(), true);
}

FInputAxisKeyMapping UInputMappingManager::GetPlayerAxisMappingStatic(APlayerController* Player, FName AxisName,
	float Scale, int32 MappingGroup)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Get Player Axis Mapping"))
	{
		return FInputAxisKeyMapping();
	}
	
	return Get()->GetPlayerAxisMapping(Player, AxisName, Scale, MappingGroup, FGameplayTag(), true);
}

const UAutoSettingsInputConfig* UInputMappingManager::GetInputConfigStatic()
{
	return Get()->GetInputConfig();
}

void DumpLayout(const FInputMappingLayout& Layout)
{
	Layout.DumpToLog();
}

void UInputMappingManager::DumpPlayers()
{
	UE_LOG(LogAutoSettingsInput, Display, TEXT("----- DumpPlayers -----"));
	UInputMappingManager* Instance = Get();
	for (int i = 0; i < Instance->RegisteredPlayerControllers.Num(); i++)
	{
		APlayerController* PC = Instance->RegisteredPlayerControllers[i];

		UE_LOG(LogAutoSettingsInput, Display, TEXT("PlayerController %i: "), i);
		if (IsValid(PC))
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Object name: %s"), *PC->GetName());
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Human readable name: %s"), *PC->GetHumanReadableName());
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Implements IAutoSettingsPlayer: %i"), (int32)PC->Implements<UAutoSettingsPlayer>());
			FPlayerInputMappings InputMappings = Instance->FindPlayerInputMappings(PC);
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Player ID (if applicable): %s"), InputMappings.PlayerId.IsEmpty() ? TEXT("EMPTY") : *InputMappings.PlayerId);
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Key Group: %s"), *InputMappings.PlayerKeyGroup.ToString());
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Base Preset Tag: %s"), *InputMappings.BasePresetTag.ToString());
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Custom Mappings: %i"), (int32)InputMappings.MappingOverrides.GetTotalNumInputDefinitions());
			DumpLayout(InputMappings.MappingOverrides);
			FInputMappingLayout MergedLayout = InputMappings.BuildMergedMappingLayout();
			UE_LOG(LogAutoSettingsInput, Display, TEXT("    Merged Mappings: %i"), (int32)MergedLayout.GetTotalNumInputDefinitions());
			DumpLayout(MergedLayout);
		}
		else
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("INVALID"));
		}
	}
	UE_LOG(LogAutoSettingsInput, Display, TEXT("----- End DumpPlayers -----"));
}

void UInputMappingManager::TestLayoutMerge()
{
	UE_LOG(LogAutoSettingsInput, Display, TEXT("----- TestLayoutMerge -----"));
	UInputMappingManager* Instance = Get();
	APlayerController* PC = Instance->RegisteredPlayerControllers[0];
	if(ensure(PC))
	{
		const FPlayerInputMappings InputMappings = Instance->FindPlayerInputMappings(PC);
		InputMappings.BuildMergedMappingLayout(true);
	}
	UE_LOG(LogAutoSettingsInput, Display, TEXT("----- End TestLayoutMerge -----"));
}

void UInputMappingManager::SetPlayerKeyGroup(APlayerController* Player, FGameplayTag KeyGroup)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Set Player Key Group"))
	{
		return;
	}
	
	FPlayerInputMappings InputMappings = FindPlayerInputMappings(Player);

	if(InputMappings.PlayerKeyGroup == KeyGroup)
	{
		// No-op, bail to avoid save
		return;
	}
	
	InputMappings.PlayerKeyGroup = KeyGroup;

	SavePlayerInputMappings(Player, InputMappings);

	// Even though the actual mappings haven't changed it all, broadcast so that widgets that do care about the new value can update
	Get()->OnMappingsChanged.Broadcast(Player);
}

void UInputMappingManager::AddPlayerActionOverride(APlayerController * Player, const FInputActionKeyMapping& NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Add Player Action Override"))
	{
		return;
	}
	
	UE_LOG(LogAutoSettingsInput, Log, TEXT("InputMappingManager: Adding action override: %s"), *NewMapping.ActionName.ToString());

	FPlayerInputMappings PlayerInputMappings = FindPlayerInputMappings(Player);

	PlayerInputMappings.AddActionOverride(NewMapping, MappingGroup, bAnyKeyGroup);

	// Resolve and apply new final merged layout
	PlayerInputMappings.Apply(Player);

	SavePlayerInputMappings(Player, PlayerInputMappings);

	OnMappingsChanged.Broadcast(Player);

	if (CVarDebugMode->GetBool())
	{
		DumpPlayers();
	}
}

void UInputMappingManager::AddPlayerAxisOverride(APlayerController* Player, const FInputAxisKeyMapping& NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Add Player Axis Override"))
	{
		return;
	}
	
	UE_LOG(LogAutoSettingsInput, Log, TEXT("InputMappingManager: Adding axis override: %s, Scale: %f"), *NewMapping.AxisName.ToString(), NewMapping.Scale);

	FPlayerInputMappings PlayerInputMappings = FindPlayerInputMappings(Player);
	
	PlayerInputMappings.AddAxisOverride(NewMapping, MappingGroup, bAnyKeyGroup);
	
	// Resolve and apply new final merged layout
	PlayerInputMappings.Apply(Player);

	SavePlayerInputMappings(Player, PlayerInputMappings);

	OnMappingsChanged.Broadcast(Player);

	if(CVarDebugMode->GetBool())
	{
		DumpPlayers();
	}
}

FInputActionKeyMapping UInputMappingManager::GetPlayerActionMapping(APlayerController* Player, FName ActionName, int32 MappingGroup, FGameplayTag KeyGroup, bool bUsePlayerKeyGroup) const
{
	FPlayerInputMappings InputOverride;
	if (Player)
	{
		InputOverride = FindPlayerInputMappings(Player);
	}
	else
	{
		InputOverride = FPlayerInputMappings(GetDefault<UAutoSettingsInputProjectConfig>()->AsWeakInterfacePtrConst());
	}

	if (bUsePlayerKeyGroup)
		KeyGroup = InputOverride.PlayerKeyGroup;

	FInputMappingLayout MergedMappingLayout = InputOverride.BuildMergedMappingLayout();

	// Use the first mapping group with the specified key group
	if (MappingGroup == -1)
	{
		for (FInputMappingGroup& MappingGroupObj : MergedMappingLayout.GetMappingGroups())
		{
			FInputActionKeyMapping Mapping = MappingGroupObj.GetAction(ActionName, KeyGroup);
			if (Mapping.Key.IsValid())
				return Mapping;
		}
		return FInputActionKeyMapping();
	}

	if (MergedMappingLayout.GetMappingGroups().IsValidIndex(MappingGroup))
		return MergedMappingLayout.GetMappingGroups()[MappingGroup].GetAction(ActionName, KeyGroup);
	else
		return FInputActionKeyMapping();
}

FInputAxisKeyMapping UInputMappingManager::GetPlayerAxisMapping(APlayerController * Player, FName AxisName, float Scale, int32 MappingGroup, FGameplayTag KeyGroup, bool bUsePlayerKeyGroup) const
{
	FPlayerInputMappings InputOverride;
	if (Player)
	{
		InputOverride = FindPlayerInputMappings(Player);
	}
	else
	{
		InputOverride = FPlayerInputMappings(GetDefault<UAutoSettingsInputProjectConfig>()->AsWeakInterfacePtrConst());
	}

	if (bUsePlayerKeyGroup)
		KeyGroup = InputOverride.PlayerKeyGroup;

	FInputMappingLayout MergedMappingLayout = InputOverride.BuildMergedMappingLayout();

	// Use the first mapping group with the specified key group
	if (MappingGroup == -1)
	{
		for (FInputMappingGroup& MappingGroupObj : MergedMappingLayout.GetMappingGroups())
		{
			FInputAxisKeyMapping Mapping = MappingGroupObj.GetAxis(AxisName, Scale, KeyGroup);
			if (Mapping.Key.IsValid())
				return Mapping;
		}
		return FInputAxisKeyMapping();
	}

	if (MergedMappingLayout.GetMappingGroups().IsValidIndex(MappingGroup))
		return MergedMappingLayout.GetMappingGroups()[MappingGroup].GetAxis(AxisName, Scale, KeyGroup);
	else
		return FInputAxisKeyMapping();
}

void UInputMappingManager::SetPlayerInputPreset(APlayerController * Player, FInputMappingPreset Preset)
{
	if(!FInputMappingUtils::IsValidPlayer(Player, true, "Set Player Input Preset"))
	{
		return;
	}
	
	const FString PresetTag = Preset.PresetTag.IsValid() ? Preset.PresetTag.ToString() : "Invalid";
	UE_LOG(LogAutoSettingsInput, Log, TEXT("Setting input preset for '%s', tag: %s"), *Player->GetHumanReadableName(), *PresetTag);
	
	if (!RegisteredPlayerControllers.Contains(Player))
		RegisterPlayerController(Player);

	FPlayerInputMappings InputOverride = FindPlayerInputMappings(Player);
	InputOverride.BasePresetTag = Preset.PresetTag;
	InputOverride.MappingOverrides = FInputMappingLayout();
	InputOverride.Apply(Player);

	SavePlayerInputMappings(Player, InputOverride);

	OnMappingsChanged.Broadcast(Player);
}

void UInputMappingManager::SetPlayerInputPreset(APlayerController* Player, FGameplayTag PresetTag)
{
	TArray<FInputMappingPreset> Presets = GetDefaultInputPresets();

	FInputMappingPreset* FoundPreset = Presets.FindByPredicate([PresetTag](FInputMappingPreset Preset) { return Preset.PresetTag == PresetTag; });
	if (FoundPreset)
	{
		SetPlayerInputPreset(Player, *FoundPreset);
	}
}

const UAutoSettingsInputConfig* UInputMappingManager::GetInputConfig() const
{
	return GetDefault<UAutoSettingsInputProjectConfig>();
}

void UInputMappingManager::PostInitProperties()
{
	Super::PostInitProperties();

	// Migrate deprecated properties

	for(FPlayerInputMappings& PlayerInput : PlayerInputOverrides)
	{
		PlayerInput.MigrateDeprecatedProperties();
	}
}

UWorld* UInputMappingManager::GetGameWorld() const
{
	UWorld* TestWorld = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) && (Context.World() != nullptr))
		{
			TestWorld = Context.World();
			break;
		}
	}

	return TestWorld;
}

void UInputMappingManager::RegisterPlayerController(APlayerController* Player)
{
	if (!ensure(IsValid(Player) && IsValid(Player->PlayerInput)))
	{
		return;
	}

	UE_LOG(LogAutoSettingsInput, Log, TEXT("Registering input overrides for %s"), *Player->GetHumanReadableName());

	// Add input overrides to a fresh player controller
	// Note: Players can be "re-registered" without first being unregistered
	// Logic here should refresh anything applicable but avoid duplication
	// Might be worth creating an "unregister" for this case

	// First find existing mappings (from config, or old PlayerControllers for the same player) or default
	FPlayerInputMappings InputOverride = FindPlayerInputMappings(Player);

	// Migrate deprecated properties - this needs to happen both when loading from config or from custom saves
	InputOverride.MigrateDeprecatedProperties();

	// Consolidate changes to base preset if any
	InputOverride.MappingOverrides.ConsolidateDefaultChanges(InputOverride.GetBasePresetMappings());

	SavePlayerInputMappings(Player, InputOverride);

	// Replace player mappings
	InputOverride.Apply(Player);
	RegisteredPlayerControllers.AddUnique(Player);

	Player->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnRegisteredPlayerControllerDestroyed);

	// Broadcast events
	OnMappingsChanged.Broadcast(Player);
}

FPlayerInputMappings UInputMappingManager::FindPlayerInputMappings(APlayerController* Player) const
{
	if (!ensure(Player))
	{
		return FPlayerInputMappings();
	}

	FPlayerInputMappings FoundMappings;
	bool bFound = IAutoSettingsPlayer::GetInputMappings(Player, FoundMappings);

	if (bFound)
	{
		UE_LOG(LogAutoSettingsInput, VeryVerbose, TEXT("Found input mappings for %s with via IAutoSettingsPlayer::GetInputMappings"), *Player->GetHumanReadableName());
	}

	// Player ID string to compare
	const FString PlayerIdString = IAutoSettingsPlayer::GetUniquePlayerIdentifier(Player);

	if(!bFound)
	{
		UE_LOG(LogAutoSettingsInput, VeryVerbose, TEXT("Checking internal mappings for %s with ID %s"), *Player->GetHumanReadableName(), *PlayerIdString);

		for (FPlayerInputMappings PlayerInputMapping : PlayerInputOverrides)
		{
			// Compare ID strings
			if (PlayerIdString == PlayerInputMapping.PlayerId)
			{
				UE_LOG(LogAutoSettingsInput, VeryVerbose, TEXT("Found existing input mappings"));
				FoundMappings = PlayerInputMapping;
				bFound = true;
			}
		}
	}

	if(bFound)
	{
		FoundMappings.SetConfig(GetDefault<UAutoSettingsInputProjectConfig>()->AsWeakInterfacePtrConst());
		return FoundMappings;
    }

	// Make default
	return FPlayerInputMappings(PlayerIdString, IAutoSettingsPlayer::GetDefaultInputMappingPreset(Player).PresetTag, GetDefault<UAutoSettingsInputProjectConfig>()->AsWeakInterfacePtrConst());
}

void UInputMappingManager::SavePlayerInputMappings(APlayerController* Player, FPlayerInputMappings& NewMappings)
{
	UE_LOG(LogAutoSettingsInput, Log, TEXT("Saving input overrides for %s"), *Player->GetHumanReadableName());
	
	// Remove existing mappings from config with that ID
	PlayerInputOverrides.RemoveAll([NewMappings](const FPlayerInputMappings& ExistingMappings)
	{
		return ExistingMappings.PlayerId == NewMappings.PlayerId;
	});

	// Add to config
	PlayerInputOverrides.Add(NewMappings);
	SaveConfig();

	if(!ensure(IsValid(Player)))
	{
		return;
	}

	// Custom save
	IAutoSettingsPlayer::SaveInputMappings(Player, NewMappings);
}

void UInputMappingManager::OnRegisteredPlayerControllerDestroyed(AActor* DestroyedActor)
{
	APlayerController* PlayerController = Cast<APlayerController>(DestroyedActor);

	if (!ensure(PlayerController))
	{
		return;
	}

	UE_LOG(LogAutoSettingsInput, Verbose, TEXT("Registered Player Controller %s for player %s destroyed"), *PlayerController->GetName(), *PlayerController->GetHumanReadableName());

	// Unregister
	RegisteredPlayerControllers.Remove(PlayerController);
	PlayerController->OnDestroyed.RemoveDynamic(this, &ThisClass::OnRegisteredPlayerControllerDestroyed);
}
