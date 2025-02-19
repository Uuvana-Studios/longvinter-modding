#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistActionsBase.h"

class FUICommandList;

class BLUEPRINTASSIST_API FBABlueprintActionsBase : public FBAActionsBase
{
public:
	bool HasOpenBlueprintEditor() const;
};

class BLUEPRINTASSIST_API FBABlueprintActions final : public FBABlueprintActionsBase
{
public:
	virtual void Init() override;

	TSharedPtr<FUICommandList> BlueprintCommands;

	void OpenVariableSelectorMenu();

	void OpenCreateSymbolMenu();

	void OpenGoToSymbolMenu();
};
