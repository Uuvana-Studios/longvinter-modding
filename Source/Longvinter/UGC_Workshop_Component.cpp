// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "UGC_Workshop_Component.h"

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


TArray<FString> UUGC_Workshop_Component::GetInstalledMods() {
	TArray<FString> installedFiles;
	IFileManager& FileManager = IFileManager::Get();
	FString Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectModsDir()) + "*";
	FileManager.FindFiles(installedFiles, *Path, false, true);
	return installedFiles;
}