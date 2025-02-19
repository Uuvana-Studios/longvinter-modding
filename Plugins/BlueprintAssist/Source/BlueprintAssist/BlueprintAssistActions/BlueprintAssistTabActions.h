#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistActionsBase.h"

class UEdGraphNode;
class FUICommandList;

class BLUEPRINTASSIST_API FBATabActionsBase : public FBAActionsBase
{
public:
	bool HasOpenTab() const;
	bool HasOpenActionMenu() const;
};

class BLUEPRINTASSIST_API FBATabActions final : public FBATabActionsBase
{
public:
	virtual void Init() override;

	TSharedPtr<FUICommandList> TabCommands;

	void SelectNodeInDirection(const TArray<UEdGraphNode*>& Nodes, int X, int Y, float DistLimit) const;

	void SelectAnyNodeInDirection(const int X, const int Y) const;

	void ShiftCameraInDirection(int X, int Y) const;

	TSharedPtr<FUICommandList> ActionMenuCommands;

	void OnToggleActionMenuContextSensitive() const;
};
