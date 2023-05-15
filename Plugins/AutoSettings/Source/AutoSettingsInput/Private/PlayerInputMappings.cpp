// Copyright Sam Bonifacio. All Rights Reserved.

#include "PlayerInputMappings.h"
#include "Misc/AutoSettingsInputConfig.h"
#include "Misc/AutoSettingsInputLogs.h"
#include "Utility/InputMappingUtils.h"

FPlayerInputMappings::FPlayerInputMappings()
{
}

FPlayerInputMappings::FPlayerInputMappings(FString PlayerId, FGameplayTag InBasePresetTag, TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
	: PlayerId(PlayerId)
	, BasePresetTag(InBasePresetTag)
{
	SetConfig(InConfig);
	ApplyDefaultKeyGroup();
}

FPlayerInputMappings::FPlayerInputMappings(bool bInNullBasePreset, TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
	: FPlayerInputMappings(FString(), FGameplayTag(), InConfig)
{
	bNullBasePreset = bInNullBasePreset;
}

FPlayerInputMappings::FPlayerInputMappings(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
	: FPlayerInputMappings(FString(), FGameplayTag(), InConfig)
{
}

void FPlayerInputMappings::SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
{
	Config = InConfig;
	MappingOverrides.SetConfig(Config);
}

void FPlayerInputMappings::ApplyDefaultKeyGroup()
{
	const TArray<FKeyGroup>& KeyGroups = Config->GetKeyGroups();
	if (KeyGroups.IsValidIndex(0))
	{
		PlayerKeyGroup = KeyGroups[0].KeyGroupTag;
	}
}

void FPlayerInputMappings::AddActionOverride(const FInputActionKeyMapping& NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	UE_LOG(LogAutoSettingsInput, Verbose, TEXT("AddActionOverride: %s, MappingGroup: %i, bAnykeyGroup: %i"), *ToString(NewMapping), MappingGroup, (int32)bAnyKeyGroup);
	
	const FInputMappingLayout BasePresetLayout = GetBasePresetMappings();
	const bool bBasePresetHasMapping = BasePresetLayout.HasMappingGroup(MappingGroup)
    && BasePresetLayout.GetMappingGroupConst(MappingGroup).GetAllActions(NewMapping.ActionName, FGameplayTag()).ContainsByPredicate([NewMapping](const FInputActionKeyMapping& BaseMapping)
    {
        return BaseMapping == NewMapping;
    });

	FConfigActionKeyMapping ConfigMapping = NewMapping;
	ConfigMapping.bIsDefault = bBasePresetHasMapping;
	
	// Resolve current final merged layout and calculate mappings that would be unbound from it
	FInputMappingLayout MergedLayout = BuildMergedMappingLayout();
	const FInputMappingLayout UnboundMappings = MergedLayout.ReplaceAction(ConfigMapping, MappingGroup, bAnyKeyGroup);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("UnboundMappings:"));
		UnboundMappings.DumpToLog();
	}

	// Store new mapping and any resulting unbound mappings in the mapping overrides
	MappingOverrides.MergeUnboundMappings(UnboundMappings);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after MergeUnboundMappings:"));
		MappingOverrides.DumpToLog();
	}
	
	MappingOverrides.ReplaceAction(ConfigMapping, MappingGroup, bAnyKeyGroup);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after ReplaceAction:"));
		MappingOverrides.DumpToLog();
	}

	MappingOverrides.RemoveRedundantMappings(BasePresetLayout);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after RemoveRedundantMappings:"));
		MappingOverrides.DumpToLog();
	}
}

void FPlayerInputMappings::AddAxisOverride(const FInputAxisKeyMapping& NewMapping, int32 MappingGroup, bool bAnyKeyGroup)
{
	UE_LOG(LogAutoSettingsInput, Verbose, TEXT("AddAxisOverride: %s, MappingGroup: %i, bAnykeyGroup: %i"), *ToString(NewMapping), MappingGroup, (int32)bAnyKeyGroup);
	
	const FInputMappingLayout BasePresetLayout = GetBasePresetMappings();
	const bool bBasePresetHasMapping = BasePresetLayout.HasMappingGroup(MappingGroup)
    && BasePresetLayout.GetMappingGroupConst(MappingGroup).GetAllAxes(NewMapping.AxisName, NewMapping.Scale, FGameplayTag()).ContainsByPredicate([NewMapping](const FInputAxisKeyMapping& BaseMapping)
    {
        return BaseMapping == NewMapping;
    });

	FConfigAxisKeyMapping ConfigMapping = NewMapping;
	ConfigMapping.bIsDefault = bBasePresetHasMapping;

	// Resolve current final merged layout and calculate mappings that would be unbound from it
	FInputMappingLayout MergedLayout = BuildMergedMappingLayout();
	const FInputMappingLayout UnboundMappings = MergedLayout.ReplaceAxis(ConfigMapping, MappingGroup, bAnyKeyGroup);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("UnboundMappings:"));
		UnboundMappings.DumpToLog();
	}

	// Store new mapping and any resulting unbound mappings in the mapping overrides
	MappingOverrides.MergeUnboundMappings(UnboundMappings);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after MergeUnboundMappings:"));
		MappingOverrides.DumpToLog();
	}
	
	MappingOverrides.ReplaceAxis(ConfigMapping, MappingGroup, bAnyKeyGroup);
	
	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after ReplaceAxis:"));
		MappingOverrides.DumpToLog();
	}

	MappingOverrides.RemoveRedundantMappings(BasePresetLayout);

	if(UE_LOG_ACTIVE(LogAutoSettingsInput, Verbose))
	{
		UE_LOG(LogAutoSettingsInput, Verbose, TEXT("MappingOverrides after RemoveRedundantMappings:"));
		MappingOverrides.DumpToLog();
	}
}

FInputMappingLayout FPlayerInputMappings::BuildMergedMappingLayout(bool bDebugLog) const
{
    // Get base preset mappings
	FInputMappingLayout BasePresetMappings = GetBasePresetMappings();

	if(bDebugLog)
	{
		UE_LOG(LogAutoSettingsInput, Display, TEXT("Base preset:"));
		BasePresetMappings.DumpToLog();
		UE_LOG(LogAutoSettingsInput, Display, TEXT("Overrides:"));
		MappingOverrides.DumpToLog();
	}

    // Apply player input overrides on top
	BasePresetMappings.MergeUnboundMappings(MappingOverrides);

	if(bDebugLog)
	{
		UE_LOG(LogAutoSettingsInput, Display, TEXT("Merge unbound:"));
		BasePresetMappings.DumpToLog();
	}
	
	BasePresetMappings.ApplyUnboundMappings();

	if(bDebugLog)
	{
		UE_LOG(LogAutoSettingsInput, Display, TEXT("Apply unbound:"));
		BasePresetMappings.DumpToLog();
	}
	
    BasePresetMappings.MergeMappings(MappingOverrides);

	if(bDebugLog)
	{
		UE_LOG(LogAutoSettingsInput, Display, TEXT("Merge overrides:"));
		BasePresetMappings.DumpToLog();
	}

    return BasePresetMappings;
}

FInputMappingLayout FPlayerInputMappings::GetBasePresetMappings() const
{
	// Use empty base preset if applicable
    if(bNullBasePreset)
    {
	    return FInputMappingLayout(Config);
    }

	// Look up specified preset or default from config
	return Config->GetInputPresetByTag(BasePresetTag).InputLayout;
}

void FPlayerInputMappings::Apply(APlayerController* PlayerController)
{
    if(!ensure(PlayerController))
    {
        return;
    }

    // Apply merged mappings to player controller
    BuildMergedMappingLayout().Apply(PlayerController);
}

void FPlayerInputMappings::MigrateDeprecatedProperties()
{
	if (PlayerIndex_DEPRECATED >= 0 && PlayerId.IsEmpty())
	{
		PlayerId = FString::FromInt(PlayerIndex_DEPRECATED);
		PlayerIndex_DEPRECATED = FPlayerInputMappings().PlayerIndex_DEPRECATED;
	}

	if (Preset_DEPRECATED.MappingGroups_DEPRECATED.Num() > 0 && MappingOverrides.GetTotalNumInputDefinitions() == 0)
	{
		BasePresetTag = Preset_DEPRECATED.PresetTag;

		// Migrate to new overrides based on old mappings
		
		// Find any mappings on base preset that were unbound in old mappings
		const FInputMappingLayout OldMappings = FInputMappingLayout(Preset_DEPRECATED.MappingGroups_DEPRECATED, Config);
		const FInputMappingLayout PresetMappings = Config->GetInputPresetByTag(BasePresetTag).InputLayout;
		FInputMappingLayout UnboundMappings = OldMappings.FindUnboundMappings(PresetMappings);
		
		// Apply the old mappings on top of the unbound ones
		MappingOverrides = UnboundMappings.MergeMappings(OldMappings);
		
		// Remove any redundant overrides that are already on the preset
		MappingOverrides.RemoveRedundantMappings(PresetMappings);

		Preset_DEPRECATED = FInputMappingPreset();
	}
}
