// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/SettingControls/SliderSetting.h"

USliderSetting::USliderSetting(const FObjectInitializer& ObjectInitializer) : UAutoSettingWidget(ObjectInitializer), RightValue(1.0f)
{
}

float USliderSetting::ClampRawValue(float RawValue) const
{
	const bool bLeftMin = LeftValue < RightValue;
	return FMath::Clamp(RawValue, bLeftMin ? LeftValue : RightValue,  bLeftMin ? RightValue : LeftValue);
}

float USliderSetting::RawValueToNormalized(float RawValue) const
{
	return FMath::GetMappedRangeValueUnclamped(FVector2D(LeftValue, RightValue), FVector2D(0.0f, 1.0f), RawValue);
}

float USliderSetting::NormalizedValueToRaw(float NormalizedValue) const
{
	return FMath::GetMappedRangeValueUnclamped(FVector2D(0.0f, 1.0f), FVector2D(LeftValue, RightValue), NormalizedValue);
}

void USliderSetting::UpdateSelection_Implementation(const FString& Value)
{
	Super::UpdateSelection_Implementation(Value);

	const float RawValue = FCString::Atof(*Value);

	const float NormalizedValue = RawValueToNormalized(RawValue);

	UpdateSliderValue(NormalizedValue, RawValue);

	OnSliderValueUpdated(NormalizedValue, RawValue);

}

void USliderSetting::UpdateSliderValue_Implementation(float NormalizedValue, float RawValue)
{
}

void USliderSetting::SliderValueUpdated(float NormalizedValue)
{
	const float RawValue = NormalizedValueToRaw(NormalizedValue);

	ApplySettingValue(FString::SanitizeFloat(RawValue), ShouldSaveCurrentValue());

	OnSliderValueUpdated(NormalizedValue, RawValue);
}

bool USliderSetting::ShouldSaveCurrentValue_Implementation() const
{
	return true;
}
