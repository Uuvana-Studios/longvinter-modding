// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistUtils.h"
#include "SGraphPin.h"

#if ENGINE_MINOR_VERSION >= 25 || ENGINE_MAJOR_VERSION >= 5
#define BA_PROPERTY FProperty
#define BA_FIND_FIELD FindUField
#define BA_FIND_PROPERTY FindFProperty
#define BA_WEAK_FIELD_PTR TWeakFieldPtr
#else
#define BA_PROPERTY UProperty
#define BA_FIND_FIELD FindField
#define BA_FIND_PROPERTY FindField
#define BA_WEAK_FIELD_PTR TWeakObjectPtr
#endif

struct FBAGraphPinHandle
{
	TWeakObjectPtr<UEdGraph> Graph = nullptr;
	FGuid NodeGuid;
	FGuid PinId;

	// for when guid fails
	FEdGraphPinType PinType;
	FName PinName;

	FBAGraphPinHandle()
	{
		Invalidate();
	}

	FBAGraphPinHandle(UEdGraphPin* Pin)
	{
		SetPin(Pin);
	}

	void SetPin(UEdGraphPin* Pin);

	UEdGraphPin* GetPin();

	bool IsValid() const
	{
		return Graph != nullptr && PinId.IsValid() && NodeGuid.IsValid();
	}

	void Invalidate()
	{
		Graph = nullptr;
		NodeGuid.Invalidate();
		PinId.Invalidate();
		PinType.ResetToDefaults();
		PinName = NAME_None;
	}

	bool operator==(const FBAGraphPinHandle& Other) const
	{
		return PinId == Other.PinId && NodeGuid == Other.NodeGuid;
	}

	bool operator!=(const FBAGraphPinHandle& Other) const
	{
		return !(*this == Other);
	}

	friend inline uint32 GetTypeHash(const FBAGraphPinHandle& Handle)
	{
		return HashCombine(GetTypeHash(Handle.PinId), GetTypeHash(Handle.NodeGuid));
	}
};

// Consider using FEdGraphPinReference
struct FBANodePinHandle
{
	TWeakObjectPtr<UEdGraphNode> Node = nullptr;
	FGuid PinId;

	// for when guid fails
	FEdGraphPinType PinType;
	FName PinName;

	FBANodePinHandle()
	{
		SetPin(nullptr);
	}

	FBANodePinHandle(UEdGraphPin* Pin)
	{
		SetPin(Pin);
	}

	void SetPin(UEdGraphPin* Pin);

	UEdGraphPin* GetPin();

	UEdGraphNode* GetNode() const
	{
		return Node.IsValid() ? Node.Get() : nullptr;
	}

	bool IsValid()
	{
		return Node != nullptr && PinId.IsValid();
	}

	static TArray<FBANodePinHandle> ConvertArray(const TArray<UEdGraphPin*>& Pins)
	{
		TArray<FBANodePinHandle> Handles;

		for (UEdGraphPin* const Pin : Pins)
		{
			Handles.Add(FBANodePinHandle(Pin));
		}

		return Handles;
	}

	bool operator==(const FBANodePinHandle& Other) const
	{
		return PinId == Other.PinId && Node == Other.Node;
	}

	bool operator==(const UEdGraphPin* Other)
	{
		return GetPin() == Other;
	}

	friend inline uint32 GetTypeHash(const FBANodePinHandle& Handle)
	{
		return HashCombine(GetTypeHash(Handle.PinId), GetTypeHash(Handle.Node->NodeGuid));
	}
};
