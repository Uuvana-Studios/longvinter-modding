// Copyright Sam Bonifacio. All Rights Reserved.
#include "InputMappingPreset.h"

FInputMappingPreset::FInputMappingPreset(FGameplayTag InTag, TArray<FInputActionKeyMapping> ActionMappings, TArray<FInputAxisKeyMapping> AxisMappings)
{
	PresetTag = InTag;

	InputLayout.SetMappings(ActionMappings, AxisMappings);
}

void FInputMappingPreset::SetConfig(TWeakInterfacePtr<const IAutoSettingsInputConfigInterface> InConfig)
{
	InputLayout.SetConfig(InConfig);
}
