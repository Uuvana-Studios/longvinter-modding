// Copyright Sam Bonifacio. All Rights Reserved.

#include "ConfigUtils.h"
#include "UObject/UnrealType.h"
#include "Misc/ConfigCacheIni.h"

void FConfigUtils::MigrateConfigPropertiesFromSection(UObject* Object, FString OldSectionName)
{
	const bool bDefaultConfig = Object->GetClass()->HasAnyClassFlags(CLASS_DefaultConfig);
	const FString FileName = bDefaultConfig ? Object->GetDefaultConfigFilename() : Object->GetClass()->GetConfigName();
	const FString NewSectionName = Object->GetClass()->GetPathName();

	bool bModified = false;

	// Iterate properties
	for ( FProperty* Property = Object->GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
	{
		if ( !Property->HasAnyPropertyFlags(CPF_Config) )
		{
			continue;
		}

		FArrayProperty* Array = CastField<FArrayProperty>( Property );

		FString Key;
		TArray<FString> Values;
		if(Array)
		{
			// Get array values
			if(bDefaultConfig)
			{
				Key = FString::Printf(TEXT("+%s"), *Property->GetName());
			}
			else
			{
				Key = Property->GetName();
			}
			GConfig->GetArray(*OldSectionName, *Key, Values, FileName);
		}
		else
		{
			// Get single value
			Key = Property->GetName();
			FString Value;
			const bool bFound = GConfig->GetString(*OldSectionName, *Key, Value, FileName);
			if(bFound)
			{
				Values.Add(Value);
			}
		}

		if(Values.Num() > 0)
		{
			// Add to new section
			GConfig->SetArray(*NewSectionName, *Key, Values, FileName);
			// Remove from old section
			GConfig->RemoveKey(*OldSectionName, *Key, FileName);

			bModified = true;
		}

	}
	
	if(bModified)
	{
		FConfigCacheIni* Cache = GConfig;
		// If properties were moved, flush the GConfig
		Cache->Flush(false, FileName);
		// Reload so that we get new values immediately
		GConfig->LoadGlobalIniFile(GGameIni, TEXT("Game"), nullptr, true);
	}
}
