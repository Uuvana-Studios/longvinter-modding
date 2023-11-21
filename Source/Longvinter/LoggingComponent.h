// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LoggingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LONGVINTER_API ULoggingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULoggingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "")
	bool SaveToLogFile(FString text, FString fileName);

	UFUNCTION(BlueprintCallable, Category = "")
	bool ClearLogFile(FString fileName);

	UFUNCTION(BlueprintCallable, Category = "")
	void DeleteOldFiles();

	UFUNCTION(BlueprintCallable, Category = "Network|Utilities", meta = (WorldContext = "WorldContextObject"))
		static FString GetPlayerIPAddress(APlayerController* Player);

};
