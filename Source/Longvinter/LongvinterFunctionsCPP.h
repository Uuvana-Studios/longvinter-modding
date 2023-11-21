// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/DateTime.h"
#include "LongvinterFunctionsCPP.generated.h"

/**
 * 
 */
UCLASS()
class LONGVINTER_API ULongvinterFunctionsCPP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
	static int64 ToUnixTimestampBPC(FDateTime DateTime);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
	static FDateTime FromUnixTimestampBPC(int64 Timestamp);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
	static FString DateTimeToString(FDateTime DateTime);
};
