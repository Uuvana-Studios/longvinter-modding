// Copyright Sam Bonifacio. All Rights Reserved.
#include "Utility/InputMappingUtils.h"
#include "AutoSettingsError.h"

bool FInputMappingUtils::IsValidPlayer(APlayerController* PlayerController, bool bLogError, const FString& ErrorContext)
{
    if(!IsValid(PlayerController))
    {
        if(bLogError)
        {
            FAutoSettingsError::LogError(FString::Printf(TEXT("%s: Invalid Player Controller object"), *ErrorContext));
        }
        return false;
    }

    if(!PlayerController->IsLocalPlayerController())
    {
        if(bLogError)
        {
            FAutoSettingsError::LogError(FString::Printf(TEXT("%s: Player Controller '%s' is not locally controlled"), *ErrorContext, *PlayerController->GetHumanReadableName()));
        }
        return false;
    }

	// There are cases where a LocalPlayerController might have it's actual LocalPlayer stolen away from it (For example, by CheatManager EnableDebugCamera, which used to cause a crash)
	// So we need to check it actually has a LocalPlayer too
    if (!IsValid(PlayerController->GetLocalPlayer()))
    {
        if (bLogError)
        {
            FAutoSettingsError::LogError(FString::Printf(TEXT("%s: Player Controller '%s' does not have a valid LocalPlayer"), *ErrorContext, *PlayerController->GetHumanReadableName()));
        }
        return false;
    }

    return true;
}
