// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.


#include "OnlineBeaconsSettings.h"
#include "BlueprintableOnlineBeacons.h"

#define BEACON_CONFIG_SECTION TEXT("/Script/OnlineSubsystemUtils.OnlineBeaconHost")
#define BEACON_CONFIG_PROPERTY_LISTENPORT TEXT("ListenPort")
#define BEACON_CONFIG_PROPERTY_INITIALTIMEOUT TEXT("BeaconConnectionInitialTimeout")
#define BEACON_CONFIG_PROPERTY_TIMEOUT TEXT("BeaconConnectionTimeout")

#define DEFAULT_ENGINE_INI *FString("DefaultEngine")
#define ENGINE_CONFIG_SECTION TEXT("/Script/Engine.Engine")

#define BEACON_NET_DRIVER "BeaconNetDriver"
#define NET_DRIVER_DEFINITIONS "NetDriverDefinitions"
#define NET_DRIVER_DEFINITIONS_INCREMENT "+NetDriverDefinitions"
#define NET_DRIVER_DEFINITIONS_DEFNAME "DefName"
#define NET_DRIVER_DEFINITIONS_DRIVERCLASSNAME "DriverClassName"
#define NET_DRIVER_DEFINITIONS_DRIVERCLASSNAMEFALLBACK "DriverClassNameFallback"

#define ERROR_ENGINE_INI TEXT("DefaultEngine.ini file could not be loaded.")

// Constructor.
UOnlineBeaconsSettings::UOnlineBeaconsSettings(const FObjectInitializer& ObjectInitializer) {
	// Load Settings.
	LoadBeaconSettings();
}

// Called to load settings.
void UOnlineBeaconsSettings::LoadBeaconSettings() {
	// If we should save new Settings.
	bool SaveSettings = false;

	// The Default Engine Ini.
	FConfigFile DefaultEngine;

	// Attempt to Load it.
	if (!FConfigCacheIni::LoadExternalIniFile(DefaultEngine, DEFAULT_ENGINE_INI, NULL, *FPaths::ProjectConfigDir(), false)) {
		// Notify Error.
		UE_LOG(LogBlueprintableOnlineBeacons, Error, ERROR_ENGINE_INI);

		// Stop Execution.
		return;
	}


	// Get Listen Port from Config.
	if (!DefaultEngine.GetInt(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_LISTENPORT, ListenPort)) {
		// Default it to 7787.
		ListenPort = 7787;

		// Notify Save Settings.
		SaveSettings = true;
	}

	// Get the Initial Timeout.
	if (!DefaultEngine.GetFloat(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_INITIALTIMEOUT, InitialTimeout)) {
		// Default it to 25.
		InitialTimeout = 25.f;

		// Notify Save Settings.
		SaveSettings = true;
	}

	// Get the Timeout.
	if (!DefaultEngine.GetFloat(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_TIMEOUT, Timeout)) {
		// Default it to 30.
		Timeout = 30.f;

		// Notify Save Settings.
		SaveSettings = true;
	}

	// Get Definitions.
	TArray<FNetDriverDefinition> Definitions = GetDriverDefinitionsFromConfig(DefaultEngine);

	// Find our Definition.
	FNetDriverDefinition* BeaconDef = Definitions.FindByPredicate([](const FNetDriverDefinition& a) { return a.DefName == BEACON_NET_DRIVER; });

	// If we have the definition.
	if (BeaconDef) {
		// Load it.
		NetDriverClass = BeaconDef->DriverClassName;
		NetDriverFallbackClass = BeaconDef->DriverClassNameFallback;
	} else {
		// Set Default Values.
		NetDriverClass = NetDriverFallbackClass = FName("OnlineSubsystemUtils.IpNetDriver");

		// Notify Save Settings, which is going to set our New Driver Definitions.
		SaveSettings = true;
	}

	// If we are to save settings, save settings.
	if (SaveSettings)
		SaveBeaconSettings();
}

// Called to save settings.
bool UOnlineBeaconsSettings::SaveBeaconSettings() {
	// The Default Engine Ini.
	FConfigFile DefaultEngine;

	// Attempt to Load it.
	if (!FConfigCacheIni::LoadExternalIniFile(DefaultEngine, DEFAULT_ENGINE_INI, NULL, *FPaths::ProjectConfigDir(), false)) {
		// Notify Error.
		UE_LOG(LogBlueprintableOnlineBeacons, Error, ERROR_ENGINE_INI);

		// Stop Execution.
		return false;
	}

	// Define our Conversion String.
	TCHAR Conversion[MAX_SPRINTF] = TEXT("");

	// Set Listen Port.
	FCString::Sprintf(Conversion, TEXT("%i"), ListenPort);
	DefaultEngine.SetString(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_LISTENPORT, Conversion);

	// Set Initial Timeout.
	FCString::Sprintf(Conversion, TEXT("%f"), InitialTimeout);
	DefaultEngine.SetString(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_INITIALTIMEOUT, Conversion);

	// Set Timeout.
	FCString::Sprintf(Conversion, TEXT("%f"), Timeout);
	DefaultEngine.SetString(BEACON_CONFIG_SECTION, BEACON_CONFIG_PROPERTY_TIMEOUT, Conversion);

	// Get all Current Net Driver Definitions.
	TArray<FNetDriverDefinition> Definitions = GetDriverDefinitionsFromConfig(DefaultEngine);

	// Find our Definition.
	FNetDriverDefinition* BeaconDef = Definitions.FindByPredicate([](const FNetDriverDefinition& a) { return a.DefName == BEACON_NET_DRIVER; });

	// If we found it, set data.
	if (BeaconDef) {
		// Set Values.
		BeaconDef->DriverClassName = NetDriverClass;
		BeaconDef->DriverClassNameFallback = NetDriverFallbackClass;
	} else {
		// Create new Definition.
		FNetDriverDefinition Def;
		
		// Set Default Values.
		Def.DefName = BEACON_NET_DRIVER;
		Def.DriverClassName = NetDriverClass;
		Def.DriverClassNameFallback = NetDriverFallbackClass;

		// Add it to List.
		Definitions.Add(Def);
	}

	// Get the Config Section.
	FConfigSection* Section = DefaultEngine.Find(ENGINE_CONFIG_SECTION);

	// Check for Section, create if invalid.
	if (!Section)
		Section = &DefaultEngine.Add(ENGINE_CONFIG_SECTION);

	// Remove all Definitions.
	Section->Remove(FName(NET_DRIVER_DEFINITIONS));

	// Remove all Increment Definitions.
	Section->Remove(FName(NET_DRIVER_DEFINITIONS_INCREMENT));

	// Add all Incrementations.
	for (FNetDriverDefinition Def : Definitions)
		Section->Add(FName(NET_DRIVER_DEFINITIONS_INCREMENT), FConfigValue("(DefName=\"" + Def.DefName.ToString() + "\",DriverClassName=\"" + Def.DriverClassName.ToString() + "\",DriverClassNameFallback=\"" + Def.DriverClassNameFallback.ToString() + "\")"));

	// Mark the Config as Dirty.
	DefaultEngine.Dirty = true;
	
	// Write the Config.
	DefaultEngine.Write(FPaths::ProjectConfigDir() + "/" + DEFAULT_ENGINE_INI + ".ini");

	// Return true.
	return true;
}

// Override Post Edit Change Property.
void UOnlineBeaconsSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	// Make sure we save the beacon settings.
	SaveBeaconSettings();
}

// Using a Config File, returns all NetDriverDefinitions.
TArray<FNetDriverDefinition> UOnlineBeaconsSettings::GetDriverDefinitionsFromConfig(FConfigFile Config) {
	// Our Results Array.
	TArray<FNetDriverDefinition> Results;

	// Get the Normal Definitions.
	TArray<FString> Definitions;
	Config.GetArray(ENGINE_CONFIG_SECTION, TEXT(NET_DRIVER_DEFINITIONS), Definitions);

	// Get the Incremental Definitions.
	TArray<FString> IncrementalDefinitions;
	Config.GetArray(ENGINE_CONFIG_SECTION, TEXT(NET_DRIVER_DEFINITIONS_INCREMENT), IncrementalDefinitions);

	// Add to Definitions Array.
	Definitions.Append(IncrementalDefinitions);

	// Loop all Net Driver Definitions until we find the Beacon.
	for (FString Def : Definitions) {
		// A Values Map, used to parse the definitions.
		TMap<FString, FString> Values;

		// Booleans used for Definition Parse Control.
		bool GettingName = false;
		bool GettingValue = false;

		// The Values just gathered from the Definition String.
		FString GetName, GetValue;

		// Loop all Characters.
		for (TCHAR Char : Def.GetCharArray()) {
			// If we've got a '(' or a ')' or a space and finally or a ',', we just continue to next character.
			if (Char == L'(' || Char == L')' || Char == L' ' || Char == L',')
				continue;

			// If we aren't getting a name and a value, make sure we start getting a name.
			if (!GettingName && !GettingValue)
				GettingName = true;

			// If we are getting a name or a value.
			if (GettingName) {
				// If we find an equals...
				if (Char == L'=') {
					// Start getting the value.
					GettingValue = true;

					// Stop getting the name.
					GettingName = false;

					// Continue to next character.
					continue;
				}

				// Add this character to the name.
				GetName += Char;
			} else if (GettingValue) {
				// If we find a '"' and it is the 2nd one we find...
				if (Char == L'\"' && GetValue.Len() > 0) {
					// Stop getting the value.
					GettingValue = false;

					// Add to the Values Map.
					Values.Add(GetName, GetValue);

					// Reset Get Name and Value.
					GetName = GetValue = FString();

					// Continue to next.
					continue;
				} else if (Char == L'\"')
					continue;

				// Add this character to the value.
				GetValue += Char;
			}
		}

		// Make sure it is a valid net driver.
		if (!Values.Contains(NET_DRIVER_DEFINITIONS_DEFNAME) || !Values.Contains(NET_DRIVER_DEFINITIONS_DRIVERCLASSNAME) || !Values.Contains(NET_DRIVER_DEFINITIONS_DRIVERCLASSNAMEFALLBACK))
			continue;

		// Create a new Driver Definition.
		FNetDriverDefinition DriverDef;

		// Set Values.
		DriverDef.DefName = FName(*Values[NET_DRIVER_DEFINITIONS_DEFNAME]);
		DriverDef.DriverClassName = FName(*Values[NET_DRIVER_DEFINITIONS_DRIVERCLASSNAME]);
		DriverDef.DriverClassNameFallback = FName(*Values[NET_DRIVER_DEFINITIONS_DRIVERCLASSNAMEFALLBACK]);

		// Add to Results.
		Results.Add(DriverDef);
	}

	// Return Results.
	return Results;
}