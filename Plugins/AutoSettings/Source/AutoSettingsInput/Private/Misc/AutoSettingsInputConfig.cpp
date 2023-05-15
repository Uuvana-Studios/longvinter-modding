// Copyright Sam Bonifacio. All Rights Reserved.

#include "Misc/AutoSettingsInputConfig.h"
#include "InputMappingPreset.h"
#include "GameFramework/InputSettings.h"
#include "Misc/AutoSettingsInputLogs.h"

UTexture* FKeyIconSet::GetIcon(FKey Key) const
{
	if(!IconMap.Contains(Key))
	{
		return nullptr;
	}

	UTexture* FoundIcon = IconMap.Find(Key)->LoadSynchronous();
	
	if(!FoundIcon)
	{
		UE_LOG(LogAutoSettingsInput, Warning, TEXT("FKeyIconSet::GetIcon: Key icon defined but empty - Key: %s, tags: %s"), *Key.ToString(), *Tags.ToString());
	}

	return FoundIcon;
}

UAutoSettingsInputConfig::UAutoSettingsInputConfig()
	: bAutoInitializePlayerInputOverrides(true)
	, AllowModifierKeys(true)
	, AllowMultipleBindingsPerKey(false)
{
	CategoryName = "Plugins";
	
	ShiftModifierOverrideText = NSLOCTEXT("AutoSettings", "ShiftKey", "Shift");
	CtrlModifierOverrideText = NSLOCTEXT("AutoSettings", "CtrlKey", "Ctrl");
	AltModifierOverrideText = NSLOCTEXT("AutoSettings", "AltKey", "Alt");
	CmdModifierOverrideText = NSLOCTEXT("AutoSettings", "CmdKey", "Cmd");

	// Gamepad stick associations
	AxisAssociations.Add(FAxisAssociation(EKeys::Gamepad_LeftX, { FKeyScale(EKeys::Gamepad_LeftStick_Right), FKeyScale(EKeys::Gamepad_LeftStick_Left, -1.f) }));
	AxisAssociations.Add(FAxisAssociation(EKeys::Gamepad_LeftY, { FKeyScale(EKeys::Gamepad_LeftStick_Up), FKeyScale(EKeys::Gamepad_LeftStick_Down, -1.f) }));
	AxisAssociations.Add(FAxisAssociation(EKeys::Gamepad_RightX, { FKeyScale(EKeys::Gamepad_RightStick_Right), FKeyScale(EKeys::Gamepad_RightStick_Left, -1.f) }));
	// This is intentionally inverted, due to how the axis values are inverted when they are provided
	AxisAssociations.Add(FAxisAssociation(EKeys::Gamepad_RightY, { FKeyScale(EKeys::Gamepad_RightStick_Up, -1.f), FKeyScale(EKeys::Gamepad_RightStick_Down) }));

	// Mouse axis associations
	AxisAssociations.Add(FAxisAssociation(EKeys::MouseX, { FKeyScale(EKeys::MouseX), FKeyScale(EKeys::MouseX, -1.f) }));
	AxisAssociations.Add(FAxisAssociation(EKeys::MouseY, { FKeyScale(EKeys::MouseY), FKeyScale(EKeys::MouseY, -1.f) }));
}

TArray<FInputMappingPreset> UAutoSettingsInputConfig::GetInputPresets() const
{
	TArray<FInputMappingPreset> Result = InputPresets;

	for(FInputMappingPreset& Preset : Result)
	{
		Preset.SetConfig(AsWeakInterfacePtrConst());
	}
	
	if (InputPresets.Num() < 1)
	{
		// Add default if no presets
		Result.Add(FInputMappingPreset(FGameplayTag::EmptyTag, true, AsWeakInterfacePtrConst()));
	}

	const TArray<FInputActionKeyMapping> DefaultActions = GetDefault<UInputSettings>()->GetActionMappings();
	const TArray<FInputAxisKeyMapping> DefaultAxis = GetDefault<UInputSettings>()->GetAxisMappings();

	for (FInputMappingPreset& Preset : Result)
	{
		// Add all default mappings first
		if (Preset.bUseDefaultMappings)
		{
			Preset.InputLayout.SetMappings(DefaultActions, DefaultAxis);
		}
		Preset.InputLayout.MarkAllMappingsDefault();
	}

	return Result;
}

UTexture* GetIcon(FKey Key, FKey AxisButtonKey, const FKeyIconSet& Set)
{
	// If the axis button key is valid, check it first
	if(AxisButtonKey.IsValid())
	{
		UTexture* AxisButtonTexture = Set.GetIcon(AxisButtonKey);
		if(AxisButtonTexture)
		{
			return AxisButtonTexture;
		}
	}

	return Set.GetIcon(Key);
}

UTexture* UAutoSettingsInputConfig::GetIconForKey(FKey InKey, FGameplayTagContainer Tags, float AxisScale) const
{
	UTexture* Result;

	// Retrieve axis button key if there is one, which may have a better icon
	const FKey AxisButton = GetAxisButton(InKey, AxisScale);
	
	// First check if an icon matches input tags exactly
	for (const FKeyIconSet& Set : KeyIconSets)
	{
		if (Set.Tags == Tags)
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
				return Result;
		}
	}

	// Check if an icon contains all input tags
	for (const FKeyIconSet& Set : KeyIconSets)
	{
		if (Set.Tags.DoesTagContainerMatch(Tags, EGameplayTagMatchType::Explicit, EGameplayTagMatchType::Explicit, EGameplayContainerMatchType::All))
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
				return Result;
		}
	}

	// Check if an icon contains any of the input tags
	for (const FKeyIconSet& Set : KeyIconSets)
	{
		if (Set.Tags.DoesTagContainerMatch(Tags, EGameplayTagMatchType::Explicit, EGameplayTagMatchType::Explicit, EGameplayContainerMatchType::Any))
		{
			Result = GetIcon(InKey, AxisButton, Set);
			if (Result)
				return Result;
		}
	}

	// Fall back to first icon we can find
	for (const FKeyIconSet& Set : KeyIconSets)
	{
		Result = GetIcon(InKey, AxisButton, Set);
		if (Result)
			return Result;
	}

	return nullptr;
}

FText UAutoSettingsInputConfig::GetKeyFriendlyName(FKey Key) const
{
	// There seems to be some issues where setting FKey of None in BP
	// then returns a display name of "(" and stuff like that, so just manually
	// handle case of none key
	if (!Key.IsValid())
		return FText::FromString(FString(TEXT("None")));

	for (const FKeyFriendlyName& KeyFriendlyName : KeyFriendlyNames)
	{
		if (KeyFriendlyName.Key == Key)
		{
			return KeyFriendlyName.FriendlyName;
		}
	}
	return Key.GetDisplayName();
}

bool UAutoSettingsInputConfig::DoesKeyGroupContainKey(FGameplayTag KeyGroupTag, FKey Key) const
{
	for (FKeyGroup KeyGroup : KeyGroups)
	{
		if (KeyGroup.KeyGroupTag == KeyGroupTag)
		{
			return KeyGroup.Contains(Key);
		}
	}

	return false;
}

bool UAutoSettingsInputConfig::SameKeyGroup(FKey KeyA, FKey KeyB) const
{
	return GetKeyGroupOfKey(KeyA) == GetKeyGroupOfKey(KeyB);
}

FKeyScale UAutoSettingsInputConfig::GetAxisKey(FKey InButtonKey) const
{
	for (FAxisAssociation AxisAssociation : AxisAssociations)
	{
		for (FKeyScale ButtonKey : AxisAssociation.ButtonKeys)
		{
			if (ButtonKey.Key == InButtonKey)
			{
				return FKeyScale(AxisAssociation.AxisKey, ButtonKey.Scale);
			}
		}
	}

	return FKeyScale();
}

FKey UAutoSettingsInputConfig::GetAxisButton(FKey AxisKey, float AxisScale) const
{
	const FAxisAssociation* FoundAssociation = AxisAssociations.FindByPredicate([AxisKey](const FAxisAssociation Association)
	{
		return Association.AxisKey == AxisKey;
	});

	if(!FoundAssociation)
	{
		return EKeys::Invalid;
	}

	const FKeyScale* FoundKeyScale = FoundAssociation->ButtonKeys.FindByPredicate([AxisScale](const FKeyScale KeyScale)
	{
		return KeyScale.Scale == AxisScale;
	});

	if(!FoundKeyScale)
	{
		return EKeys::Invalid;
	}

	return FoundKeyScale->Key;
}


bool UAutoSettingsInputConfig::IsKeyAllowed(FKey Key) const
{
	if (AllowedKeys.Num() > 0)
	{
		bool bHasAllowedKey = false;

		for (FKey AllowedKey : AllowedKeys)
		{
			if (Key == AllowedKey)
			{
				bHasAllowedKey = true;
				break;
			}
		}

		if (!bHasAllowedKey)
		{
			// Whitelist populated but doesn't have key, therefore key is disallowed
			return false;
		}
	}

	// Disallow if key on blacklist

	for (FKey DisallowedKey : DisallowedKeys)
	{
		if (Key == DisallowedKey)
		{
			return false;
		}
	}

	// Passed both whitelist and blacklist, key is allowed
	return true;
}

bool UAutoSettingsInputConfig::ShouldBindingsBeUniqueBetweenMappingGroups(int32 MappingGroupA, int32 MappingGroupB) const
{
	// Always enforce unique if not allowing multiple bindings per key
	if (!AllowMultipleBindingsPerKey)
	{
		return true;
	}

	// If AllowMultipleBindingsPerKey is true, check if both the mapping groups are linked
	// This also considers if both mapping groups are the same, and are contained in any link

	for (const FMappingGroupLink& Link : MappingGroupLinks)
	{
		if (Link.MappingGroups.Contains(MappingGroupA) && Link.MappingGroups.Contains(MappingGroupB))
		{
			// Both mapping groups are linked and should have unique bindings
			return true;
		}
	}

	return false;
}

FInputMappingPreset UAutoSettingsInputConfig::GetInputPresetByTag(FGameplayTag PresetTag) const
{
	TArray<FInputMappingPreset> Presets = GetInputPresets();
	for(FInputMappingPreset& Preset : Presets)
	{
		if(Preset.PresetTag == PresetTag)
		{
			return Preset;
		}
	}
	check(Presets.Num() > 0);
	return Presets[0];
}

TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> UAutoSettingsInputConfig::AsWeakInterfacePtrConst() const
{
	return TWeakInterfacePtr<const IAutoSettingsInputConfigInterface>(const_cast<ThisClass*>(this));
}

FGameplayTag UAutoSettingsInputConfig::GetKeyGroupOfKey(FKey Key) const
{
	for (FKeyGroup KeyGroup : KeyGroups)
	{
		if (KeyGroup.Contains(Key))
		{
			return KeyGroup.KeyGroupTag;
		}
	}

	return FGameplayTag();
}

bool UAutoSettingsInputConfig::IsKeyGroupDefined(FGameplayTag KeyGroupTag) const
{
	return KeyGroups.ContainsByPredicate([KeyGroupTag](const FKeyGroup& KeyGroup) { return KeyGroup.KeyGroupTag == KeyGroupTag; });
}

bool UAutoSettingsInputConfig::IsAxisKey(FKey Key) const
{
	const FAxisAssociation* FoundAssociation = AxisAssociations.FindByPredicate([Key](const FAxisAssociation Association)
    {
        return Association.AxisKey == Key;
    });

	return FoundAssociation != nullptr;
}

#if WITH_EDITOR

#endif
