// © 2021 Uuvana Studios Oy. All Rights Reserved.


#include "LoggingComponent.h"
#include "Misc/FileHelper.h"

// Sets default values for this component's properties
ULoggingComponent::ULoggingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULoggingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void ULoggingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool ULoggingComponent::SaveToLogFile(FString text, FString fileName) {
	text = "\r\n" + text;
	return FFileHelper::SaveStringToFile(text, *(FPaths::ProjectLogDir() + fileName), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

bool ULoggingComponent::ClearLogFile(FString fileName) {
	return FFileHelper::SaveStringToFile(TEXT(""), *(FPaths::ProjectLogDir() + fileName));
}

void ULoggingComponent::DeleteOldFiles() {
	FJsonSerializableArray filenames;
	IFileManager::Get().FindFiles(filenames, *(FPaths::ProjectLogDir()));
	FString filenamePrefix = "AdminPanelServer-";
	FTimespan week = FTimespan(7, 0, 0, 0);

	for (int i = 0; i < filenames.Num(); i++) {
		if (filenames[i].StartsWith(filenamePrefix)) {
			FString fileDateString = filenames[i].RightChop(filenamePrefix.Len()).LeftChop(4);
			FJsonSerializableArray dateArray;
			fileDateString.ParseIntoArray(dateArray, TEXT("."));
			FDateTime fileDate = FDateTime(FCString::Atoi(*(dateArray[2])), FCString::Atoi(*(dateArray[1])), FCString::Atoi(*(dateArray[0])));
			
			if (fileDate <= (FDateTime::Today() - week)) {
				IFileManager& FileManager = IFileManager::Get();
				FileManager.Delete(*(FPaths::ProjectLogDir() + filenames[i]));
			}
		}
	}
}