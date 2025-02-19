// Copyright Qibo Pang 2022. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "ImageCacheCommon.generated.h"

UENUM(BlueprintType)
enum class EFileTypeDesc : uint8
{
	None,
	ImageFile,
	BinaryData,
};

