// Copyright Sam Bonifacio. All Rights Reserved.
#pragma once
#include "UI/ActionMapping.h"

class FInputMappingUtils
{
public:

	// Checks if the given Player Controller is valid for input mapping purposes (is a local player)
	static bool IsValidPlayer(APlayerController* PlayerController, bool bLogError = false, const FString& ErrorContext = FString());
	
};

inline FString ToString(FInputActionKeyMapping Mapping)
{
	return FString::Printf(TEXT("ActionName: %s, Key: %s, bShift: %i, bCtrl: %i, bAlt: %i, bCmd: %i"),
	*Mapping.ActionName.ToString(),
	*Mapping.Key.ToString(),
	(int32)Mapping.bShift,
	(int32)Mapping.bCtrl,
	(int32)Mapping.bAlt,
	(int32)Mapping.bCmd);
}

inline FString ToString(FInputAxisKeyMapping Mapping)
{
	return FString::Printf(TEXT("AxisName: %s, Key: %s, Scale: %f"),
	*Mapping.AxisName.ToString(),
	*Mapping.Key.ToString(),
	Mapping.Scale);
}
