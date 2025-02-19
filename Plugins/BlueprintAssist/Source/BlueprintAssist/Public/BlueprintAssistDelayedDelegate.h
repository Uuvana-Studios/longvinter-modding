// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_DELEGATE(FBAOnDelayEnded);

class BLUEPRINTASSIST_API FBADelayedDelegate
{
	FBAOnDelayEnded Delegate;
	int32 TicksRemaining = -1;

public:
	void SetOnDelayEnded(FBAOnDelayEnded OnDelayEnded);
	void StartDelay(int32 NumTicks);

	void Tick();

	bool IsComplete() const { return TicksRemaining == -1; }
	bool IsActive() const { return TicksRemaining >= 0; }

	void Cancel();
};
