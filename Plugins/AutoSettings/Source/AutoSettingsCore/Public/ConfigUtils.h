// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class AUTOSETTINGSCORE_API FConfigUtils
{
public:

	static void MigrateConfigPropertiesFromSection(UObject* Object, FString OldSection);
	
};
