/*
	This code was written by Nick Darnell
	
	Plugin created by Rama
*/
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "VirtualCursorFunctionLibrary.generated.h"
  
/**
 * 
 */
UCLASS()
class GAMEPADUMGPLUGIN_API UVirtualCursorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool bVirtualCursorEnabled;

	UFUNCTION(BlueprintCallable, Category="Game")
	static void EnableVirtualCursor(class APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void DisableVirtualCursor(class APlayerController* PC);

	UFUNCTION(BlueprintPure, Category="Game")
	static bool IsCursorOverInteractableWidget();

	UFUNCTION(BlueprintPure, Category = "Game")
	static bool GetVirtualCursorEnabled();
};
