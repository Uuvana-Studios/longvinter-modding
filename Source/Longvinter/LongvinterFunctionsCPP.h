// © 2021 Uuvana Studios Oy. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/DateTime.h"
#include "Misc/DefaultValueHelper.h"
#include "LongvinterFunctionsCPP.generated.h"

USTRUCT(BlueprintType)
struct LONGVINTER_API FmySunPositionData
{
    GENERATED_BODY()

public:
    /** Sun Elevation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    float Elevation;

    /** Sun Elevation, corrected for atmospheric diffraction */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    float CorrectedElevation;

    /** Sun azimuth */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    float Azimuth;

    /** Sunrise time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    FTimespan SunriseTime;

    /** Sunset time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    FTimespan SunsetTime;

    /** Solar noon */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Position")
    FTimespan SolarNoon;
}; // Add the missing semicolon here

/**
 * 
 */
UCLASS()
class LONGVINTER_API ULongvinterFunctionsCPP : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    ULongvinterFunctionsCPP();

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
    static int64 ToUnixTimestampBPC(FDateTime DateTime);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
    static FDateTime FromUnixTimestampBPC(int64 Timestamp);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
    static FString DateTimeToString(FDateTime DateTime);

    UFUNCTION(BlueprintPure, meta = (CompactNodeTitle = "->", DisplayName = "To String (int64)", BlueprintAutocast), Category = "Discord")
    static FString ToString(int64 Value);

    /** Get the sun's position data based on position, date and time */
    UFUNCTION(BlueprintCallable, Category = "Sun Position")
    static void GetMySunPosition(float Latitude, float Longitude, float TimeZone, bool bIsDaylightSavingTime, int32 Year, int32 Month, int32 Day, int32 Hours, int32 Minutes, int32 Seconds, FmySunPositionData& SunPositionData);

    /** Calculate the FOV based on the aspect ratio */
    UFUNCTION(BlueprintCallable, Category = "Camera")
    static float CalculateFOV(float AspectRatio);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Custom")
    static void GetKeysForAction(APlayerController* PlayerController, FName ActionName, FKey& OutKeyboardKey, FKey& OutGamepadKey);

    UFUNCTION(BlueprintCallable, Category = "Custom")
    static void BackupSaveGames(bool DailyBackup);

    UFUNCTION(BlueprintCallable, Category = "Custom")
    static int64 ParseStringToInt64C(const FString& InputString);

    UFUNCTION(BlueprintCallable, Category = "Network")
    static FString GetLocalIPAddress();
};