// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "OnlineLoginStatus.generated.h"

UENUM(BlueprintType, Meta = (ScriptName = "ELoginStatus"))
enum class EOnlineLoginStatus : uint8
{
    /** Player has not logged in or chosen a local profile */
    NotLoggedIn,
    /** Player is using a local profile but is not logged in */
    UsingLocalProfile,
    /** Player has been validated by the platform specific authentication service */
    LoggedIn
};