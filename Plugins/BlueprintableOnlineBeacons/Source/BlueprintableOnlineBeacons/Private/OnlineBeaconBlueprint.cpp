// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.


#include "OnlineBeaconBlueprint.h"
#include "BlueprintableOnlineBeacons.h"
#include "EngineMinimal.h"
#include "OnlineBeaconHostObjectDynamic.h"
#include "OnlineSubsystem.h"

// Override Begin Play to bind Timer.
void AOnlineBeaconBlueprint::BeginPlay() {
    // Super.
    Super::BeginPlay();

    // Set Timer Every Tick.
    GetWorldTimerManager().SetTimer(TickTimer, this, &AOnlineBeaconBlueprint::DoLocalTick, 0.1f, true);
}

// Override End Play.
void AOnlineBeaconBlueprint::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Super.
    Super::EndPlay(EndPlayReason);

    // Remove Tick Timer.
    GetWorldTimerManager().ClearTimer(TickTimer);
    TickTimer.Invalidate();
}

// Our Custom Tick.
void AOnlineBeaconBlueprint::DoLocalTick() {
    // Get Current Connection State.
    const EBeaconConnectionStateBlueprint CurState = GetConnectionStateBlueprint();
    
    // Check for Changes.
    if (LastConnectionState != CurState) {
        // Send Event.
        OnConnectionStateChanged(LastConnectionState, CurState);

        // Check for Server Disconnect.
        if (LastConnectionState == EBeaconConnectionStateBlueprint::BCE_Open && (CurState == EBeaconConnectionStateBlueprint::BCE_Closed || CurState == EBeaconConnectionStateBlueprint::BCE_Invalid)) {
            // Call On Disconnected.
            OnDisconnected(EBeaconDisconnectReason::BDR_Server);

            // Destroy this Beacon.
            AOnlineBeaconClient::DestroyBeacon();
        }

        // Set Last Connection State.
        LastConnectionState = CurState;
    }
}

/**
 * Call this function to begin connection to a server, given its Address.
 * Returns if connection was success.
 */
bool AOnlineBeaconBlueprint::ConnectByIP(FString Address) {
    // Create the URL.
    FURL Url(nullptr, *Address, ETravelType::TRAVEL_Absolute);

    // Set the Port to the Default Listen Port.
    GConfig->GetInt(TEXT("/Script/OnlineSubsystemUtils.OnlineBeaconHost"), TEXT("ListenPort"), Url.Port, GEngineIni);

    // Notify.
    UE_LOG(LogBlueprintableOnlineBeacons, Log, TEXT("Beacon %s is attempting to Connect to %s."), *GetName(), *Url.GetHostPortString());

    // If we didn't bind the Connection Failure, bind it.
    if (!HostConnectionFailure.IsBoundToObject(this))
        HostConnectionFailure.BindUObject(this, &AOnlineBeaconBlueprint::OnConnectionFailureBlueprint);

    // Initialize.
    return InitClient(Url);
}

/**
 * Call this function to begin connection to a server, given its Session Result.
 * Returns if connection was success.
 */
bool AOnlineBeaconBlueprint::ConnectBySession(const FBlueprintSessionResult& Session) {
    // Get the Online Subsystem.
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

    // If the Subsystem is Invalid...
    if (!Subsystem) {
        // Notify.
        UE_LOG(LogBlueprintableOnlineBeacons, Error, TEXT("Beacon %s could not connect to session since there are no Online Subsystems currently active."), *GetName());

        // Stop Execution.
        return false;
    }

    // Get the Session Interface.
    IOnlineSessionPtr Sessions = Subsystem->GetSessionInterface();
    
    // If the Interface is Invalid, return false.
    if (!Sessions) {
        // Notify.
        UE_LOG(LogBlueprintableOnlineBeacons, Error, TEXT("Beacon %s could not connect to session since the Sessions Interface was invalid."), *GetName());

        // Stop Execution.
        return false;
    }

    // Create the Address.
    FString Address;

    // Get the Address.
    Sessions->GetResolvedConnectString(Session.OnlineResult, EName::NAME_BeaconPort, Address);

    // Connect By the IP Address.
    return ConnectByIP(Address);
}

// Overrides the default On Connected functionality to call Blueprint Events.
void AOnlineBeaconBlueprint::OnConnected() {
    // Notify.
    UE_LOG(LogBlueprintableOnlineBeacons, Log, TEXT("Beacon %s has connected to Host."), *GetName());

    // Call On Connected, Blueprint Version.
    OnConnectedBlueprint();
}

// Overrides the default Destroy Beacon to call Blueprint Events.
void AOnlineBeaconBlueprint::DestroyBeacon() {
    // Call On Disconnected.
    OnDisconnected(EBeaconDisconnectReason::BDR_Client);

    // Super.
    AOnlineBeaconClient::DestroyBeacon();
}

// Overrides the default Handle Network Failure to call Blueprint Events.
void AOnlineBeaconBlueprint::HandleNetworkFailure(UWorld* World, UNetDriver* NtDrv, ENetworkFailure::Type FailureType, const FString& ErrorString) {
    // Call Parent.
    AOnlineBeaconClient::HandleNetworkFailure(World, NtDrv, FailureType, ErrorString);

    // Call Network Error.
    OnNetworkFailure(FailureType);
}

/**
 * Call this function to disconnect this Beacon from a server.
 * Be warned that calling this WILL destroy your Actor.
 */
void AOnlineBeaconBlueprint::Disconnect() {
    // Notify.
    UE_LOG(LogBlueprintableOnlineBeacons, Log, TEXT("Disconnecting Beacon %s."), *GetName());

    // Call the Destroy Beacon Function.
    DestroyBeacon();
}

/**
 * Gets the current state of the Beacon connection, for easy blueprint use.
 */
EBeaconConnectionStateBlueprint AOnlineBeaconBlueprint::GetConnectionStateBlueprint() const {
    return static_cast<EBeaconConnectionStateBlueprint>(GetConnectionState());
}

/**
 * This function will return a list of all clients connected on this beacon.
 */
TArray<AOnlineBeaconBlueprint*> AOnlineBeaconBlueprint::GetConnectedClients() const {
    // Get Host.
    AOnlineBeaconHostObjectDynamic* Host = Cast<AOnlineBeaconHostObjectDynamic>(GetBeaconOwner());

    // If Host is invalid, return result.
    if (!Host)
        return TArray<AOnlineBeaconBlueprint*>();

    // Return Clients.
    return Host->GetClients();
}

/**
 * This function disconnects a client from this beacon.
 */
void AOnlineBeaconBlueprint::DisconnectClient(AOnlineBeaconBlueprint* Client) const {
    // Get Host.
    AOnlineBeaconHostObject* Host = GetBeaconOwner();

    // If Host if invalid, return.
    if (!Host)
        return;

    // Call Disconnect Client.
    Host->DisconnectClient(Client);
}