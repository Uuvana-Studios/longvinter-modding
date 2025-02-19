#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistActionsBase.h"

class FUICommandList;

class BLUEPRINTASSIST_API FBAGlobalActionsBase : public FBAActionsBase
{
public:
	bool CanOpenEditDetailsMenu() const;
	bool HasWorkflowModes() const;
};

class BLUEPRINTASSIST_API FBAGlobalActions final : public FBAGlobalActionsBase
{
public:
	TSharedPtr<FUICommandList> GlobalCommands;

	virtual void Init() override;

	void OpenBlueprintAssistDebugMenu() const;
	void OpenFocusSearchBoxMenu();
	void OpenEditDetailsMenu();
	void OpenWindowMenu();
	void OpenBlueprintAssistHotkeyMenu();
	void OpenTabSwitcherMenu();
	void ToggleFullscreen();
	void SwitchWorkflowMode();
	void OpenAssetCreationMenu();
	void FocusSearchBox();

	bool CanFocusSearchBox() const;

	TSharedPtr<SWidget> FindSearchBox() const;
};
