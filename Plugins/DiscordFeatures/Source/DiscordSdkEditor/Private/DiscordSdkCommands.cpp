// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiscordSdkCommands.h"
#include "Framework/Commands/UICommandInfo.h"

#define LOCTEXT_NAMESPACE "FDiscordSdkEditorModule"

void FDiscordSdkCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Discord", "Configure the Discord SDK", EUserInterfaceActionType::ToggleButton, FInputGesture());
	
}

#undef LOCTEXT_NAMESPACE
