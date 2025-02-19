// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FExtender;
class FUICommandList;
class UEdGraph;
class UEdGraphNode;
class UEdGraphPin;
class UK2Node_VariableSet;
class UK2Node_VariableGet;
class UK2Node_Variable;

class BLUEPRINTASSIST_API FBAGraphExtender final
{
public:
	static void ApplyExtender();

	static TSharedRef<FExtender> ExtendSelectedNode(const TSharedRef<FUICommandList> CommandList, const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bIsEditable);

	static TSharedRef<FExtender> ExtendPin(const TSharedRef<FUICommandList> CommandList, const UEdGraph* Graph, const UEdGraphNode* Node, const UEdGraphPin* Pin, bool bIsEditable);

	static bool GenerateGetter(const UEdGraph* Graph, const UEdGraphNode* Node);

	static bool GenerateSetter(const UEdGraph* Graph, const UEdGraphNode* Node);

	static void GenerateGetterAndSetter(const UEdGraph* Graph, const UEdGraphNode* Node);

	static void ConvertGetToSet(const UEdGraph* Graph, UK2Node_VariableGet* VariableGetNode);

	static void ConvertSetToGet(const UEdGraph* Graph, UK2Node_VariableSet* VariableSetNode);

	static void GoToDefinition(const UEdGraphPin* Pin);

	static UK2Node_VariableSet* CreateVariableSetFromVariable(FVector2D NodePos, UEdGraph* Graph, const UK2Node_Variable* Variable);

	static UK2Node_VariableGet* CreateVariableGetFromVariable(FVector2D NodePos, UEdGraph* Graph, const UK2Node_Variable* Variable);
};
