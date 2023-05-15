/*
	This code was written by Nick Darnell
	
	Plugin created by Rama
*/
#include "VirtualCursorFunctionLibrary.h"
#include "GamepadUMGPluginPrivatePCH.h"
#include "GamepadCursorSettings.h"
#include "GameAnalogCursor.h"

bool UVirtualCursorFunctionLibrary::bVirtualCursorEnabled = false;

void UVirtualCursorFunctionLibrary::EnableVirtualCursor(class APlayerController* PC)
{
	FGameAnalogCursor::EnableAnalogCursor(PC, TSharedPtr<SWidget>());
	bVirtualCursorEnabled = true;
}

void UVirtualCursorFunctionLibrary::DisableVirtualCursor(class APlayerController* PC)
{
	FGameAnalogCursor::DisableAnalogCursor(PC);
	bVirtualCursorEnabled = false;
}

bool UVirtualCursorFunctionLibrary::IsCursorOverInteractableWidget()
{
	TSharedPtr<FGameAnalogCursor> Analog = GetDefault<UGamepadCursorSettings>()->GetAnalogCursor();
	if ( Analog.IsValid() )
	{
		return Analog->IsHovered();
	}

	return false;
}

bool UVirtualCursorFunctionLibrary::GetVirtualCursorEnabled() 
{
	return bVirtualCursorEnabled;
}