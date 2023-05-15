// Copyright Epic Games, Inc. All Rights Reserved.

#include "LongvinterGameMode.h"
#include "LongvinterCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALongvinterGameMode::ALongvinterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
