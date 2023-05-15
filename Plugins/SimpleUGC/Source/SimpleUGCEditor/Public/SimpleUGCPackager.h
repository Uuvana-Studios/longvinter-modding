// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FSimpleUGCCommand
{
	TSharedPtr<class IPlugin> PluginInfo;
	TSharedPtr<class FUICommandInfo> CommandInfo;
};

class FSimpleUGCPackager : public TSharedFromThis<FSimpleUGCPackager>
{
public:
	FSimpleUGCPackager();
	~FSimpleUGCPackager();

	void OpenPluginPackager(TSharedRef<class IPlugin> Plugin);

	void PackagePlugin(TSharedRef<class IPlugin> Plugin, const FString& OutputDirectory);

	/** Generates submenu content for the plugin packager command */
	void GeneratePackagerMenuContent(class FMenuBuilder& MenuBuilder);

	/** Generates the menu content for the plugin packager toolbar button */
	TSharedRef<class SWidget> GeneratePackagerComboButtonContent();

private:
	/** Gets all available game mod plugin packages  */
	void FindAvailableGameMods(TArray<TSharedRef<class IPlugin>>& OutAvailableGameMods);

	/** Gets all available game mod plugins and registers command info for them */
	void GetAvailableUGCCommands(const TArray<TSharedRef<class IPlugin>>& AvailableUGC);

	/** Generates menu content for the supplied set of commands */
	void GeneratePackagerMenuContent_Internal(class FMenuBuilder& MenuBuilder, const TArray<TSharedPtr<FUICommandInfo>>& Commands);

	/**
	* Checks if a plugin has any unsaved content
	*
	* @param	Plugin			The plugin to check for unsaved content
	* @return	True if all mod content has been saved, false otherwise
	*/
	bool IsAllContentSaved(TSharedRef<class IPlugin> Plugin);

private:
	TArray<TSharedPtr<class FUICommandInfo>> UGCCommands;
};
