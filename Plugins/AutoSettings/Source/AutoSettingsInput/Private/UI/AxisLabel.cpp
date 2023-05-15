// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/AxisLabel.h"
#include "InputMappingManager.h"


void UAxisLabel::InitializeLabel()
{
	UpdateLabel();
}

void UAxisLabel::UpdateLabel_Implementation()
{
	const FInputAxisKeyMapping Mapping = UInputMappingManager::Get()->GetPlayerAxisMapping(GetOwningPlayer(), AxisName, Scale, MappingGroup, KeyGroup, bUsePlayerKeyGroup);

	KeyLabel->Key = Mapping.Key;
	KeyLabel->AxisScale = Scale; // Use the scale from the widget, not the mapping. If the mapping is bound to an axis key like a thumbstick, the mapping itself won't have the scale of the widget.
	if(Mapping.Scale != 0.f)
	{
		// This is where it gets a bit weird.
		// Unreal inverts the gamepad right stick Y axis for whatever reason, and from any public communication they don't intend to fix it because of backwards compatibility concerns.
		// Normally, projects with right stick controlling the camera would have LookUpRate bound to Right Stick Y Axis with a scale of -1 to "invert back" the Unreal inversion.
		// So at this point just using the Scale of the mapping widget would be incorrect as an axis "LookUpRate 1" for the "Look Up" binding would result in "Right Stick Down".
		// However, we know that in this case the actual mapping has an inverted scale of -1, so in that case we can use that to flip the scale back and get "Right Stick Up" again.
		// What a garbage fire, right?
		KeyLabel->AxisScale *= Mapping.Scale;
	}
	KeyLabel->IconTags = IconTags;

	KeyLabel->UpdateKeyLabel();
}

void UAxisLabel::NativePreConstruct()
{
	Super::NativePreConstruct();

	UpdateLabel();
}
