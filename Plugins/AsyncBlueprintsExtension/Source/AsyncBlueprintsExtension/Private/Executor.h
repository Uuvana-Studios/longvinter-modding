// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "Async/Async.h"

class UAsyncTask;

class ASYNCBLUEPRINTSEXTENSION_API Executor
{
private:
	TFuture<void> future;
	UAsyncTask* owner;

public:
	Executor(TFunction<void()>&& function, TFunction<void()>&& callback, EAsyncExecution executionType, UAsyncTask* owner);

	void stop();

	~Executor() = default;
};
