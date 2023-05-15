// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "InputMappingGroup.h"
#include "InputMappingLayout.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct AUTOSETTINGSINPUT_API FInputMappingLayout
{
    GENERATED_BODY()

    FInputMappingLayout() {}

	FInputMappingLayout(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
		: Config(InConfig)
	{}
	
    FInputMappingLayout(TArray<FInputMappingGroup> InMappingGroups, TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
	    : MappingGroups(InMappingGroups)
		, Config(InConfig)
	{}

	UPROPERTY(config, EditAnywhere, Category = "Input Mapping Preset", meta = (EditCondition = "!bUseDefaultMappings"))
	TArray<FInputMappingGroup> MappingGroups;

	TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> Config;

	void SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig);

    // Returns mapping groups that which should remove bindings that conflict with ones in the given group
    TArray<int32> GetMappingGroupsToUnbind(int32 SourceMappingGroup) const;

    // Set the mappings for this preset, which constructs Mapping Groups to store them in
    void SetMappings(TArray<FInputActionKeyMapping> ActionMappings, TArray<FInputAxisKeyMapping> AxisMappings);

    // Add the given action and remove any existing actions that it should replace
    // Returns a layout containing any mappings that were unbound
    FInputMappingLayout ReplaceAction(const FConfigActionKeyMapping& Action, int32 MappingGroupId, bool bAnyKeyGroup = false);

    // Add the given axis and remove any existing axes that it should replace
    // Returns a layout containing any mappings that were unbound
    FInputMappingLayout ReplaceAxis(const FConfigAxisKeyMapping& Axis, int32 MappingGroupId, bool bAnyKeyGroup = false);

    // Unbind any actions or axes that are bound to the given chord
    // Returns a layout containing any mappings that were unbound
    FInputMappingLayout UnbindChord(FKey Key, TArray<int32> MappingGroupIds, bool ShiftDown = false, bool CtrlDown = false, bool AltDown = false, bool CmdDown = false);

    void RemoveAction(FName ActionName, int32 MappingGroupId, FGameplayTag KeyGroup, bool bRemoveFromUnbound = false);

    void RemoveAxis(FName AxisName, float Scale, int32 MappingGroupId, FGameplayTag KeyGroup, bool bRemoveFromUnbound = false, bool bIgnoreAxisKeys = false, bool bAnyScale = false);

    TArray<FInputMappingGroup>& GetMappingGroups() { return MappingGroups; }

	const TArray<FInputMappingGroup>& GetMappingGroupsConst() const { return MappingGroups; }

	bool HasMappingGroup(int32 MappingGroupIndex) const;

    FInputMappingGroup& GetMappingGroup(int32 MappingGroupIndex);

    const FInputMappingGroup& GetMappingGroupConst(int32 MappingGroupIndex) const;
    
    // Return all action mappings in this layout
    TArray<FInputActionKeyMapping> GetActions(bool bIncludeNullMappings = true) const
    {
        TArray<FInputActionKeyMapping> Actions;
        for (FInputMappingGroup Group : MappingGroups)
        {
            for (FInputActionKeyMapping& Action : Group.ActionMappings)
            {
            	if(bIncludeNullMappings || Action.Key.IsValid())
            	{
                    Actions.AddUnique(Action);
            	}
            }
        }
        return Actions;
    }

    // Return all axis mappings in this layout
    TArray<FInputAxisKeyMapping> GetAxes(bool bIncludeNullMappings = true) const
    {
        TArray<FInputAxisKeyMapping> Axes;
        for (FInputMappingGroup Group : MappingGroups)
        {
            for (FInputAxisKeyMapping& Axis : Group.AxisMappings)
            {
            	if(bIncludeNullMappings || Axis.Key.IsValid())
            	{
                    Axes.AddUnique(Axis);
            	}
            }
        }
        return Axes;
    }

	// Returns the first action that matches the given parameters
	FInputActionKeyMapping GetAction(int32 MappingGroup, FName ActionName, FGameplayTag KeyGroup = FGameplayTag()) const;

	// Returns the first axis that matches the given parameters
	FInputAxisKeyMapping GetAxis(int32 MappingGroup, FName AxisName, float Scale, FGameplayTag KeyGroup = FGameplayTag()) const;

	// Compares this layout to the given source layout and returns mappings from the source that are not bound in this one
    FInputMappingLayout FindUnboundMappings(const FInputMappingLayout& SourceLayout) const;

	// Merges mappings from the given source layout into this one, overwriting existing mappings where they conflict
    FInputMappingLayout MergeMappings(const FInputMappingLayout& OverridesLayout);

    // Merges unbound mappings
    FInputMappingLayout MergeUnboundMappings(const FInputMappingLayout& OverridesLayout);

	// Apply unbound mappings by removing them from existing mappings
    void ApplyUnboundMappings();

	// Convert all mappings into unbound mappings
    FInputMappingLayout ToUnboundMappings() const;

    void RemoveUnboundMappings();

    // Remove mappings and unbound mappings that are shared by the other mapping layout
    void RemoveRedundantMappings(const FInputMappingLayout& BaseLayout);

    // Apply this layout to the given player controller
    void Apply(APlayerController* Player);

    int32 GetTotalNumInputDefinitions() const { return GetActions().Num() + GetAxes().Num(); }

	void MarkAllMappingsDefault();
	void ConsolidateDefaultChanges(const FInputMappingLayout& BaseLayout);
	
	void DumpToLog() const;

    bool operator==(const FInputMappingLayout& Other) const
    {
        return MappingGroups == Other.MappingGroups;
    }
};
