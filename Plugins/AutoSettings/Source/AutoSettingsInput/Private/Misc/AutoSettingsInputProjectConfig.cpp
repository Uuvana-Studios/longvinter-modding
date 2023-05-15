// Copyright Sam Bonifacio. All Rights Reserved.


#include "Misc/AutoSettingsInputProjectConfig.h"
#include "ConfigUtils.h"
#include "Engine/Texture.h"

UAutoSettingsInputProjectConfig::UAutoSettingsInputProjectConfig()
{
	if(HasAllFlags(RF_ClassDefaultObject))
	{
		FConfigUtils::MigrateConfigPropertiesFromSection(this, TEXT("/Script/AutoSettings.AutoSettingsConfig"));
	}
}

FText UAutoSettingsInputProjectConfig::GetKeyFriendlyNameStatic(FKey Key)
{
	return GetDefault<UAutoSettingsInputProjectConfig>()->GetKeyFriendlyName(Key);
}

FGameplayTag UAutoSettingsInputProjectConfig::GetKeyGroupStatic(FKey Key)
{
	return GetDefault<UAutoSettingsInputProjectConfig>()->GetKeyGroupOfKey(Key);
}

TArray<UTexture*> UAutoSettingsInputProjectConfig::LoadKeyIcons(FGameplayTagContainer KeyIconTags)
{
	TArray<UTexture*> LoadedTextures;
	for(const FKeyIconSet& KeyIconSet : GetDefault<UAutoSettingsInputProjectConfig>()->KeyIconSets)
	{
		if(KeyIconSet.Tags.HasAll(KeyIconTags))
		{
			for(const TPair<FKey, TSoftObjectPtr<UTexture>>& KeyIconPair : KeyIconSet.IconMap)
			{
				UTexture* LoadedTexture = KeyIconPair.Value.LoadSynchronous();
				if(IsValid(LoadedTexture))
				{
					LoadedTextures.Add(LoadedTexture);
				}
			}
		}
	}
	return LoadedTextures;
}

#if WITH_EDITOR
FText UAutoSettingsInputProjectConfig::GetSectionText() const
{
	return INVTEXT("Auto Settings Input");
}

FText UAutoSettingsInputProjectConfig::GetSectionDescription() const
{
	return INVTEXT("Configure the Auto Settings input mapping module");
}
#endif

void UAutoSettingsInputProjectConfig::PostInitProperties()
{
	Super::PostInitProperties();
	
	// Migrate deprecated properties
	
	for(FName& Action : BlacklistedActions_DEPRECATED)
	{
		PreservedActions.AddUnique(Action);
	}
	BlacklistedActions_DEPRECATED.Empty();
	
	for(FName& Axis : BlacklistedAxes_DEPRECATED)
	{
		PreservedAxes.AddUnique(Axis);
	}
	BlacklistedAxes_DEPRECATED.Empty();

	for(FAxisAssociation& AxisAssociation : AxisAssociations)
	{
		if(AxisAssociation.AnalogKey_DEPRECATED.IsValid() && !AxisAssociation.AxisKey.IsValid())
		{
			AxisAssociation.AxisKey = AxisAssociation.AnalogKey_DEPRECATED;
			AxisAssociation.AnalogKey_DEPRECATED = EKeys::Invalid;
		}
		if(AxisAssociation.BooleanKeys_DEPRECATED.Num() > 0 && AxisAssociation.ButtonKeys.Num() == 0)
		{
			AxisAssociation.ButtonKeys = AxisAssociation.BooleanKeys_DEPRECATED;
			AxisAssociation.BooleanKeys_DEPRECATED.Reset();
		}
	}

	for(FInputMappingPreset& Preset : InputPresets)
	{
		if(Preset.MappingGroups_DEPRECATED.Num() > 0 && Preset.InputLayout.MappingGroups.Num() == 0)
		{
			Preset.InputLayout.MappingGroups = Preset.MappingGroups_DEPRECATED;
			Preset.MappingGroups_DEPRECATED.Reset();
		}
	}

	for(FKeyIconSet& IconSet : KeyIconSets)
	{
		for(FKeyIconPair& Pair : IconSet.Icons_DEPRECATED)
		{
			if(IconSet.IconMap.Contains(Pair.Key_DEPRECATED))
			{
				continue;
			}
			IconSet.IconMap.Add(Pair.Key_DEPRECATED, Pair.Icon_DEPRECATED);
		}
	}
}
