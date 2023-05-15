// Copyright Sam Bonifacio. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KeyMappingTypes.h"
#include "GameFramework/PlayerInput.h"
#include "UObject/WeakInterfacePtr.h"
#include "InputMappingGroup.generated.h"

class IAutoSettingsInputConfigInterface;

// An input mapping group represents a set of mappings for which each action or axis has a single binding
// Each action should have a unique name and each axis should have a unique name + scale combination, however axis keys count as all scales
// It's valid to have axis mappings for (Name: MoveForward, Scale: 1, Key: W) and (Name: MoveForward, Scale: -1, Key: S) at the same time
// It's not valid to have (Name: MoveForward, Scale: 1, Key: GamepadLeftStickY) and (Name: MoveForward, Scale: -1, Key: S)
USTRUCT()
struct AUTOSETTINGSINPUT_API FInputMappingGroup
{
	GENERATED_BODY()

	FInputMappingGroup()
	{}
	
	FInputMappingGroup(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
	{
		SetConfig(InConfig);
	}
	
	// Action mappings in this group
	UPROPERTY(config, EditAnywhere, Category = "Input Mapping Group", meta = (TitleProperty = "ActionName"))
	TArray<FConfigActionKeyMapping> ActionMappings;

	// Axis mappings in this group
	UPROPERTY(config, EditAnywhere, Category = "Input Mapping Group", meta = (TitleProperty = "AxisName"))
	TArray<FConfigAxisKeyMapping> AxisMappings;

	UPROPERTY(config)
	TArray<FConfigActionKeyMapping> UnboundActionMappings;

	UPROPERTY(config)
	TArray<FConfigAxisKeyMapping> UnboundAxisMappings;

	TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> Config;

	void SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig);
	
	// Return all action mappings that match the given parameters
	TArray<FInputActionKeyMapping> GetAllActions(FName ActionName, FGameplayTag KeyGroup) const;

	// Return all axis mappings that match the given parameters
	TArray<FInputAxisKeyMapping> GetAllAxes(FName AxisName, float Scale, FGameplayTag KeyGroup, bool bAnyScale = false) const;

	// Returns the first action that matches the given parameters
	FInputActionKeyMapping GetAction(FName ActionName, FGameplayTag KeyGroup) const;

	// Returns the first axis that matches the given parameters
	FInputAxisKeyMapping GetAxis(FName AxisName, float Scale, FGameplayTag KeyGroup) const;

	// Add the given action and remove any existing actions that it should replace
	// Returns any actions that were unbound from different keys, if any
	FInputMappingGroup ReplaceAction(FConfigActionKeyMapping Action, bool bAnyKeyGroup = false);

	// Add the given axis and remove any exist axes that it should replace
	// Returns any axes that were unbound from different keys, if any
	FInputMappingGroup ReplaceAxis(FConfigAxisKeyMapping Axis, bool bAnyKeyGroup = false);

	// Unbind any actions or axes that are bound to the given chord
	// Returns a mapping group containing any mappings that were unbound
	FInputMappingGroup UnbindChord(FKey Key, bool ShiftDown = false, bool CtrlDown = false, bool AltDown = false, bool CmdDown = false);

	void RemoveAction(FName ActionName, FGameplayTag KeyGroup, bool bRemoveFromUnbound = false);

	void RemoveAxis(FName AxisName, float Scale, FGameplayTag KeyGroup, bool bRemoveFromUnbound = false, bool bIgnoreAxisKeys = false, bool bAnyScale = false);

	void RemoveMappings(FInputMappingGroup& MappingsToRemove);

	FInputMappingGroup FindUnboundMappings(const FInputMappingGroup& SourceMappingGroup) const;

	// Convert all mappings into unbound mappings
	FInputMappingGroup ToUnboundMappings() const;

	void RemoveUnboundMappings();

	// Remove mappings and unbound mappings that are shared by the other mapping group
	void RemoveRedundantMappings(const FInputMappingGroup& BaseMappingGroup);

	void MarkAllMappingsDefault();

	bool operator==(const FInputMappingGroup& Other) const
	{
		return (ActionMappings == Other.ActionMappings
			&& AxisMappings == Other.AxisMappings);
	}

	FInputMappingGroup operator+=(const FInputMappingGroup& Other);

};
