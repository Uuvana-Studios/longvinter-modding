// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "GameFramework/PlayerInput.h"
#include "KeyMappingTypes.generated.h"

// Sadly the mapping names aren't editable in config unless using these child classes
// Config editable action mapping
USTRUCT(BlueprintType)
struct AUTOSETTINGSINPUT_API FConfigActionKeyMapping : public FInputActionKeyMapping
{
	GENERATED_BODY()

	FConfigActionKeyMapping() {}

	/**
	 * True if this mapping was using the default key from the base preset at the time it was saved
	 * This allows us to keep it in sync if the default mapping is changed
	 */ 
	UPROPERTY()
	bool bIsDefault = false;

	FConfigActionKeyMapping(FInputActionKeyMapping Base);

	FString ToDebugString() const
	{
		return FString::Printf(TEXT("ActionName: %s, Key: %s, bShift: %i, bCtrl: %i, bAlt: %i, bCmd: %i, bIsDefault: %i"),
            *ActionName.ToString(),
            *Key.ToString(),
            (int32)bShift,
            (int32)bCtrl,
            (int32)bAlt,
            (int32)bCmd,
            (int32)bIsDefault);
	}

	bool operator!=(const FConfigActionKeyMapping& Other) const
	{
		return !(*this == Other);
	}
};

// Config editable axis mapping
USTRUCT(BlueprintType)
struct AUTOSETTINGSINPUT_API FConfigAxisKeyMapping : public FInputAxisKeyMapping
{
	GENERATED_BODY()

	FConfigAxisKeyMapping() {}

	/**
	 * True if this mapping was using the default key from the base preset at the time it was saved
	 * This allows us to keep it in sync if the default mapping is changed
	 */ 
	UPROPERTY()
	bool bIsDefault = false;

	FConfigAxisKeyMapping(FInputAxisKeyMapping Base);

	FString ToDebugString() const
	{
		return FString::Printf(TEXT("AxisName: %s, Key: %s, Scale: %f, bIsDefault: %i"),
            *AxisName.ToString(),
            *Key.ToString(),
            Scale,
            (int32)bIsDefault);
	}

	bool operator!=(const FConfigAxisKeyMapping& Other) const
	{
		return !(*this == Other);
	}
};
