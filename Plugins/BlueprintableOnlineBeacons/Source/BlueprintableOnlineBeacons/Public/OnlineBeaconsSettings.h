// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine.h"
#include "OnlineBeaconsSettings.generated.h"

/**
 * Used to configure the default Beacon Settings.
 */
UCLASS()
class BLUEPRINTABLEONLINEBEACONS_API UOnlineBeaconsSettings : public UObject {
	GENERATED_BODY()
	
	public:
		
		// Constructor.
		UOnlineBeaconsSettings(const FObjectInitializer& ObjectInitializer);

		// Called to load settings.
		void LoadBeaconSettings();

		// Called to save settings.
		bool SaveBeaconSettings();

		// Override Post Edit Change Property.
		virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent);

		// Using a Config File, returns all NetDriverDefinitions.
		static TArray<FNetDriverDefinition> GetDriverDefinitionsFromConfig(FConfigFile Config);

		// The port Beacons will try connecting to.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OnlineBeacons")
		int32 ListenPort;

		// The initial timeout for Online Beacons (Value needs to be less than the Timeout).
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OnlineBeacons")
		float InitialTimeout;

		// The timeout for Online Beacons.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OnlineBeacons")
		float Timeout;

		/**
		 * Used by the Engine for the Beacon Net Driver.
		 * And engine RESTART is required for this to be applied.
		 * Some default values are:
		 * OnlineSubsystemUtils.IpNetDriver (for Online Subsystem Null)
		 * OnlineSubsystemSteam.SteamNetDriver (for default Steam Subsystem)
		 * /Script/SteamSockets.SteamSocketsNetDriver (for Steam Sockets)
		 */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OnlineBeacons|NetDriver")
		FName NetDriverClass;

		/**
		 * Used by the Engine for the Beacon Net Driver as Fallback on the case the first Driver fails to Initialize.
		 * An engine RESTART is required for this to be applied.
		 * Some default values are:
		 * OnlineSubsystemUtils.IpNetDriver (for Online Subsystem Null)
		 * OnlineSubsystemSteam.SteamNetDriver (for default Steam Subsystem)
		 * /Script/SteamSockets.SteamSocketsNetDriver (for Steam Sockets)
		 */
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "OnlineBeacons|NetDriver")
		FName NetDriverFallbackClass;
};
