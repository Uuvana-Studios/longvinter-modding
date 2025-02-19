// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "UObject/NoExportTypes.h"

#include "Executor.h"

#include "AsyncTask.generated.h"

/**
* Base async task class
*/
UCLASS(Category = "Threading|Async", Abstract, BlueprintType)
class ASYNCBLUEPRINTSEXTENSION_API UAsyncTask : public UObject
{
	GENERATED_BODY()

protected:
	/**
	* Running state
	*/
	UPROPERTY(Category = "Threading|Async", BlueprintReadOnly, Meta = (AllowPrivateAccess))
	bool isStillRunning;

	Executor* executor;

protected:
	virtual EAsyncExecution getAsyncExecutionType() const;
	
public:
	UAsyncTask();

	/**
	* Called before Execute
	*/
	UFUNCTION(Category = Initialization, BlueprintImplementableEvent)
	void initialization();

	/**
	* This method will run asynchronously
	*/
	UFUNCTION(Category = Async, BlueprintImplementableEvent)
	void execute();

	virtual bool run(TFunction<void()>&& callback);

	virtual void stop();

	virtual ~UAsyncTask();
};
