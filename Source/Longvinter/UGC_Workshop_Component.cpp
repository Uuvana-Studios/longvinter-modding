﻿// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "UGC_Workshop_Component.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "IPlatformFilePak.h"
#include "Algo/Sort.h"
#include "Misc/SecureHash.h"

// Sets default values for this component's properties
UUGC_Workshop_Component::UUGC_Workshop_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUGC_Workshop_Component::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UUGC_Workshop_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void UUGC_Workshop_Component::GetInstalledMods(TArray<FString>& Mods, TArray<FString>& SteamMods) {
	IFileManager& FileManager = IFileManager::Get();

	TArray<FString> directories;
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "Mods/");
	FileManager.FindFiles(directories, *(Path + "*"), false, true);

	for (FString directory : directories) {
		TArray<FString> files;
		FileManager.FindFiles(files, *(Path + directory + "/*.workshopID"), true, false);

		for (FString file : files) {
			Mods.AddUnique(FPaths::GetBaseFilename(*(Path + directory + "/" + file)));
			break;
		}
	}

	FString SteamPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() + "SteamMods/") + "*";
	FileManager.FindFiles(SteamMods, *SteamPath, false, true);

	Algo::Sort(Mods);
	Algo::Sort(SteamMods);
	return;
}

void UUGC_Workshop_Component::copyMods(bool CopyRootOnly, bool SkipMount)
{
	FString sourceDirectory1 = FPaths::Combine(FPaths::ProjectDir(), TEXT("SteamMods/"));
	FString destinationDirectory1 = FPaths::Combine(FPaths::ProjectDir(), TEXT("Mods/"));
	FString sourceDirectory2 = FPaths::Combine(FPaths::ProjectDir(), TEXT("Server/Longvinter/SteamMods/"));
	FString destinationDirectory2 = FPaths::Combine(FPaths::ProjectDir(), TEXT("Server/Longvinter/Mods/"));

	// Function to process each directory
	auto ProcessDirectory = [](const FString& srcDir, const FString& dstDir, const bool server, const bool mount)
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		IFileManager& FileManager = IFileManager::Get();
		// Create the destination directory if it doesn't exist
		if (!PlatformFile.DirectoryExists(*dstDir))
		{
			PlatformFile.CreateDirectory(*dstDir);
		}

		// Empty the dst directory to remove old items
		TArray<FString> modDirs;
		FileManager.FindFiles(modDirs, *(dstDir + "*"), false, true);

		for (FString modDir : modDirs) {
			PlatformFile.DeleteDirectoryRecursively(*(dstDir + modDir));
		}

		// Get all subdirectories
		TArray<FString> subdirectories;
		FileManager.FindFiles(subdirectories, *(srcDir + "*"), false, true);

		// Process each subdirectory
		for (FString subdirectory : subdirectories)
		{
			// Get all .uplugin files in this directory
			TArray<FString> pluginFiles;
			FileManager.FindFiles(pluginFiles, *(srcDir + subdirectory + "/*.uplugin"), true, false);

			// Process each .uplugin file
			for (FString pluginFile : pluginFiles)
			{
				// Get the name of the plugin (without extension)
				FString pluginName = FPaths::GetBaseFilename(pluginFile);

				// Construct the destination directory for this plugin
				FString pluginDstDir = FPaths::Combine(dstDir, pluginName);

				// Copy the entire subdirectory to the new location
				PlatformFile.CopyDirectoryTree(*pluginDstDir, *(srcDir + subdirectory), true);

				// test
				FFileHelper::SaveStringToFile(subdirectory, *(pluginDstDir + "/" + subdirectory + ".workshopID"));

				// Copy the WindowsNoEditor folder to WindowsServer and LinuxServer
				FString sourcePaksDir = FPaths::Combine(pluginDstDir, TEXT("Content/Paks/WindowsNoEditor"));
				FString destinationPaksDirWindowsServer = FPaths::Combine(pluginDstDir, TEXT("Content/Paks/WindowsServer"));
				FString destinationPaksDirLinuxServer = FPaths::Combine(pluginDstDir, TEXT("Content/Paks/LinuxServer"));

				PlatformFile.CopyDirectoryTree(*destinationPaksDirWindowsServer, *sourcePaksDir, true);
				PlatformFile.CopyDirectoryTree(*destinationPaksDirLinuxServer, *sourcePaksDir, true);

				if (mount) {
					FString PakDirectory = server ? "WindowsServer" : "WindowsNoEditor";
					FString modPakFilePath = pluginDstDir + "/Content/Paks/" + PakDirectory;

					TArray<FString> pakFiles;
					FileManager.FindFiles(pakFiles, *(modPakFilePath + "/*.pak"), true, false);

					for (FString pakFile : pakFiles) {
						// Make sure it's a pak platform file.
						if (FPakPlatformFile* PakPlatformFile = static_cast<FPakPlatformFile*>(&PlatformFile)) {
							// Mount the pak file.
							PakPlatformFile->Mount(*(modPakFilePath + "/" + pakFile), 100, nullptr);
						}

					}
				}
			}
		}
	};

	// Process both directories
	ProcessDirectory(sourceDirectory1, destinationDirectory1, false, !SkipMount);
	if (!CopyRootOnly) {
		ProcessDirectory(sourceDirectory2, destinationDirectory2, true, !SkipMount);
	}
}

bool UUGC_Workshop_Component::CheckModFolderSync(bool Server, FString& hash1_s, FString& hash2_s) {
	IFileManager& FileManager = IFileManager::Get();

	FString modsFolder = Server ? (FPaths::ProjectDir() + "Server/Longvinter/Mods/") : (FPaths::ProjectDir()+ "Mods/");
	FString steamModsFolder = Server ? (FPaths::ProjectDir() + "Server/Longvinter/SteamMods/") : (FPaths::ProjectDir() + "SteamMods/");

	TArray<FString> mods;
	FileManager.FindFiles(mods, *(modsFolder + "*"), false, true);

	for (const FString& mod : mods) {
		TArray<FString> modIDs;
		FileManager.FindFiles(modIDs, *(modsFolder + mod + "/*.workshopID"), true, false);

		FString tmpStr, modID, fileExt;
		if (modIDs.Num() > 0) {
			tmpStr = modIDs[0];
			tmpStr.Split(".", &modID, &fileExt);
		} else {
			continue;
		}

		FString PakDir = "/Content/Paks/WindowsNoEditor";

		TArray<FString> pakFiles;
		FileManager.FindFiles(pakFiles, *(modsFolder + mod + PakDir + "/*.pak"), true, false);
		TArray<FString> steamPakFiles;
		FileManager.FindFiles(steamPakFiles, *(steamModsFolder + modID + PakDir + "/*.pak"), true, false);

		if (steamPakFiles.Num() > 0 && pakFiles.Num() > 0) {
			FMD5Hash hash1 = FMD5Hash::HashFile(*(modsFolder + mod + PakDir + "/" + steamPakFiles[0]));
			FMD5Hash hash2 = FMD5Hash::HashFile(*(steamModsFolder + modID + PakDir + "/" + pakFiles[0]));

			if (LexToString(hash1) != LexToString(hash2)) {
				hash1_s = LexToString(hash1);
				hash2_s = LexToString(hash2);
				return false;
			}
		} else {
			hash1_s = steamPakFiles.Num() == 0 ? "failed to find steam pak file" : "found steam pak file";
			hash2_s = pakFiles.Num() == 0 ? "failed to find mod pak file" : "found mod pak file";
			return false;
		}
	}
	return true;
}