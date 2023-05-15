// Copyright Sam Bonifacio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AutoSettingsInputConfig.h"
#include "AutoSettingsInputProjectConfig.generated.h"

/**
 * Config Subclass used for the page in project settings
 * This is separate to the base class, so that project settings do not affect the default values of the base class objects (which makes them easier to test)
 */
UCLASS()
class AUTOSETTINGSINPUT_API UAutoSettingsInputProjectConfig : public UAutoSettingsInputConfig
{
	GENERATED_BODY()
	
public:

	UAutoSettingsInputProjectConfig();

	// Returns the key group that contains the given key
	UFUNCTION(BlueprintPure, Category = "Auto Settings Config", DisplayName = "Get Key Friendly Name")
    static FText GetKeyFriendlyNameStatic(FKey Key);

	// Returns the key group that contains the given key
	UFUNCTION(BlueprintPure, Category = "Auto Settings Config", DisplayName = "Get Key Group")
    static FGameplayTag GetKeyGroupStatic(FKey Key);

	/**
	 * Synchronously load all key icon textures in any KeyIconSet(s) with all of the given tags
	 * Returns all textures that were loaded, which should be stored as a variable for as long as the textures need to remain loaded
	 */
	UFUNCTION(BlueprintCallable, Category = "Auto Settings Config")
	static TArray<UTexture*> LoadKeyIcons(FGameplayTagContainer KeyIconTags);

#if WITH_EDITOR
	virtual bool SupportsAutoRegistration() const override { return true; }
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
#endif

	virtual void PostInitProperties() override;
};
