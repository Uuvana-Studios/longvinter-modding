#pragma once

#include "GraphFormatterTypes.h"

struct FBAFormatterUtils
{
	static bool IsSameRow(const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* NodeA, UEdGraphNode* NodeB);
	static void StraightenRow(TSharedPtr<FBAGraphHandler> GraphHandler, const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* Node);
	static void StraightenRowWithFilter(TSharedPtr<FBAGraphHandler> GraphHandler, const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* Node, TFunctionRef<bool(const FPinLink&)> Pred);
};
