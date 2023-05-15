// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/SettingControls/ComboBoxSetting.h"

void UComboBoxSetting::NativeConstruct()
{
	Super::NativeConstruct();

	if(IsValid(ComboBox))
		ComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UComboBoxSetting::ComboBoxSelectionChanged);
}

void UComboBoxSetting::UpdateSelection_Implementation(const FString& Value)
{
	Super::UpdateSelection_Implementation(Value);

	if (IsValid(ComboBox))
	{
		// If we don't have an option for the supplied value, use the value directly
		// This is not a perfect solution but it's better than having blank text show as the selection
		
		const bool bHasOption = HasOptionWithValue(Value);
		const FString Selection = bHasOption ? GetOptionByValue(Value).Label.ToString() : Value;

		if(!bHasOption && ComboBox->FindOptionIndex(Selection) == -1)
		{
			ComboBox->AddOption(Selection);
		}

		ComboBox->SetSelectedOption(Selection);
	}
}

void UComboBoxSetting::UpdateOptions_Implementation(const TArray<FSettingOption>& InOptions)
{
	Super::UpdateOptions_Implementation(InOptions);

	if (IsValid(ComboBox))
	{
		const FString SelectedOption = ComboBox->GetSelectedOption();
		
		ComboBox->ClearOptions();

		for (FSettingOption Option : InOptions)
		{
			ComboBox->AddOption(Option.Label.ToString());
		}

		// Preserve previously selected value
		ComboBox->SetSelectedOption(SelectedOption);
	}

}

void UComboBoxSetting::ComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!IsUpdatingSettingSelection() && !IsUpdatingSettingOptions()) // Ignore selection changes while the setting is updating selection or options
	{
		// Look up the option based on the label, if not use the selected item directly
		const FText SelectedItemLabel = FText::FromString(SelectedItem);
		const FString Selection = HasOptionWithLabel(SelectedItemLabel) ? GetOptionByLabel(SelectedItemLabel).Value : SelectedItem;
		ApplySettingValue(Selection);
	}
}
