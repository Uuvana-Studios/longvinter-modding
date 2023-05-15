// Copyright Sam Bonifacio. All Rights Reserved.

#include "InputMappingLayout.h"
#include "Misc/AutoSettingsInputConfig.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutoSettingsInputLogs.h"

void FInputMappingLayout::SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
{
	Config = InConfig;
	for (int32 i = 0; i < MappingGroups.Num(); i++)
	{
		MappingGroups[i].SetConfig(Config);
	}
}

TArray<int32> FInputMappingLayout::GetMappingGroupsToUnbind(int32 SourceMappingGroup) const
{
	TArray<int32> Result;

	for (int32 i = 0; i < MappingGroups.Num(); i++)
	{
		if (Config->ShouldBindingsBeUniqueBetweenMappingGroups(SourceMappingGroup, i))
		{
			Result.Add(i);
		}
	}

	return Result;
}

void FInputMappingLayout::SetMappings(TArray<FInputActionKeyMapping> ActionMappings, TArray<FInputAxisKeyMapping> AxisMappings)
{
	MappingGroups.Empty();

	for (const FInputActionKeyMapping& Action : ActionMappings)
	{
		// Find first group without that action
		FInputMappingGroup* TargetGroup = nullptr;
		for (FInputMappingGroup& Group : MappingGroups)
		{
			if (Group.GetAllActions(Action.ActionName, Config->GetKeyGroupOfKey(Action.Key)).Num() == 0)
			{
				TargetGroup = &Group;
				break;
			}
		}
		if (!TargetGroup)
		{
			MappingGroups.Add(FInputMappingGroup(Config));
			TargetGroup = &MappingGroups.Last(); 
		}
		TargetGroup->ActionMappings.Add(Action);
	}

	for (const FInputAxisKeyMapping& Axis : AxisMappings)
	{
		// Find first group without that axis
		const bool bIsAxisKey = Config->IsAxisKey(Axis.Key);

		FInputMappingGroup* TargetGroup = nullptr;
		for (FInputMappingGroup& Group : MappingGroups)
		{
			if (Group.GetAllAxes(Axis.AxisName, Axis.Scale, Config->GetKeyGroupOfKey(Axis.Key), bIsAxisKey).Num() == 0)
			{
				TargetGroup = &Group;
				break;
			}
		}
		if (!TargetGroup)
		{
			MappingGroups.Add(FInputMappingGroup(Config));
			TargetGroup = &MappingGroups.Last();
		}
		TargetGroup->AxisMappings.Add(Axis);
	}
}

FInputMappingLayout FInputMappingLayout::ReplaceAction(const FConfigActionKeyMapping& Action, int32 MappingGroup, bool bAnyKeyGroup)
{
	// Use first mapping group if none specified
	if (MappingGroup < 0)
		MappingGroup = 0;

	FInputMappingLayout UnboundMappings(Config);
	
	// Unbind key from applicable mapping groups
	// Multiple mappings can have invalid key (unbound) so don't unbind in that case
	if (Action.Key.IsValid())
	{
		// Don't check AllowMultipleBindingsPerKey here - it will be checked in GetMappingGroupsToUnbind
		UnboundMappings = UnbindChord(Action.Key, GetMappingGroupsToUnbind(MappingGroup), Action.bShift, Action.bCtrl, Action.bAlt, Action.bCmd);
	}

	// Replace action and collect any more unbound ones
	UnboundMappings.GetMappingGroup(MappingGroup) += GetMappingGroup(MappingGroup).ReplaceAction(Action, bAnyKeyGroup);
	
	return UnboundMappings;
}

FInputMappingLayout FInputMappingLayout::ReplaceAxis(const FConfigAxisKeyMapping& Axis, int32 MappingGroup, bool bAnyKeyGroup)
{
	// Use first mapping group if none specified
	if (MappingGroup < 0)
		MappingGroup = 0;

	FInputMappingLayout UnboundMappings(Config);

	// Unbind key from applicable mapping groups
	// Multiple mappings can have invalid key (unbound) so don't unbind in that case
	if (Axis.Key.IsValid())
	{
		// Don't check AllowMultipleBindingsPerKey here - it will be checked in GetMappingGroupsToUnbind
		UnboundMappings = UnbindChord(Axis.Key, GetMappingGroupsToUnbind(MappingGroup));
	}

	// Replace axis and collect any more unbound ones
	UnboundMappings.GetMappingGroup(MappingGroup) += GetMappingGroup(MappingGroup).ReplaceAxis(Axis, bAnyKeyGroup);
	
	return UnboundMappings;
}

FInputMappingLayout FInputMappingLayout::UnbindChord(FKey Key, TArray<int32> MappingGroupIds, bool ShiftDown, bool CtrlDown, bool AltDown, bool CmdDown)
{
	FInputMappingLayout UnboundMappings(Config);
	for (int32 MappingGroupId : MappingGroupIds)
	{
		FInputMappingGroup& Group = MappingGroups[MappingGroupId];
		FInputMappingGroup& UnboundMappingsGroup = UnboundMappings.GetMappingGroup(MappingGroupId);
		UnboundMappingsGroup = Group.UnbindChord(Key, ShiftDown, CtrlDown, AltDown, CmdDown);
	}
	return UnboundMappings;
}

void FInputMappingLayout::RemoveAction(FName ActionName, int32 MappingGroupId, FGameplayTag KeyGroup, bool bRemoveFromUnbound)
{
	GetMappingGroup(MappingGroupId).RemoveAction(ActionName, KeyGroup, bRemoveFromUnbound);
}

void FInputMappingLayout::RemoveAxis(FName AxisName, float Scale, int32 MappingGroupId, FGameplayTag KeyGroup, bool bRemoveFromUnbound, bool bIgnoreAxisKeys, bool bAnyScale)
{
	GetMappingGroup(MappingGroupId).RemoveAxis(AxisName, Scale, KeyGroup, bRemoveFromUnbound, bIgnoreAxisKeys, bAnyScale);
}

bool FInputMappingLayout::HasMappingGroup(int32 MappingGroupIndex) const
{
	return MappingGroups.IsValidIndex(MappingGroupIndex);
}

FInputMappingGroup& FInputMappingLayout::GetMappingGroup(int32 MappingGroupIndex)
{
	if (!MappingGroups.IsValidIndex(MappingGroupIndex))
	{
		// Add new mapping group(s) if we need
		const int32 NumToAdd = MappingGroupIndex - MappingGroups.Num() + 1;
		for(int32 i=0; i<NumToAdd;i++)
		{
			MappingGroups.Add(FInputMappingGroup(Config));
		}
	}
	return MappingGroups[MappingGroupIndex];
}

const FInputMappingGroup& FInputMappingLayout::GetMappingGroupConst(int32 MappingGroupIndex) const
{
	check(MappingGroups.IsValidIndex(MappingGroupIndex));
	return MappingGroups[MappingGroupIndex];
}

FInputActionKeyMapping FInputMappingLayout::GetAction(int32 MappingGroup, FName ActionName, FGameplayTag KeyGroup) const
{
	if(!ensure(MappingGroups.IsValidIndex(MappingGroup)))
	{
		return FInputActionKeyMapping();
	}
	return GetMappingGroupConst(MappingGroup).GetAction(ActionName, KeyGroup);
}

FInputAxisKeyMapping FInputMappingLayout::GetAxis(int32 MappingGroup, FName AxisName, float Scale, FGameplayTag KeyGroup) const
{
	if(!ensure(MappingGroups.IsValidIndex(MappingGroup)))
	{
		return FInputAxisKeyMapping();
	}
	return GetMappingGroupConst(MappingGroup).GetAxis(AxisName, Scale, KeyGroup);
}

FInputMappingLayout FInputMappingLayout::FindUnboundMappings(const FInputMappingLayout& SourceLayout) const
{
	FInputMappingLayout UnboundMappings;
	for(int32 i = 0; i< SourceLayout.MappingGroups.Num(); i++)
	{
		if(MappingGroups.IsValidIndex(i))
		{
			UnboundMappings.GetMappingGroup(i) = GetMappingGroupConst(i).FindUnboundMappings(SourceLayout.GetMappingGroupConst(i));
		}
		else
		{
			UnboundMappings.GetMappingGroup(i) = SourceLayout.GetMappingGroupConst(i);
		}
	}
	return UnboundMappings;
}

FInputMappingLayout FInputMappingLayout::MergeMappings(const FInputMappingLayout& OverridesLayout)
{
	for(int32 i = 0; i< OverridesLayout.MappingGroups.Num();i++)
	{
		const FInputMappingGroup& MappingGroup = OverridesLayout.MappingGroups[i];
		for(FConfigActionKeyMapping Action : MappingGroup.ActionMappings)
		{
			ReplaceAction(Action, i, false);
		}
		for(FConfigAxisKeyMapping Axis : MappingGroup.AxisMappings)
		{
			ReplaceAxis(Axis, i, false);
		}
	}
	return *this;
}

FInputMappingLayout FInputMappingLayout::MergeUnboundMappings(const FInputMappingLayout& OverridesLayout)
{
	for (int32 i = 0; i < OverridesLayout.MappingGroups.Num(); i++)
	{
		const FInputMappingGroup& MappingGroup = OverridesLayout.MappingGroups[i];
		for (const FInputActionKeyMapping& UnboundAction : MappingGroup.UnboundActionMappings)
		{
			RemoveAction(UnboundAction.ActionName, i, Config->GetKeyGroupOfKey(UnboundAction.Key), true);
			GetMappingGroup(i).UnboundActionMappings.Add(UnboundAction);
		}
		for (const FInputAxisKeyMapping& UnboundAxis : MappingGroup.UnboundAxisMappings)
		{
			// Remove unbound mapping, but if the new mapping is not an axis key then leave any existing axis key unbound mappings, because the unbound mapping should still apply to other scales
			// If the new mapping is an axis key, we can just remove all unbound mappings from all scales
			const bool bIsAxisKey = Config->IsAxisKey(UnboundAxis.Key);
			const bool bIgnoreAxisKeys = !bIsAxisKey;
			RemoveAxis(UnboundAxis.AxisName, UnboundAxis.Scale, i, Config->GetKeyGroupOfKey(UnboundAxis.Key), true, bIgnoreAxisKeys);
			GetMappingGroup(i).UnboundAxisMappings.Add(UnboundAxis);
		}
	}
	return *this;
}

void FInputMappingLayout::ApplyUnboundMappings()
{
	for (int32 i = 0; i < MappingGroups.Num(); i++)
	{
		FInputMappingGroup& MappingGroup = MappingGroups[i];
		for (const FInputActionKeyMapping& UnboundAction : MappingGroup.UnboundActionMappings)
		{
			RemoveAction(UnboundAction.ActionName, i, Config->GetKeyGroupOfKey(UnboundAction.Key), false);
		}
		MappingGroup.UnboundActionMappings.Reset();
		for (const FInputAxisKeyMapping& UnboundAxis : MappingGroup.UnboundAxisMappings)
		{
			// If the unbound axis is an axis key, it spans the all scales and should remove all mappings on the same axis regardless of scale
			const bool bIsAxisKey = Config->IsAxisKey(UnboundAxis.Key);
			RemoveAxis(UnboundAxis.AxisName, UnboundAxis.Scale, i, Config->GetKeyGroupOfKey(UnboundAxis.Key), false, false, bIsAxisKey);
		}
		MappingGroup.UnboundAxisMappings.Reset();
	}
}

FInputMappingLayout FInputMappingLayout::ToUnboundMappings() const
{
	FInputMappingLayout Result;
	for(const FInputMappingGroup& Group : MappingGroups)
	{
		Result.MappingGroups.Add(Group.ToUnboundMappings());
	}
	return Result;
}

void FInputMappingLayout::RemoveUnboundMappings()
{
	for (FInputMappingGroup& Group : MappingGroups)
	{
		Group.RemoveUnboundMappings();
	}
}

void FInputMappingLayout::RemoveRedundantMappings(const FInputMappingLayout& BaseLayout)
{
	for(int32 i = 0; i< MappingGroups.Num(); i++)
	{
		if(BaseLayout.MappingGroups.IsValidIndex(i))
		{
			GetMappingGroup(i).RemoveRedundantMappings(BaseLayout.GetMappingGroupConst(i));
		}
	}
}

void FInputMappingLayout::MarkAllMappingsDefault()
{
	for (FInputMappingGroup& Group : MappingGroups)
	{
		Group.MarkAllMappingsDefault();
	}
}

void FInputMappingLayout::ConsolidateDefaultChanges(const FInputMappingLayout& BaseLayout)
{
	for(int32 i = 0; i< MappingGroups.Num(); i++)
	{
		for(FConfigActionKeyMapping& Mapping : GetMappingGroup(i).ActionMappings)
		{
			if(Mapping.bIsDefault)
			{
				const FGameplayTag KeyGroup = Config->GetKeyGroupOfKey(Mapping.Key);
				FConfigActionKeyMapping DefaultMapping = BaseLayout.GetMappingGroupConst(i).GetAction(Mapping.ActionName, KeyGroup);
				if(Mapping != DefaultMapping)
				{
					Mapping = DefaultMapping;
					Mapping.bIsDefault = true;
				}
			}
		}

		for(FConfigAxisKeyMapping& Mapping : GetMappingGroup(i).AxisMappings)
		{
			if(Mapping.bIsDefault)
			{
				const FGameplayTag KeyGroup = Config->GetKeyGroupOfKey(Mapping.Key);
				FConfigAxisKeyMapping DefaultMapping = BaseLayout.GetMappingGroupConst(i).GetAxis(Mapping.AxisName, Mapping.Scale, KeyGroup);
				if(Mapping != DefaultMapping)
				{
					Mapping = DefaultMapping;
					Mapping.bIsDefault = true;
				}
			}
		}
	}
}

void FInputMappingLayout::DumpToLog() const
{
	UE_LOG(LogAutoSettingsInput, Display, TEXT("        Mapping Groups:"));
	if (GetMappingGroupsConst().Num() == 0)
	{
		UE_LOG(LogAutoSettingsInput, Display, TEXT("            NONE"));
	}
	for (int MappingGroupIndex = 0; MappingGroupIndex < GetMappingGroupsConst().Num(); MappingGroupIndex++)
	{
		const FInputMappingGroup& MappingGroup = GetMappingGroupsConst()[MappingGroupIndex];
		UE_LOG(LogAutoSettingsInput, Display, TEXT("            %i:"), MappingGroupIndex);
		
		UE_LOG(LogAutoSettingsInput, Display, TEXT("                Actions:"));
		if (MappingGroup.ActionMappings.Num() == 0)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    NONE"));
		}
		for (int ActionIndex = 0; ActionIndex < MappingGroup.ActionMappings.Num(); ActionIndex++)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    %i: %s"), ActionIndex, *MappingGroup.ActionMappings[ActionIndex].ToDebugString());
		}
		
		UE_LOG(LogAutoSettingsInput, Display, TEXT("                Axes:"));
		if (MappingGroup.AxisMappings.Num() == 0)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    NONE"));
		}
		for (int AxisIndex = 0; AxisIndex < MappingGroup.AxisMappings.Num(); AxisIndex++)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    %i: %s"), AxisIndex, *MappingGroup.AxisMappings[AxisIndex].ToDebugString());
		}
		
		UE_LOG(LogAutoSettingsInput, Display, TEXT("                Unbound Actions:"));
		if (MappingGroup.UnboundActionMappings.Num() == 0)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    NONE"));
		}
		for (int ActionIndex = 0; ActionIndex < MappingGroup.UnboundActionMappings.Num(); ActionIndex++)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    %i: %s"), ActionIndex, *MappingGroup.UnboundActionMappings[ActionIndex].ToDebugString());
		}
		
		UE_LOG(LogAutoSettingsInput, Display, TEXT("                Unbound Axes:"));
		if (MappingGroup.UnboundAxisMappings.Num() == 0)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    NONE"));
		}
		for (int AxisIndex = 0; AxisIndex < MappingGroup.UnboundAxisMappings.Num(); AxisIndex++)
		{
			UE_LOG(LogAutoSettingsInput, Display, TEXT("                    %i: %s"), AxisIndex, *MappingGroup.UnboundAxisMappings[AxisIndex].ToDebugString());
		}
	}
}

void FInputMappingLayout::Apply(APlayerController* Player)
{
	if (!ensure(Player && Player->PlayerInput))
	{
		return;
	}

	// Loop through ActionMappings and remove any that aren't in Blacklist,
	// then add ones from GetAction() onto it (thus preserving any actions from the blacklist)
	Player->PlayerInput->ActionMappings.RemoveAll([this](const FInputActionKeyMapping& Mapping)
	{
		return !Config->GetPreservedActions().Contains(Mapping.ActionName);
	});
	Player->PlayerInput->ActionMappings.Append(GetActions(false));

	// Same for axes
	Player->PlayerInput->AxisMappings.RemoveAll([this](const FInputAxisKeyMapping& Mapping)
	{
		return !Config->GetPreservedAxes().Contains(Mapping.AxisName);
	});
	Player->PlayerInput->AxisMappings.Append(GetAxes(false));

	// Rebuild
	Player->PlayerInput->ForceRebuildingKeyMaps(false);
}
