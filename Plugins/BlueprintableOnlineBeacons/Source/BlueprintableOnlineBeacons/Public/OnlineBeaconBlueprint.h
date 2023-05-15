// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineBeaconClient.h"
#include "FindSessionsCallbackProxy.h"
#include "OnlineBeaconBlueprint.generated.h"

/**
 * Reimplements the State of the Connection for Blueprint Use.
 */
UENUM(BlueprintType)
enum class EBeaconConnectionStateBlueprint : uint8 {
    BCE_Invalid UMETA(DisplayName = "Invalid", Tooltip = "Connection is invalid, possibly uninitialized."),
    BCE_Closed  UMETA(DisplayName = "Closed", Tooltip = "Connection permanently closed."),
    BCE_Pending UMETA(DisplayName = "Pending", Tooltip = "Connection is pending."),
    BCE_Open    UMETA(DisplayName = "Open", Tooltip = "Connection is open.")
};

/**
 * The Reason for a Beacon getting Disconnected.
 */
UENUM(BlueprintType)
enum class EBeaconDisconnectReason : uint8 {
    BDR_Client UMETA(DisplayName = "Client", Tooltip = "This Beacon was disconnected by the Client at its own request."),
    BDR_Server UMETA(DisplayName = "Server", Tooltip = "This Beacon was disconnected by the Server.")
};

/**
 * Allows Blueprint Implementation for a Online Beacon.
 * RPCs created on this Actor replicates between the Client/Server connection.
 */
UCLASS(Blueprintable)
class BLUEPRINTABLEONLINEBEACONS_API AOnlineBeaconBlueprint : public AOnlineBeaconClient {
	GENERATED_BODY()

    private:

        // Last Beacon Connection State.
        EBeaconConnectionStateBlueprint LastConnectionState;

        // Our Tick Timer.
        FTimerHandle TickTimer;

        // Override Begin Play to bind Timer.
        virtual void BeginPlay() override;

        // Override End Play.
        virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
        // Our custom tick.
        void DoLocalTick();
 
    public:

        /**
         * Call this function to begin connection to a server, given an Address.
         * Returns if connection was success.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Connection|Client")
        bool ConnectByIP(FString Address);

        /**
         * Call this function to begin connection to a server, given its Session Result.
         * Returns if connection was success.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Connection|Client")
        bool ConnectBySession(const FBlueprintSessionResult& Session);

        // Overrides the default On Connected functionality to call Blueprint Events.
        virtual void OnConnected() override;

        /**
         * Called when the Beacon has successfully connected.
         */
        UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Connected"), Category = "OnlineBeacon|Connection|Client")
        void OnConnectedBlueprint();

        /**
         * Called when the Beacon has failed its connection to the server.
         */
        UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Connection Failed"), Category = "OnlineBeacon|Connection|Client")
        void OnConnectionFailureBlueprint();

        // Overrides the default Destroy Beacon to call Blueprint Events.
        virtual void DestroyBeacon() override;

        /**
         * Called when the Beacon is disconnecting.
         */
        UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Disconnected"), Category = "OnlineBeacon|Connection|Client")
        void OnDisconnected(EBeaconDisconnectReason Reason);
 
        // Overrides the default Handle Network Failure to call Blueprint Events.
        virtual void HandleNetworkFailure(UWorld* World, UNetDriver *NtDrv, ENetworkFailure::Type FailureType, const FString& ErrorString) override;
 
        /**
         * Called when the beacon finds a network failure.
         */
        UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Network Failure"), Category = "OnlineBeacon|Connection|Client")
        void OnNetworkFailure(ENetworkFailure::Type FailureType);

        /**
         * Called when the beacon connection state changes.
         */
        UFUNCTION(BlueprintImplementableEvent, Category = "OnlineBeacon|Connection|Client")
        void OnConnectionStateChanged(EBeaconConnectionStateBlueprint From, EBeaconConnectionStateBlueprint To);
 
        /**
         * Call this function to disconnect this Beacon from a server.
         * Be warned that calling this WILL destroy your Actor.
         */
        UFUNCTION(BlueprintCallable, Category = "OnlineBeacon|Connection|Client")
        void Disconnect();

        /**
         * Gets the current state of the Beacon connection, for easy blueprint use.
         */
        UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Connection State"), Category = "OnlineBeacon|Utilities|Client")
        EBeaconConnectionStateBlueprint GetConnectionStateBlueprint() const;

        /**
         * This event is called on the server when a new client connects.
         */
        UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, meta = (DisplayName = "On Client Connected"), Category = "OnlineBeacon|Connection|Server")
        void OnClientConnected(AOnlineBeaconBlueprint* Client);

        /**
         * This event is called on the server when a new client disconnects.
         */
        UFUNCTION(BlueprintImplementableEvent, BlueprintAuthorityOnly, meta = (DisplayName = "On Client Disconnected"), Category = "OnlineBeacon|Connection|Server")
        void OnClientDisconnected(AOnlineBeaconBlueprint* Client);

        /**
         * This function will return a list of all clients connected on this beacon.
         */
        UFUNCTION(BlueprintPure, BlueprintAuthorityOnly, Category = "OnlineBeacon|Utilities|Server")
        TArray<AOnlineBeaconBlueprint*> GetConnectedClients() const;

        /**
         * This function disconnects a client from this beacon.
         */
        UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "OnlineBeacon|Connection|Server")
        void DisconnectClient(AOnlineBeaconBlueprint* Client) const;
};
