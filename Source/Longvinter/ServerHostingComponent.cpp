// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "ServerHostingComponent.h"
#include <Runtime/Json/Public/Serialization/JsonSerializerMacros.h>
#include <Runtime/Core/Public/Misc/FileHelper.h>

// Sets default values for this component's properties
UServerHostingComponent::UServerHostingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UServerHostingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UServerHostingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Set variables to the server's Game.ini
void SetServerName(FString ServerName, FJsonSerializableArray& FileArray) {
	FString variableName = "ServerName";
	FString newConfigLine = variableName + "=" + ServerName;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetMaxPlayers(FString MaxPlayers, FJsonSerializableArray& FileArray) {
	FString variableName = "MaxPlayers";
	FString newConfigLine = variableName + "=" + MaxPlayers;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetPassword(FString Password, FJsonSerializableArray& FileArray) {
	FString variableName = "Password";
	FString newConfigLine = variableName + "=" + Password;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetServerTag(FString ServerTag, FJsonSerializableArray& FileArray) {
	FString variableName = "ServerTag";
	FString newConfigLine = variableName + "=" + ServerTag;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetCommunity(FString Community, FJsonSerializableArray& FileArray) {
	FString variableName = "CommunityWebsite";
	FString newConfigLine = variableName + "=" + Community;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetAdmins(FString Admins, FJsonSerializableArray& FileArray) {
	FString variableName = "AdminSteamID";
	FString newConfigLine = variableName + "=" + Admins;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetPVP(FString PVP, FJsonSerializableArray& FileArray) {
	FString variableName = "PVP";
	FString newConfigLine = variableName + "=" + PVP;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetDecay(FString Decay, FJsonSerializableArray& FileArray) {
	FString variableName = "TentDecay";
	FString newConfigLine = variableName + "=" + Decay;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetMaxTents(FString MaxTents, FJsonSerializableArray& FileArray) {
	FString variableName = "MaxTents";
	FString newConfigLine = variableName + "=" + MaxTents;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetPublic(FString Public, FJsonSerializableArray& FileArray) {
	FString variableName = "Public";
	FString newConfigLine = variableName + "=" + Public;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetCoopPlay(FString CoopPlay, FJsonSerializableArray& FileArray) {
	FString variableName = "CoopPlay";
	FString newConfigLine = variableName + "=" + CoopPlay;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetCoopSpawn(FString CoopSpawn, FJsonSerializableArray& FileArray) {
	FString variableName = "CoopSpawn";
	FString newConfigLine = variableName + "=" + CoopSpawn;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetDefaultPrestigeLevel(FString DefaultPrestige, FJsonSerializableArray& FileArray) {
	FString variableName = "DefaultPrestigeLevel";
	FString newConfigLine = variableName + "=" + DefaultPrestige;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

void SetDisableWanderingTraders(FString DisableWanderingTraders, FJsonSerializableArray& FileArray) {
	FString variableName = "DisableWanderingTraders";
	FString newConfigLine = variableName + "=" + DisableWanderingTraders;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			FileArray[i].Empty();
			FileArray[i].Append(newConfigLine);
		}
	}
}

// Get variables from the server's Game.ini
void GetServerName(FString& ServerName, FJsonSerializableArray& FileArray) {
	FString variableName = "ServerName";
	FString newConfigLine = variableName + "=" + ServerName;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			ServerName = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetMaxPlayers(FString& MaxPlayers, FJsonSerializableArray& FileArray) {
	FString variableName = "MaxPlayers";
	FString newConfigLine = variableName + "=" + MaxPlayers;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			MaxPlayers = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetPassword(FString& Password, FJsonSerializableArray& FileArray) {
	FString variableName = "Password";
	FString newConfigLine = variableName + "=" + Password;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			Password = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetServerTag(FString& ServerTag, FJsonSerializableArray& FileArray) {
	FString variableName = "ServerTag";
	FString newConfigLine = variableName + "=" + ServerTag;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			ServerTag = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetCommunity(FString& Community, FJsonSerializableArray& FileArray) {
	FString variableName = "CommunityWebsite";
	FString newConfigLine = variableName + "=" + Community;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			Community = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetAdmins(FString& Admins, FJsonSerializableArray& FileArray) {
	FString variableName = "AdminSteamID";
	FString newConfigLine = variableName + "=" + Admins;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			Admins = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetPVP(FString& PVP, FJsonSerializableArray& FileArray) {
	FString variableName = "PVP";
	FString newConfigLine = variableName + "=" + PVP;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			PVP = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetDecay(FString& Decay, FJsonSerializableArray& FileArray) {
	FString variableName = "TentDecay";
	FString newConfigLine = variableName + "=" + Decay;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			Decay = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetMaxTents(FString& MaxTents, FJsonSerializableArray& FileArray) {
	FString variableName = "MaxTents";
	FString newConfigLine = variableName + "=" + MaxTents;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			MaxTents = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetPublic(FString& Public, FJsonSerializableArray& FileArray) {
	FString variableName = "Public";
	FString newConfigLine = variableName + "=" + Public;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			Public = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetCoopPlay(FString& CoopPlay, FJsonSerializableArray& FileArray) {
	FString variableName = "CoopPlay";
	FString newConfigLine = variableName + "=" + CoopPlay;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			CoopPlay = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetCoopSpawn(FString& CoopSpawn, FJsonSerializableArray& FileArray) {
	FString variableName = "CoopSpawn";
	FString newConfigLine = variableName + "=" + CoopSpawn;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			CoopSpawn = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetDefaultPrestige(FString& DefaultPrestige, FJsonSerializableArray& FileArray) {
	FString variableName = "DefaultPrestigeLevel";
	FString newConfigLine = variableName + "=" + DefaultPrestige;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			DefaultPrestige = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void GetDisableWanderingTraders(FString& DisableWanderingTraders, FJsonSerializableArray& FileArray) {
	FString variableName = "DisableWanderingTraders";
	FString newConfigLine = variableName + "=" + DisableWanderingTraders;

	for (int i = 0; i < FileArray.Num(); i++) {
		if (FileArray[i].StartsWith(variableName)) {
			DisableWanderingTraders = FileArray[i].RightChop(variableName.Len() + 1);
		}
	}
}

void UServerHostingComponent::StartServer() {
	FString path = FPaths::ProjectDir() + "Server/Longvinter/Binaries/Win64/LongvinterServer-Win64-Shipping.exe";
	ServerProcessHandle = FPlatformProcess::CreateProc(*path, TEXT(""), true, false, false, nullptr, 0, nullptr, nullptr);
	LocalServerStarted = true;
}

void UServerHostingComponent::StopServer() {
	if (ServerProcessHandle.IsValid()) {
		FPlatformProcess::TerminateProc(ServerProcessHandle);
		FPlatformProcess::CloseProc(ServerProcessHandle);

		UE_LOG(LogTemp, Log, TEXT("Server stopped successfully."));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No valid server process to stop."));
	}
}

void UServerHostingComponent::WipeServer() {
	IFileManager& FileManager = IFileManager::Get();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString path = FPaths::ProjectDir() + "Server/Longvinter/Saved/SaveGames/";

	if (PlatformFile.DirectoryExists(*path)) {
		FileManager.DeleteDirectory(*path, true, true);
	}
}

void UServerHostingComponent::SaveServerVariables(FString ServerName, FString MaxPlayers, FString Password, FString ServerTag, FString Community, FString Admins, FString PVP, FString Decay, FString MaxTents, FString Public, FString CoopPlay, FString CoopSpawn, FString DefaultPrestige, FString DisableWanderingTraders) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString path = FPaths::ProjectDir() + "Server/Longvinter/Saved/Config/WindowsServer/Game.ini";
	FString defaultPath = FPaths::ProjectDir() + "Server/Longvinter/Saved/Config/WindowsServer/Game.ini.default";
	if (!PlatformFile.FileExists(*path)) {
		PlatformFile.CopyFile(*path, *defaultPath);
	}

	FJsonSerializableArray FileArray;

	FFileHelper::LoadFileToStringArray(FileArray, *path);

	if (FileArray.Num() < 1) {
		PlatformFile.CopyFile(*path, *defaultPath);
	}

	SetServerName(ServerName, FileArray);
	SetMaxPlayers(MaxPlayers, FileArray);
	SetPassword(Password, FileArray);
	SetServerTag(ServerTag, FileArray);
	SetCommunity(Community, FileArray);
	SetAdmins(Admins, FileArray);
	SetPVP(PVP, FileArray);
	SetDecay(Decay, FileArray);
	SetMaxTents(MaxTents, FileArray);
	SetPublic(Public, FileArray);
	SetCoopPlay(CoopPlay, FileArray);
	SetCoopSpawn(CoopSpawn, FileArray);
	SetDefaultPrestigeLevel(DefaultPrestige, FileArray);
	SetDisableWanderingTraders(DisableWanderingTraders, FileArray);

	FFileHelper::SaveStringArrayToFile(FileArray, *path);
}

void UServerHostingComponent::LoadServerVariables(FString& ServerName, FString& MaxPlayers, FString& Password, FString& ServerTag, FString& Community, FString& Admins, FString& PVP, FString& Decay, FString& MaxTents, FString& Public, FString& CoopPlay, FString& CoopSpawn, FString& DefaultPrestige, FString& DisableWanderingTraders) {
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString path = FPaths::ProjectDir() + "Server/Longvinter/Saved/Config/WindowsServer/Game.ini";
	FString defaultPath = FPaths::ProjectDir() + "Server/Longvinter/Saved/Config/WindowsServer/Game.ini.default";
	if (!PlatformFile.FileExists(*path)) {
		PlatformFile.CopyFile(*path, *defaultPath);
	}

	FJsonSerializableArray FileArray;

	FFileHelper::LoadFileToStringArray(FileArray, *path);

	if (FileArray.Num() < 1) {
		PlatformFile.CopyFile(*path, *defaultPath);
	}

	GetServerName(ServerName, FileArray);
	GetMaxPlayers(MaxPlayers, FileArray);
	GetPassword(Password, FileArray);
	GetServerTag(ServerTag, FileArray);
	GetCommunity(Community, FileArray);
	GetAdmins(Admins, FileArray);
	GetPVP(PVP, FileArray);
	GetDecay(Decay, FileArray);
	GetMaxTents(MaxTents, FileArray);
	GetPublic(Public, FileArray);
	GetCoopPlay(CoopPlay, FileArray);
	GetCoopSpawn(CoopSpawn, FileArray);
	GetDefaultPrestige(DefaultPrestige, FileArray);
	GetDisableWanderingTraders(DisableWanderingTraders, FileArray);
}