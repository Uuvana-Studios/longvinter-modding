// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/KeyLabel.h"
#include "GlobalKeyIconTagManager.h"
#include "InputMappingManager.h"
#include "Engine/Texture.h"
#include "Misc/AutoSettingsInputConfig.h"

UKeyLabel::UKeyLabel(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	KeyInvalidText = NSLOCTEXT("AutoSettings", "KeyInvalid", "-");
}

FText UKeyLabel::GetDisplayName() const
{
	if (!LabelOverride.IsEmpty())
	{
		return LabelOverride;
	}

	if (!HasValidKey())
	{
		return KeyInvalidText;
	}

	return UInputMappingManager::GetInputConfigStatic()->GetKeyFriendlyName(Key);
}

UTexture * UKeyLabel::GetIcon() const
{
	return UGlobalKeyIconTagManager::Get()->GetIconForKey(Key, IconTags, AxisScale);
}

bool UKeyLabel::HasIcon() const
{
	return GetIcon() != nullptr;
}

bool UKeyLabel::HasValidKey() const
{
	return Key.IsValid();
}

FSlateBrush UKeyLabel::GetIconBrush_Implementation() const
{
	FSlateBrush Brush = FSlateBrush();
	Brush.SetResourceObject(GetIcon());
	return Brush;
}

ESlateVisibility UKeyLabel::GetIconVisibility_Implementation() const
{
	return HasIcon() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
}

ESlateVisibility UKeyLabel::GetDisplayNameVisibility_Implementation() const
{
	return HasIcon() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible;
}

void UKeyLabel::NativeConstruct()
{
	Super::NativeConstruct();

	UGlobalKeyIconTagManager::Get()->OnGlobalKeyIconTagsModified.AddUniqueDynamic(this, &ThisClass::OnGlobalKeyIconTagsModified);
}

void UKeyLabel::OnGlobalKeyIconTagsModified()
{
	// Key icon may have changed based on global tags, so refresh
	UpdateKeyLabel();
}
