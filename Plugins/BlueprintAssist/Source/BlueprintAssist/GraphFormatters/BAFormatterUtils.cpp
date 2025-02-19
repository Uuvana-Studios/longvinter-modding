#include "BAFormatterUtils.h"

#include "BlueprintAssistUtils.h"

bool FBAFormatterUtils::IsSameRow(const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* NodeA, UEdGraphNode* NodeB)
{
	TSet<FPinLink> VisitedLinks;
	TQueue<UEdGraphNode*> PendingNodes;
	PendingNodes.Enqueue(NodeA);
	while (!PendingNodes.IsEmpty())
	{
		UEdGraphNode* Node = nullptr;
		PendingNodes.Dequeue(Node);

		// UE_LOG(LogTemp, Warning, TEXT("Checking node %s"), *FBAUtils::GetNodeName(Node));

		if (Node == NodeB)
		{
			return true;
		}

		for (const FPinLink& PinLink : FBAUtils::GetPinLinks(Node))
		{
			if (VisitedLinks.Contains(PinLink))
			{
				continue;
			}

			VisitedLinks.Add(PinLink);
			VisitedLinks.Add(PinLink.MakeOppositeLink());

			if (!SameRowMapping.Contains(PinLink))
			{
				continue;
			}

			PendingNodes.Enqueue(PinLink.GetNode());
		}
	}

	return false;
}

void FBAFormatterUtils::StraightenRow(TSharedPtr<FBAGraphHandler> GraphHandler, const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* Node)
{
	StraightenRowWithFilter(GraphHandler, SameRowMapping, Node, [](const FPinLink& Link) { return true; });
}

void FBAFormatterUtils::StraightenRowWithFilter(TSharedPtr<FBAGraphHandler> GraphHandler, const TMap<FPinLink, bool>& SameRowMapping, UEdGraphNode* Node, TFunctionRef<bool(const FPinLink&)> Pred)
{
	TQueue<FPinLink> PendingLinks;
	for (const FPinLink& Link : FBAUtils::GetPinLinks(Node))
	{
		PendingLinks.Enqueue(Link);
	}

	TSet<FPinLink> StraightenedLinks;
	while (!PendingLinks.IsEmpty())
	{
		FPinLink Link;
		PendingLinks.Dequeue(Link);

		if (!Pred(Link))
		{
			continue;
		}

		if (StraightenedLinks.Contains(Link))
		{
			continue;
		}

		StraightenedLinks.Add(Link);
		StraightenedLinks.Add(Link.MakeOppositeLink());

		if (SameRowMapping.Contains(Link))
		{
			FBAUtils::StraightenPin(GraphHandler, Link);

			for (const FPinLink& NewLink : FBAUtils::GetPinLinks(Link.GetToNode()))
			{
				PendingLinks.Enqueue(NewLink);
			}
		}
	}
}