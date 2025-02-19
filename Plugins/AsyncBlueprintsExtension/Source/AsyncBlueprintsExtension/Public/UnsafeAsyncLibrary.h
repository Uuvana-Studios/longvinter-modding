// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"

#include "AsyncUtility.h"

#include "UnsafeAsyncLibrary.generated.h"

UCLASS()
class ASYNCBLUEPRINTSEXTENSION_API UUnsafeAsyncLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	* Run task asynchronous with optional callback after task completes
	* You need to save Task variable during all async task to prevent garbage collection for Task
	* Callback will run in separate thread
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatelessAsyncTaskWithCallbackInSeparateThread(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* You need to save Task variable during all async task to prevent garbage collection for Task
	* Callback will run in separate thread
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatefulAsyncTaskWithCallbackInSeparateThread(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Async task manager replace outer with itself
	* Callback will run in separate thread
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatelessAsyncTaskWithManagerWithCallbackInSeparateThread(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Async task manager replace outer with itself
	* Callback will run in separate thread
	* @param task Object reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool runStatefulAsyncTaskWithManagerWithCallbackInSeparateThread(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Callback will run in separate thread
	* @param taskClass Class reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate without parameters
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool createAndRunStatelessAsyncTaskWithCallbackInSeparateThread(UPARAM(Meta = (AllowAbstract = "false")) TSubclassOf<UAsyncTask> taskClass, const FStatelessAsyncTaskCallback& callback);

	/**
	* Run task asynchronous with optional callback after task completes
	* Callback will run in separate thread
	* @param taskClass Class reference of AsyncTask subclass
	* @param callback Called after task is completed. Delegate with Task as parameter
	*/
	UFUNCTION(Category = "Threading|Async", BlueprintCallable, Meta = (AutoCreateRefTerm = callback))
	static bool createAndRunStatefulAsyncTaskWithCallbackInSeparateThread(UPARAM(Meta = (AllowAbstract = "false")) TSubclassOf<UAsyncTask> taskClass, const FStatefulAsyncTaskCallback& callback);
};
