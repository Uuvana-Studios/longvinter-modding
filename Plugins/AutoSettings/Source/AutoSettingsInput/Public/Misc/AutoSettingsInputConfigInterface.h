// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AutoSettingsInputConfigInterface.generated.h"

struct FKey;
struct FInputMappingPreset;
struct FKeyGroup;
struct FGameplayTag;

UINTERFACE()
class UAutoSettingsInputConfigInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface to access input config
 */
class AUTOSETTINGSINPUT_API IAutoSettingsInputConfigInterface
{
	GENERATED_BODY()

public:

	virtual bool ShouldAllowMultipleBindingsPerKey() const = 0;

	// True if bindings should be kept unique between two mapping groups
	virtual bool ShouldBindingsBeUniqueBetweenMappingGroups(int32 MappingGroupA, int32 MappingGroupB) const = 0;

	// Returns the input preset with the given tag, or the first input preset if none are found with the tag
	virtual FInputMappingPreset GetInputPresetByTag(FGameplayTag PresetTag) const = 0;
	
	virtual TArray<FName> GetPreservedActions() const = 0;
	
	virtual TArray<FName> GetPreservedAxes() const = 0;
	
	virtual const TArray<FKeyGroup>& GetKeyGroups() const = 0;

	virtual FGameplayTag GetKeyGroupOfKey(FKey Key) const = 0;
	
	// True if there is actually a key group definition with the given tag
	virtual bool IsKeyGroupDefined(FGameplayTag KeyGroupTag) const = 0;

	// True if the given key is an axis key in any stored Axis Association
	virtual bool IsAxisKey(FKey Key) const = 0;

};
