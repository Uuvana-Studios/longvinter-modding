// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DiscordSdkEditorStyle.h"

class FDiscordSdkCommands : public TCommands<FDiscordSdkCommands>
{
public:

	FDiscordSdkCommands()
		: TCommands<FDiscordSdkCommands>(TEXT("DiscordSdkEditor"), NSLOCTEXT("Contexts", "DiscordSdkEditor", "DiscordSdkEditor"), NAME_None, FDiscordSdkEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< class FUICommandInfo > PluginAction;
};
