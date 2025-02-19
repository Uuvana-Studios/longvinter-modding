// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"

class BLUEPRINTASSIST_API FBAGraphCommands final : public TCommands<FBAGraphCommands>
{
public:
	FBAGraphCommands();

	TSharedPtr<FUICommandInfo> GenerateGetter;

	TSharedPtr<FUICommandInfo> GenerateSetter;

	TSharedPtr<FUICommandInfo> GenerateGetterAndSetter;

	TSharedPtr<FUICommandInfo> ConvertGetToSet;
	TSharedPtr<FUICommandInfo> ConvertSetToGet;

	virtual void RegisterCommands() override;
};
