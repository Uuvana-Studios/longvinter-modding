// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.


#include "OnlineBeaconHostObjectDynamic.h"
#include "BlueprintableOnlineBeacons.h"

// Constructor.
AOnlineBeaconHostObjectDynamic::AOnlineBeaconHostObjectDynamic() {
	// Create Root Component.
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	// Create Child Actor Component.
	Relay = CreateDefaultSubobject<UChildActorComponent>("Relay");

	// Setup Attachment to Root.
	Relay->SetupAttachment(RootComponent);
}

// Sets a new Beacon Class for this Host Object.
void AOnlineBeaconHostObjectDynamic::SetClientBeaconActorClass(TSubclassOf<AOnlineBeaconClient> Class) {
	// Prevent Set Class as the same class and invalid class.
	if (Class == ClientBeaconActorClass || Class == nullptr)
		return;
	
	// Set the Beacon Actor Class.
	ClientBeaconActorClass = Class;

	// Set the Name.
	BeaconTypeName = Class->GetName();

	// Set Relay Class.
	Relay->SetChildActorClass(ClientBeaconActorClass);

	// Set Beacon Owner.
	GetRelay()->SetBeaconOwner(this);
}

// Returns a list of all Clients connected to this Host.
TArray<AOnlineBeaconBlueprint*> AOnlineBeaconHostObjectDynamic::GetClients() const {
	// Create Result.
	TArray<AOnlineBeaconBlueprint*> Result;

	// Loop all Clients, add to result.
	for (AOnlineBeaconClient* Client : ClientActors)
		Result.Add(GetBeacon(Client));
	
	// Return Result.
	return Result;
}

// Helper function to get any actor casted to our beacon blueprint.
AOnlineBeaconBlueprint* AOnlineBeaconHostObjectDynamic::GetBeacon(AActor* Actor) {
	// Check for Actor.
	if (!Actor)
		return nullptr;

	// Return it casted.
	return Cast<AOnlineBeaconBlueprint>(Actor);
}

// Helper function to get the Relay casted to our beacon client.
AOnlineBeaconBlueprint* AOnlineBeaconHostObjectDynamic::GetRelay() const {
	// Return it casted.
	return GetBeacon(Relay->GetChildActor());
}

// Called when a client is connected to this dynamic host object.
void AOnlineBeaconHostObjectDynamic::OnClientConnected(AOnlineBeaconClient* NewClientActor, UNetConnection* ClientConnection) {
	// Call parent function.
	AOnlineBeaconHostObject::OnClientConnected(NewClientActor, ClientConnection);
	
	// Notify.
	UE_LOG(LogBlueprintableOnlineBeacons, Log, TEXT("Client connected to Beacon %s."), *GetName());

	// Get Relay.
	AOnlineBeaconBlueprint* RelayObj = GetRelay();
	
	// Prevent Invalid Relay.
	if (!RelayObj)
		return;
	
	// Call Relay function.
	RelayObj->OnClientConnected(GetBeacon(NewClientActor));
}

// Called when a client is disconnected from this dynamic host object.
void AOnlineBeaconHostObjectDynamic::NotifyClientDisconnected(AOnlineBeaconClient* LeavingClientActor) {
	// Call parent function.
	AOnlineBeaconHostObject::NotifyClientDisconnected(LeavingClientActor);
	
	// Notify.
	UE_LOG(LogBlueprintableOnlineBeacons, Log, TEXT("Client disconnected from Beacon %s."), *GetName());

	// Get Relay.
	AOnlineBeaconBlueprint* RelayObj = GetRelay();
	
	// Prevent Invalid Relay.
	if (!RelayObj)
		return;
	
	// Call Relay function.
	RelayObj->OnClientDisconnected(GetBeacon(LeavingClientActor));
}