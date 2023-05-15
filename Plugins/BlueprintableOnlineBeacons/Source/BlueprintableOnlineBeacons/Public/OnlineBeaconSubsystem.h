// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OnlineBeaconHost.h"
#include "OnlineBeaconClient.h"
#include "OnlineBeaconHostObjectDynamic.h"
#include "OnlineBeaconSubsystem.generated.h"

/**
 * This Subsystem automatically creates all necessary actors 
 *
 */
UCLASS()
class BLUEPRINTABLEONLINEBEACONS_API UOnlineBeaconSubsystem : public UWorldSubsystem {
	GENERATED_BODY()
	
    private:

        // A Map of all Current Beacons that we are hosting, with their respective hosts.
        UPROPERTY()
        TMap<TSubclassOf<class AOnlineBeaconClient>, AOnlineBeaconHostObjectDynamic*> HostingBeacons;

        // The Current Host Beacon.
        UPROPERTY()
        AOnlineBeaconHost* HostBeacon;

    public:

        // Override Initialize.
        virtual void Initialize(FSubsystemCollectionBase& Collection) override;

        /**
         * Starts hosting a Online Beacon.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Subsystem|Hosting")
        void StartHostingBeacon(TSubclassOf<class AOnlineBeaconClient> Class);

        /**
         * Starts hosting multiple Online Beacons at once.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Subsystem|Hosting")
        void StartHostingBeacons(const TArray<TSubclassOf<class AOnlineBeaconClient>> Classes);

        /**
         * Stops hosting a Online Beacon.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Subsystem|Hosting")
        void StopHostingBeacon(TSubclassOf<class AOnlineBeaconClient> Class);

        /**
         * Stops hosting multiple Online Beacons at once.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Subsystem|Hosting")
        void StopHostingBeacons(const TArray<TSubclassOf<class AOnlineBeaconClient>> Classes);

        /**
         * Returns whether a specific beacon class is being hosted.
         */
        UFUNCTION(BlueprintPure, Category = "OnlineBeacon|Subsystem|Utilities")
        bool IsHostingBeacon(TSubclassOf<class AOnlineBeaconClient> Class);


        /**
         *
         */
};
