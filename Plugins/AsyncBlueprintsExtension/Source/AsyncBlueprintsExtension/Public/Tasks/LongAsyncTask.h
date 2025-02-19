// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "AsyncTask.h"

#include "LongAsyncTask.generated.h"

/**
 * Base class for long tasks
 */
UCLASS(Category = "Threading|Async", Abstract, BlueprintType, Blueprintable)
class ASYNCBLUEPRINTSEXTENSION_API ULongAsyncTask : public UAsyncTask
{
	GENERATED_BODY()
	
protected:
	virtual EAsyncExecution getAsyncExecutionType() const override;

public:
	ULongAsyncTask() = default;

	virtual ~ULongAsyncTask() = default;
};
