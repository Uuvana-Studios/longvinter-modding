// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "AsyncUtility.h"

#include "AsyncLibrary.generated.h"

UCLASS()
class ASYNCBLUEPRINTSEXTENSION_API UAsyncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

private:
	static void runCallbackInGameThread(const TFunction<void()>& callback);

public:
	/**
	* Run task asynchronous with optional callback after task completes 
	* You need to save Task variable during all async task to prevent garbage collection for Task
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatelessAsyncTask(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* You need to save Task variable during all async task to prevent garbage collection for Task
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatefulAsyncTask(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Async task manager replace outer with itself
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatelessAsyncTaskWithManager(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Async task manager replace outer with itself
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatefulAsyncTaskWithManager(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* @param taskClass Class reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool createAndRunStatelessAsyncTask(UPARAM(Meta = (AllowAbstract = "false")) TSubclassOf<UAsyncTask> taskClass, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* @param taskClass Class reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool createAndRunStatefulAsyncTask(UPARAM(Meta = (AllowAbstract = "false")) TSubclassOf<UAsyncTask> taskClass, const FStatefulAsyncTaskCallback& callback);

	/**
	* Run Delegate in main game thread
	* You may use this function to modify UI or something else from another thread
	* @param delegate Function to called in main game thread
	*/
	UFUNCTION(Category = Threading, BlueprintCallable)
	static void runOnGameThread(const FGameThreadFunction& delegate);

	/**
	* Block thread
	* Don't use in main game thread
	* @param seconds Block non-main thread for this number of seconds
	*/
	UFUNCTION(Category = Threading, BlueprintCallable, Meta = (CompactNodeTitle = Sleep))
	static void sleep(float seconds = 1.0f);

	/**
	* Block thread
	* Don't use in main game thread
	* @param seconds Block non-main thread for this number of seconds
	*/
	UFUNCTION(Category = Threading, BlueprintCallable, Meta = (CompactNodeTitle = Wait))
	static void wait(float seconds = 1.0f);

	/**
	* Returns current thread id
	*/
	UFUNCTION(Category = "Threading|Utility", BlueprintCallable, BlueprintPure)
	static UPARAM(DisplayName = ThreadId) int64 getCurrentThreadId();

	/**
	* Returns current thread name
	* May returns empty string if thread has no name
	*/
	UFUNCTION(Category = "Threading|Utility", BlueprintCallable, BlueprintPure)
	static UPARAM(DisplayName = ThreadName) FString getCurrentThreadName();

	/**
	* Create AsyncTask
	* If outer is invalid returns null
	* @param taskClass Class reference of AsyncTask subclass
	* @param outer Owner of the constructed async task
	*/
	UFUNCTION(Category = "Threading|Utility", BlueprintCallable, Meta = (DefaultToSelf = outer))
	static UPARAM(DisplayName = Result) UAsyncTask* makeAsyncTask(UPARAM(Meta = (AllowAbstract = "false")) TSubclassOf<UAsyncTask> taskClass, UObject* outer);

	/**
	* Getter for Async Tasks Manager
	*/
	UFUNCTION(Category = Threading, BlueprintCallable, BlueprintPure)
	static UAsyncTasksManager* getAsyncTasksManager();

	/**
	* Primarily used for stopping infinite async tasks
	*/
	UFUNCTION(Category = "Threading|Utility", BlueprintCallable)
	static void stopTasks();
};
