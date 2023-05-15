// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "AutoSettingsInputConfigInterface.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "GameFramework/PlayerInput.h"
#include "InputMappingPreset.h"
#include "Engine/Texture.h"
#include "AutoSettingsInputConfig.generated.h"

// Deprecated structure
USTRUCT()
struct FKeyIconPair
{
	GENERATED_BODY()

	UPROPERTY()
	FKey Key_DEPRECATED;

	UPROPERTY()
	UTexture* Icon_DEPRECATED;
};

// Set of key icons
USTRUCT()
struct FKeyIconSet
{
	GENERATED_BODY()
	
	// Tags to identify this icon set
	// May include platform such as "XBox" or "PlayStation" and/or variations such as "Small" or "Large"
	UPROPERTY(config, EditAnywhere, Category = "Key Icon Set")
	FGameplayTagContainer Tags;

	// Icons defined for different keys
	UPROPERTY(config, EditAnywhere, Category = "Key Icon Set")
	TMap<FKey, TSoftObjectPtr<UTexture>> IconMap;
	
	UPROPERTY()
	TArray<FKeyIconPair> Icons_DEPRECATED;

	// Return the icon texture for the given key
	UTexture* GetIcon(FKey Key) const;
};

// Defines a group of keys which should be separated from other keys
USTRUCT()
struct FKeyGroup
{
	GENERATED_BODY()
	
	// Tag to identify this key group
	UPROPERTY(config, EditAnywhere, Category = "Key Group")
	FGameplayTag KeyGroupTag;

	// If true, will include all gamepad keys in this key group
	UPROPERTY(config, EditAnywhere, Category = "Key Group")
	bool bUseGamepadKeys;

	// If true, will include all keys that are not gamepad keys in this key group
	UPROPERTY(config, EditAnywhere, Category = "Key Group")
	bool bUseNonGamepadKeys;

	// Keys to include in this key group
	UPROPERTY(config, EditAnywhere, Category = "Key Group", meta = (TitleProperty = "KeyName"))
	TArray<FKey> Keys;

	bool Contains(FKey Key) const
	{
		const bool bIsGamepad = Key.IsGamepadKey();
		if (bIsGamepad && bUseGamepadKeys)
			return true;
		if (!bIsGamepad && bUseNonGamepadKeys)
			return true;
		return Keys.Contains(Key);
	}
};

// Represents pairing of a button key and a scale, used to associate buttons with an axis
USTRUCT()
struct FKeyScale
{
	GENERATED_BODY()

	// Button key
	UPROPERTY(config, EditAnywhere, Category = "Key Scale")
	FKey Key;

	// Scale of the axis that the button key should be associated with
	UPROPERTY(config, EditAnywhere, Category = "Key Scale")
	float Scale;

	FKeyScale()
		: Scale(1.f)
	{
	}

	FKeyScale(FKey InKey)
		: Key(InKey)
		, Scale(1.f)
	{
	}

	FKeyScale(FKey InKey, float InScale)
		: Key(InKey)
		, Scale(InScale)
	{
	}

};

// Defines a set of mapping groups which are linked
USTRUCT()
struct FMappingGroupLink
{
	GENERATED_BODY()

	FMappingGroupLink(){}
	FMappingGroupLink(const TArray<int32>& MappingGroups)
		: MappingGroups(MappingGroups)
	{
		
	}
	
	UPROPERTY(config, EditAnywhere, Category = "Mapping Group Link")
	TArray<int32> MappingGroups;

};

// Determines association between axis and buttons
// Used to tell the system that for example "Gamepad Left Stick Y" axis with a scale of -1 should be associated with "Gamepad Left Stick Down"
USTRUCT()
struct FAxisAssociation
{
	GENERATED_BODY()

	FAxisAssociation(){}
	FAxisAssociation(FKey InAxisKey, TArray<FKeyScale> InButtonKeys)
	: AxisKey(InAxisKey)
	, ButtonKeys(InButtonKeys)
	{
	}
	
	// Axis key for this association
	UPROPERTY(config, EditAnywhere, Category = "Axis Association")
	FKey AxisKey;

	UPROPERTY(config)
	FKey AnalogKey_DEPRECATED;

	// Button keys to associate axis with
	UPROPERTY(config, EditAnywhere, Category = "Axis Association", meta = (TitleProperty = "Key"))
	TArray<FKeyScale> ButtonKeys;

	UPROPERTY(config)
	TArray<FKeyScale> BooleanKeys_DEPRECATED;

};

// Defines friendly user-facing name text for a key
USTRUCT()
struct FKeyFriendlyName
{
	GENERATED_BODY()

	// Specified key
	UPROPERTY(config, EditAnywhere, Category = "Key Friendly Name")
	FKey Key;

	// User-facing friendly text name
	UPROPERTY(config, EditAnywhere, Category = "Key Friendly Name")
	FText FriendlyName;
};

/**
 * Configuration object for Auto Settings Input
 */
UCLASS(config=Game, defaultconfig)
class AUTOSETTINGSINPUT_API UAutoSettingsInputConfig : public UDeveloperSettings, public IAutoSettingsInputConfigInterface
{
	GENERATED_BODY()

public:

	UAutoSettingsInputConfig();

	// Player Controllers need to be initialized with AutoSettings to set up their custom input overrides, otherwise they will still be using the mappings from the project settings Input page
	// If true, the system will automatically try to initialize the correct input overrides for new PlayerControllers it finds, which is the easiest option
	// If false, input can be manually initialized per PlayerController with InitializePlayerInputOverrides for greater flexibility
	UPROPERTY(config, EditAnywhere, Category = Input)
	bool bAutoInitializePlayerInputOverrides;

	// Allow Shift, Ctrl, Alt, Cmd modifiers for action mappings
	// Note: Axis mappings cannot have modifiers
	UPROPERTY(config, EditAnywhere, Category = Input)
	bool AllowModifierKeys;

	// If not empty, text to use for shift modifier labels if an icon is not found
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (EditCondition = "AllowModifierKeys"))
	FText ShiftModifierOverrideText;

	// If not empty, text to use for ctrl modifier labels if an icon is not found
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (EditCondition = "AllowModifierKeys"))
	FText CtrlModifierOverrideText;

	// If not empty, text to use for alt modifier labels if an icon is not found
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (EditCondition = "AllowModifierKeys"))
	FText AltModifierOverrideText;

	// If not empty, text to use for cmd modifier labels if an icon is not found
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (EditCondition = "AllowModifierKeys"))
	FText CmdModifierOverrideText;

	// Input mapping presets for this project
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (TitleProperty = "PresetTag"))
	TArray<FInputMappingPreset> InputPresets;

	// If false, new bindings will remove key from all other mappings
	UPROPERTY(config, EditAnywhere, Category = Input)
	bool AllowMultipleBindingsPerKey;

	// (Advanced) If AllowMultipleBindingsPerKey is true, this determines mapping groups which should remain "linked" with unique bindings
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (EditCondition = "AllowMultipleBindingsPerKey", TitleProperty = "MappingGroups"))
	TArray<FMappingGroupLink> MappingGroupLinks;

	// Actions in this list will always be preserved when modifying bindings
	// Eg. You have UI-related actions that should remain bound even when the same inputs are bound to in-game actions
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FName> PreservedActions;

	// Axes in this list will always be preserved when modifying bindings
	// See BlacklistedActions
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FName> PreservedAxes;

	// Sets of key icons for this project
	// Example sets might include "Xbox Small" or "Playstation Transparent", where the platform is determined on a global application-wide level and the variant is determined by location in game
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FKeyIconSet> KeyIconSets;

	// Friendly key names to use if an icon is not available
	// If a friendly name is not specified for a key, the default FKey GetDisplayName will be used
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (TitleProperty = "Key"))
	TArray<FKeyFriendlyName> KeyFriendlyNames;

	// Groups of keys to separate from other keys
	// Useful if you want a separate column for gamepad controls in your input UI or other similar cases
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (TitleProperty = "KeyGroupTag"))
	TArray<FKeyGroup> KeyGroups;

	// Key whitelist, use to allow specific keys for input mapping
	// Leave empty to allow all keys
	// Keys here will have no effect if also in DisallowedKeys
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FKey> AllowedKeys;

	// Key blacklist, use to disallow specific keys for input mapping
	// Will take precedence over AllowedKeys
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FKey> DisallowedKeys;

	// If any of these keys are pressed while listening for input binding, it will instead cancel and keep the original binding
	UPROPERTY(config, EditAnywhere, Category = Input)
	TArray<FKey> BindingEscapeKeys;

	// Distance in pixels the mouse must travel before it will trigger a capture for a mouse axis while listening for input bindings
	// Lower values are more sensitive, higher values mean you have to move the mouse further
	UPROPERTY(config, EditAnywhere, Category = Input)
	float MouseMoveCaptureDistance = 80.f;

	// Determines association between axes and buttons
	// This is used during axis rebinding to determine which axis should be used when a key press is detected
	// e.g. If the system detects the player has pressed "Gamepad Left Stick Down" key when binding "Move Backward",
	// these associations could specify that the "Gamepad Left Stick Y" axis should be bound with a scale of -1
	// otherwise the mapping will only have 2 key states (pressed and not pressed) rather than the full range of the axis
	UPROPERTY(config, EditAnywhere, Category = Input, meta = (TitleProperty = "AxisKey"))
	TArray<FAxisAssociation> AxisAssociations;

	// Returns all input presets in config or construct one from Input settings if none are found
	TArray<FInputMappingPreset> GetInputPresets() const;

	// Returns the icon texture for the given key and key icon tags
	UTexture* GetIconForKey(FKey InKey, FGameplayTagContainer Tags, float AxisScale = 0.f) const;

	// Returns the Friendly Name override for the key if available (specified in AutoSettings config) or falls back to the FKey DisplayName
	FText GetKeyFriendlyName(FKey Key) const;

	// Returns true if a Key Group with the specified tag exists and contains the given key
	bool DoesKeyGroupContainKey(FGameplayTag KeyGroupTag, FKey Key) const;

	// True if both keys are in the same key group
	bool SameKeyGroup(FKey KeyA, FKey KeyB) const;

	// Returns axis key for the given button key
	FKeyScale GetAxisKey(FKey InButtonKey) const;

	// Gets an axis button in any stored Axis Association
	// e.g. turn AxisKey= "Gamepad Left Stick Y" AxisScale= -1 into "Gamepad Left Stick Down"
	FKey GetAxisButton(FKey AxisKey, float AxisScale) const;

	// True if the given key is whitelisted (if applicable) and not blacklisted
	bool IsKeyAllowed(FKey Key) const;

	TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> AsWeakInterfacePtrConst() const;

	// IAutoSettingsInputConfigInterface

	virtual bool ShouldAllowMultipleBindingsPerKey() const override { return AllowMultipleBindingsPerKey; }
	virtual bool ShouldBindingsBeUniqueBetweenMappingGroups(int32 MappingGroupA, int32 MappingGroupB) const override;
	virtual FInputMappingPreset GetInputPresetByTag(FGameplayTag PresetTag) const override;
	virtual TArray<FName> GetPreservedActions() const override { return PreservedActions; }
	virtual TArray<FName> GetPreservedAxes() const override { return PreservedAxes; }
	virtual const TArray<struct FKeyGroup>& GetKeyGroups() const override { return KeyGroups; }
	virtual FGameplayTag GetKeyGroupOfKey(FKey Key) const override;
	virtual bool IsKeyGroupDefined(FGameplayTag KeyGroupTag) const override;
	virtual bool IsAxisKey(FKey Key) const override;
	
	// End IAUtoSettingsInputConfigInterface

#if WITH_EDITOR
	virtual bool SupportsAutoRegistration() const override { return false; }
#endif

protected:

	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FName> BlacklistedActions_DEPRECATED;

	UPROPERTY(config, meta = (DeprecatedProperty))
	TArray<FName> BlacklistedAxes_DEPRECATED;

};
