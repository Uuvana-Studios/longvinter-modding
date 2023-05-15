// Copyright Sam Bonifacio. All Rights Reserved.

#include "InputMappingGroup.h"
#include "KeyMappingTypes.h"
#include "Misc/AutoSettingsInputConfig.h"
#include "AutoSettingsError.h"
#include "Misc/AutoSettingsInputProjectConfig.h"

void FInputMappingGroup::SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
{
	Config = InConfig;
}

TArray<FInputActionKeyMapping> FInputMappingGroup::GetAllActions(FName ActionName, FGameplayTag KeyGroup) const
{
	const bool bAnyKeyGroup = !KeyGroup.IsValid();

	if(!bAnyKeyGroup && !Config->IsKeyGroupDefined(KeyGroup))
	{
		FAutoSettingsError::LogError(FString::Printf(TEXT("Undefined Key Group with tag '%s'. Please add a Key Group with the tag in the Auto Settings page of the project settings."), *KeyGroup.ToString()));
	}

	TArray<FInputActionKeyMapping> Result;
	for (FInputActionKeyMapping Action : ActionMappings)
	{
		if (Action.ActionName == ActionName && (bAnyKeyGroup || Config->GetKeyGroupOfKey(Action.Key) == KeyGroup))
		{
			Result.Add(Action);
		}
	}
	return Result;
}

TArray<FInputAxisKeyMapping> FInputMappingGroup::GetAllAxes(FName AxisName, float Scale, FGameplayTag KeyGroup, bool bAnyScale) const
{
	const bool bAnyKeyGroup = !KeyGroup.IsValid();

	if (!bAnyKeyGroup && !Config->IsKeyGroupDefined(KeyGroup))
	{
		FAutoSettingsError::LogError(FString::Printf(TEXT("Undefined Key Group with tag '%s'. Please add a Key Group with the tag in the Auto Settings page of the project settings."), *KeyGroup.ToString()));
	}

	TArray<FInputAxisKeyMapping> Result;
	for (FInputAxisKeyMapping Axis : AxisMappings)
	{
		if (Axis.AxisName == AxisName && (bAnyScale || Axis.Scale == Scale || Config->IsAxisKey(Axis.Key)) && (bAnyKeyGroup || Config->GetKeyGroupOfKey(Axis.Key) == KeyGroup))
		{
			Result.Add(Axis);
		}
	}
	return Result;
}

FInputActionKeyMapping FInputMappingGroup::GetAction(FName ActionName, FGameplayTag KeyGroup) const
{
	TArray<FInputActionKeyMapping> Actions = GetAllActions(ActionName, KeyGroup);
	if(Actions.Num() == 0)
	{
		return FInputActionKeyMapping();
	}
	// Return last, to prioritize mappings that were added more recently
	return Actions.Last();
}

FInputAxisKeyMapping FInputMappingGroup::GetAxis(FName AxisName, float Scale, FGameplayTag KeyGroup) const
{
	TArray<FInputAxisKeyMapping> Axes = GetAllAxes(AxisName, Scale, KeyGroup);
	if(Axes.Num() == 0)
	{
		return FInputAxisKeyMapping();
	}
	// Return last, to prioritize mappings that were added more recently
	return Axes.Last();
}

FInputMappingGroup FInputMappingGroup::ReplaceAction(FConfigActionKeyMapping Action, bool bAnyKeyGroup)
{
	// Filter key group unless using any
	const FGameplayTag KeyGroup = bAnyKeyGroup ? FGameplayTag() : Config->GetKeyGroupOfKey(Action.Key);

	FInputMappingGroup MappingsToRemove = FInputMappingGroup(Config);
	
	for (FInputActionKeyMapping& ExistingAction : GetAllActions(Action.ActionName, KeyGroup))
	{
		MappingsToRemove.ActionMappings.Add(ExistingAction);
	}

	RemoveMappings(MappingsToRemove);
	ActionMappings.Add(FConfigActionKeyMapping(Action));

	// Remove unbound mapping
	RemoveAction(Action.ActionName, KeyGroup, true);
	
	return MappingsToRemove.ToUnboundMappings();
}

FInputMappingGroup FInputMappingGroup::ReplaceAxis(FConfigAxisKeyMapping Axis, bool bAnyKeyGroup)
{
	const bool bIsAxisKey = Config->IsAxisKey(Axis.Key);

	// Filter key group unless using any
	const FGameplayTag KeyGroup = bAnyKeyGroup ? FGameplayTag() : Config->GetKeyGroupOfKey(Axis.Key);

	FInputMappingGroup MappingsToRemove = FInputMappingGroup(Config);

	for (FInputAxisKeyMapping& ExistingAxis : GetAllAxes(Axis.AxisName, Axis.Scale, KeyGroup, bIsAxisKey))
	{
		MappingsToRemove.AxisMappings.Add(ExistingAxis);
	}

	RemoveMappings(MappingsToRemove);
	AxisMappings.Add(FConfigAxisKeyMapping(Axis));

	// Remove unbound mapping, but if the new mapping is not an axis key then leave any existing axis key unbound mappings, because the unbound mapping should still apply to other scales
	// If the new mapping is an axis key, we can just remove all unbound mappings from all scales
	const bool bIgnoreAxisKeys = !bIsAxisKey;
	const bool bAnyScale = bIsAxisKey;
	RemoveAxis(Axis.AxisName, Axis.Scale, KeyGroup, true, bIgnoreAxisKeys, bAnyScale);
	
	return MappingsToRemove.ToUnboundMappings();
}

FInputMappingGroup FInputMappingGroup::UnbindChord(FKey Key, bool ShiftDown, bool CtrlDown, bool AltDown, bool CmdDown)
{
	FInputMappingGroup MappingsToRemove = FInputMappingGroup(Config);

	// Remove all action mappings with same chord
	for(FInputActionKeyMapping& Action : ActionMappings)
	{
		if(Action.Key == Key && Action.bShift == ShiftDown && Action.bCtrl == CmdDown && Action.bAlt == AltDown && Action.bCmd == CmdDown)
		{
			MappingsToRemove.ActionMappings.Add(Action);
		}
	}

	// Since axis cannot have modifiers, only unbind axis with same key if no modifiers are down
	if(!ShiftDown && !CtrlDown && !AltDown && !CmdDown)
	{
		for(FInputAxisKeyMapping& Axis : AxisMappings)
		{
			if(Axis.Key == Key)
			{
				MappingsToRemove.AxisMappings.Add(Axis);
			}
		}
	}
	
	RemoveMappings(MappingsToRemove);

	return MappingsToRemove.ToUnboundMappings();
}

void FInputMappingGroup::RemoveAction(FName ActionName, FGameplayTag KeyGroup, bool bRemoveFromUnbound)
{
	const bool bAnyKeyGroup = !KeyGroup.IsValid();
	FInputMappingGroup MappingsToRemove = FInputMappingGroup(Config);

	TArray<FConfigActionKeyMapping>& TargetArray = bRemoveFromUnbound ? UnboundActionMappings : ActionMappings;
	TArray<FConfigActionKeyMapping>& RemovalArray = bRemoveFromUnbound ? MappingsToRemove.UnboundActionMappings : MappingsToRemove.ActionMappings;
	
	// Remove all action mappings with same chord
	for (FInputActionKeyMapping& Action : TargetArray)
	{
		if (Action.ActionName == ActionName && (bAnyKeyGroup || Config->GetKeyGroupOfKey(Action.Key) == KeyGroup))
		{
			RemovalArray.Add(Action);
		}
	}

	RemoveMappings(MappingsToRemove);
}

void FInputMappingGroup::RemoveAxis(FName AxisName, float Scale, FGameplayTag KeyGroup, bool bRemoveFromUnbound, bool bIgnoreAxisKeys, bool bAnyScale)
{
	const bool bAnyKeyGroup = !KeyGroup.IsValid();
	FInputMappingGroup MappingsToRemove = FInputMappingGroup(Config);

	TArray<FConfigAxisKeyMapping>& TargetArray = bRemoveFromUnbound ? UnboundAxisMappings : AxisMappings;
	TArray<FConfigAxisKeyMapping>& RemovalArray = bRemoveFromUnbound ? MappingsToRemove.UnboundAxisMappings : MappingsToRemove.AxisMappings;
	
	// Remove all action mappings with same chord
	for (FInputAxisKeyMapping& Axis : TargetArray)
	{
		const bool bIsAxisKey = Config->IsAxisKey(Axis.Key);
		const bool bIgnore = bIsAxisKey && bIgnoreAxisKeys;
		if (!bIgnore && Axis.AxisName == AxisName && ( bAnyScale || Axis.Scale == Scale) && (bAnyKeyGroup || Config->GetKeyGroupOfKey(Axis.Key) == KeyGroup))
		{
			RemovalArray.Add(Axis);
		}
	}

	RemoveMappings(MappingsToRemove);
}

void FInputMappingGroup::RemoveMappings(FInputMappingGroup& MappingsToRemove)
{
	for(FInputActionKeyMapping& Action : MappingsToRemove.ActionMappings)
	{
		ActionMappings.Remove(Action);
	}
	for(FInputAxisKeyMapping& Axis : MappingsToRemove.AxisMappings)
	{
		AxisMappings.Remove(Axis);
	}
	for(FInputActionKeyMapping& Action : MappingsToRemove.UnboundActionMappings)
	{
		UnboundActionMappings.Remove(Action);
	}
	for(FInputAxisKeyMapping& Axis : MappingsToRemove.UnboundAxisMappings)
	{
		UnboundAxisMappings.Remove(Axis);
	}
}

FInputMappingGroup FInputMappingGroup::FindUnboundMappings(const FInputMappingGroup& SourceMappingGroup) const
{
	FInputMappingGroup UnboundMappings = FInputMappingGroup(Config);
	for(const FInputActionKeyMapping& Action : SourceMappingGroup.ActionMappings)
	{
		if(GetAllActions(Action.ActionName, FGameplayTag()).Num() == 0)
		{
			UnboundMappings.UnboundActionMappings.Add(Action);
		}
	}
	for(const FInputAxisKeyMapping& Axis : SourceMappingGroup.AxisMappings)
	{
		if(GetAllAxes(Axis.AxisName, Axis.Scale, FGameplayTag(), false).Num() == 0)
		{
			UnboundMappings.UnboundAxisMappings.Add(Axis);
		}
	}
	return UnboundMappings;
}

FInputMappingGroup FInputMappingGroup::ToUnboundMappings() const
{
	FInputMappingGroup Result = FInputMappingGroup(Config);
	Result.UnboundActionMappings.Append(ActionMappings);
	Result.UnboundAxisMappings.Append(AxisMappings);
	return Result;
}

void FInputMappingGroup::RemoveUnboundMappings()
{
	FInputMappingGroup MappingsToRemove(Config);
	for(FInputActionKeyMapping& Action : UnboundActionMappings)
	{
		if(!Action.Key.IsValid())
		{
			MappingsToRemove.UnboundActionMappings.Add(Action);
		}
	}
	for(FInputAxisKeyMapping& Axis : UnboundAxisMappings)
	{
		if(!Axis.Key.IsValid())
		{
			MappingsToRemove.UnboundAxisMappings.Add(Axis);
		}
	}
	RemoveMappings(MappingsToRemove);
}

void FInputMappingGroup::RemoveRedundantMappings(const FInputMappingGroup& BaseMappingGroup)
{
	FInputMappingGroup MappingsToRemove(Config);
	for(FInputActionKeyMapping& Action : ActionMappings)
	{
		if(BaseMappingGroup.ActionMappings.Contains(Action))
		{
			// Other mapping group contains the same mapping, so remove
			MappingsToRemove.ActionMappings.Add(Action);
		}
	}
	for(FConfigAxisKeyMapping& Axis : AxisMappings)
	{
		if(BaseMappingGroup.AxisMappings.Contains(Axis))
		{
			// Other mapping group contains the same mapping

			// Edge case: the mapping is not redundant if it's overriding an unbound axis
			bool bOverridingUnboundAxis = false;
			for(FInputAxisKeyMapping& UnboundAxisMapping : UnboundAxisMappings)
			{
				if(UnboundAxisMapping.AxisName == Axis.AxisName && Config->IsAxisKey(UnboundAxisMapping.Key))
				{
					bOverridingUnboundAxis = true;
					break;
				}
			}

			if(!bOverridingUnboundAxis)
			{
				// Just remove normally
				MappingsToRemove.AxisMappings.Add(Axis);
			}
		}
	}

	// For unbound mappings, we don't check the key of the unbound mapping itself against the other mapping group
	// The unbound mapping is used as a flag to say "there was something unbound here, so don't show anything from the base preset, even if there are no overrides"
	
	for(FInputActionKeyMapping& UnboundAction : UnboundActionMappings)
	{
		const FGameplayTag KeyGroup = Config->GetKeyGroupOfKey(UnboundAction.Key);
		if(!BaseMappingGroup.GetAction(UnboundAction.ActionName, KeyGroup).Key.IsValid())
		{
			// Already not bound on the other mapping group, so remove
			MappingsToRemove.UnboundActionMappings.Add(UnboundAction);
		}
	}
	for(FInputAxisKeyMapping& UnboundAxis : UnboundAxisMappings)
	{
		const FGameplayTag KeyGroup = Config->GetKeyGroupOfKey(UnboundAxis.Key);
		if(!BaseMappingGroup.GetAxis(UnboundAxis.AxisName, UnboundAxis.Scale, KeyGroup).Key.IsValid())
		{
			// Already not bound on the other mapping group, so remove
			MappingsToRemove.UnboundAxisMappings.Add(UnboundAxis);
		}
	}
	RemoveMappings(MappingsToRemove);
}

void FInputMappingGroup::MarkAllMappingsDefault()
{
	for(FConfigActionKeyMapping& Action : ActionMappings)
	{
		Action.bIsDefault = true;
	}
	for(FConfigAxisKeyMapping& Axis : AxisMappings)
	{
		Axis.bIsDefault = true;
	}
}

FInputMappingGroup FInputMappingGroup::operator+=(const FInputMappingGroup& Other)
{
	ActionMappings.Append(Other.ActionMappings);
	AxisMappings.Append(Other.AxisMappings);
	UnboundActionMappings.Append(Other.UnboundActionMappings);
	UnboundAxisMappings.Append(Other.UnboundAxisMappings);
	return *this;
}
