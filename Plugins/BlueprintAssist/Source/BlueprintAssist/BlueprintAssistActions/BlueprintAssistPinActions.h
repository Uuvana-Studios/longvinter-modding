#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistNodeActions.h"

class FUICommandList;

class BLUEPRINTASSIST_API FBAPinActionsBase : public FBANodeActionsBase
{
public:
	bool HasSelectedPin() const;
	bool HasEditablePin() const;
};

class BLUEPRINTASSIST_API FBAPinActions final : public FBAPinActionsBase
{
public:
	virtual void Init() override;

	// Graph commands
	TSharedPtr<FUICommandList> PinCommands;
	TSharedPtr<FUICommandList> PinEditCommands;

	void LinkToHoveredPin();
	void OpenPinLinkMenu();
	void DuplicateNodeForEachLink();
	void OnEditSelectedPinValue();
};
