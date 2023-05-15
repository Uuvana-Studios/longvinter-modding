// Copyright Pandores Marketplace 2021. All Rights Reserved.

#pragma once

#include "Templates/SharedPointer.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Modules/ModuleManager.h"

DECLARE_DELEGATE(FDiscordAchievementAction)

#define UE_HAS_TOOLMENUS ( ( ENGINE_MINOR_VERSION >= 24 || ENGINE_MAJOR_VERSION > 4 ) && 1 )

class FDiscordSdkEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void RegisterMenus();

	// Called when we click on the action in the dropdown
	FDiscordAchievementAction& GetOnDiscordAchievementAction()
	{
		return OnDiscordAchievementAction;
	}

private:
	TSharedRef<class SWidget> GenerateButtonCombo();

	FDiscordAchievementAction OnDiscordAchievementAction;

#if !UE_HAS_TOOLMENUS
	void AddToolbarButton(FToolBarBuilder& Builder);
#endif // !UE_HAS_TOOLMENUS
};

