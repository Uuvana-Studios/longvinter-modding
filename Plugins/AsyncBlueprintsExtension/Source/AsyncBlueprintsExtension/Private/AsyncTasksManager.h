// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "UObject/NoExportTypes.h"

#include "Tasks/AsyncTask.h"

#include "AsyncTasksManager.generated.h"

/**
 * Handle async task instances
 */
UCLASS()
class ASYNCBLUEPRINTSEXTENSION_API UAsyncTasksManager : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TSet<UAsyncTask*> tasks;

	FCriticalSection tasksMutex;

private:
	UAsyncTasksManager() = default;

	~UAsyncTasksManager() = default;
	
public:
	static UAsyncTasksManager* get();

	void addTask(UAsyncTask* task);

	void removeTask(UAsyncTask* task);

	/**
	* Stop and wait task
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable)
	void stopAndWaitTask(UAsyncTask* task);

	/**
	* Stop and wait for all tasks
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable)
	void clearAllTasks();

	/**
	* Stop and wait for all infinite tasks
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable)
	void clearInfiniteTasks();
};
