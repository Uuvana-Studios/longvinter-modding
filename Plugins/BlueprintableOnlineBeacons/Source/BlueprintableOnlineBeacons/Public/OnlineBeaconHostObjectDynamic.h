// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "OnlineBeaconBlueprint.h"
#include "OnlineBeaconHostObject.h"
#include "OnlineBeaconHostObjectDynamic.generated.h"

/**
 * Exposes the Beacon Class for the Online Beacon Subsystem.
 */
UCLASS()
class BLUEPRINTABLEONLINEBEACONS_API AOnlineBeaconHostObjectDynamic : public AOnlineBeaconHostObject {
	GENERATED_BODY()

	public:

		// Constructor.
		AOnlineBeaconHostObjectDynamic();

		// Sets a new Beacon Class for this Host Object.
		void SetClientBeaconActorClass(TSubclassOf<AOnlineBeaconClient> Class);

		// Returns a list of all Clients connected to this Host.
		TArray<AOnlineBeaconBlueprint*> GetClients() const;
	
	private:

		// The Current Online Beacon Blueprint used for Server-side events.
		UPROPERTY()
		UChildActorComponent* Relay;

		// Helper function to get any actor casted to our beacon blueprint.
		static AOnlineBeaconBlueprint* GetBeacon(AActor* Actor);
	
		// Helper function to get the Relay casted to our beacon blueprint.
		AOnlineBeaconBlueprint* GetRelay() const;

		// Called when a client is connected to this dynamic host object.
		virtual void OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) override;

		// Called when a client is disconnected from this dynamic host object.
		virtual void NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) override;
};
