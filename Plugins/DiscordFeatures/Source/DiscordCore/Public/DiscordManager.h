// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DiscordManager.generated.h"

class UDiscordCore;

/**
 *	Base class for Discord Manager.
 */
UCLASS(Abstract)
class DISCORDCORE_API UDiscordManager : public UObject
{
	GENERATED_BODY()
public:

protected:
	TWeakObjectPtr<UDiscordCore> DiscordCore;
};
