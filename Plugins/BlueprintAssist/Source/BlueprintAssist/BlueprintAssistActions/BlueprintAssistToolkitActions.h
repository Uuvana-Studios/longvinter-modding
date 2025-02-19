#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistActionsBase.h"

class UEdGraphPin;
class FUICommandList;

class BLUEPRINTASSIST_API FBAToolkitActionsBase : public FBAActionsBase
{
public:
	bool HasToolkit() const;
};

class BLUEPRINTASSIST_API FBAToolkitActions final : public FBAToolkitActionsBase
{
public:
	virtual void Init() override;

	// Graph commands
	TSharedPtr<FUICommandList> ToolkitCommands;

	void GoToParentClassDefinition() const;
};
