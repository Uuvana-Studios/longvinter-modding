// Copyright (C) 2020 Alan Renato Bunese - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// Declare Blueprintable Online Beacons Log.
DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintableOnlineBeacons, Log, All);

/**
 * Used by the engine to register modules.
 */
class FBlueprintableOnlineBeaconsModule : public IModuleInterface {

	public:

		/** IModuleInterface implementation */

		// Called by the Engine when the Module Starts up.
		virtual void StartupModule() override;

		// Called by the Engine when the Module Shutsdown.
		virtual void ShutdownModule() override;

		// If this module support dynamic reloading. (For Easy Code Compilation and Reloading)
		virtual bool SupportsDynamicReloading() override;

		// If this module is used as a game module.
		virtual bool IsGameModule() const override;

		/** IModuleInterface implementation */
};
