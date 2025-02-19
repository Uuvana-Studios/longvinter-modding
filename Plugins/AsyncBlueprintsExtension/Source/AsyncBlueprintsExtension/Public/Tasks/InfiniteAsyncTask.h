// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "Tasks/LongAsyncTask.h"

#include "InfiniteAsyncTask.generated.h"

/**
 * Base class for infinite tasks
 */
UCLASS(Category = "Threading|Async", Abstract, BlueprintType, Blueprintable)
class ASYNCBLUEPRINTSEXTENSION_API UInfiniteAsyncTask : public ULongAsyncTask
{
	GENERATED_BODY()

private:
	/**
	* Running state
	*/
	UPROPERTY(Category = "Threading|Async", BlueprintReadWrite, Meta = (Deprecated, AllowPrivateAccess))
	bool isRunning;

public:
	UInfiniteAsyncTask();

	bool run(TFunction<void()>&& callback) override;

	/**
	* Stop task
	*/
	UFUNCTION(Category = "Threding|Async", BlueprintCallable)
	void stop() override;

	virtual ~UInfiniteAsyncTask();
};
