// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/AxisMapping.h"
#include "InputMappingManager.h"

void UAxisMapping::NativeConstruct()
{
	Super::NativeConstruct();
}

void UAxisMapping::UpdateLabel_Implementation()
{
	GetAxisLabel()->AxisName = AxisName;
	GetAxisLabel()->Scale = Scale;
	GetAxisLabel()->MappingGroup = MappingGroup;
	GetAxisLabel()->KeyGroup = KeyGroup;
	GetAxisLabel()->bUsePlayerKeyGroup = false;
	GetAxisLabel()->IconTags = IconTags;
	GetAxisLabel()->InitializeLabel();
}

void UAxisMapping::BindChord(FCapturedInput CapturedInput)
{
	const FInputChord Chord = CapturedInput.Chord;
	FKey Key = Chord.Key;

	FKeyScale AxisKeyScale = UInputMappingManager::GetInputConfigStatic()->GetAxisKey(Key);
	const bool bUseAxisKey = AxisKeyScale.Key.IsValid();

	const FKey FinalKey = bUseAxisKey ? AxisKeyScale.Key : Key;

	// Just multiply all the scales, it'll be fine...
	float FinalScale = Scale * CapturedInput.AxisScale;
	if(bUseAxisKey)
	{
		FinalScale *= AxisKeyScale.Scale;
	}

	const FInputAxisKeyMapping NewMapping = FInputAxisKeyMapping(AxisName, FinalKey, FinalScale);

	// Override all key groups if none is specified
	const bool bAnyKeyGroup = !KeyGroup.IsValid();

	UInputMappingManager::Get()->AddPlayerAxisOverride(GetOwningPlayer(), NewMapping, MappingGroup, bAnyKeyGroup);
}
