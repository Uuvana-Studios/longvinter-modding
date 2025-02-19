// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "AsyncTask.h"

#include "ShortAsyncTask.generated.h"

/**
 * Base class for short tasks
 */
UCLASS(Category = "Threading|Async", Abstract, BlueprintType, Blueprintable)
class ASYNCBLUEPRINTSEXTENSION_API UShortAsyncTask : public UAsyncTask
{
	GENERATED_BODY()
	
private:
	virtual EAsyncExecution getAsyncExecutionType() const override;

public:
	UShortAsyncTask() = default;

	virtual ~UShortAsyncTask() = default;
};
