// Copyright Sam Bonifacio. All Rights Reserved.

#include "UI/InputLabel.h"
#include "InputMappingManager.h"

UInputLabel::UInputLabel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), MappingGroup(-1), bUsePlayerKeyGroup(true)
{
}

void UInputLabel::UpdateLabel_Implementation()
{

}

void UInputLabel::NativeConstruct()
{
	Super::NativeConstruct();

	UInputMappingManager::Get()->OnMappingsChanged.AddUniqueDynamic(this, &UInputLabel::MappingsChanged);

	UpdateLabel();
}

void UInputLabel::MappingsChanged(APlayerController* Player)
{
	if (Player == GetOwningPlayer())
	{
		UpdateLabel();
	}
}
