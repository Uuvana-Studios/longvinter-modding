#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistCommands.h"

class FBAGraphHandler;

class BLUEPRINTASSIST_API FBAActionsBase
{
public:
	virtual ~FBAActionsBase() = default;
	virtual void Init() = 0;

	bool CanExecuteActions() const;

	static TSharedPtr<FBAGraphHandler> GetGraphHandler();
};
