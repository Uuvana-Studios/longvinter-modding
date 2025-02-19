// Copyright (c) 2022 Semyon Gritsenko

#include "UnsafeAsyncLibrary.h"

#include "AsyncTasksManager.h"
#include "AsyncLibrary.h"

bool UUnsafeAsyncLibrary::runStatelessAsyncTaskWithCallbackInSeparateThread(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound();

			UAsyncTasksManager::get()->removeTask(task);
		});
}

bool UUnsafeAsyncLibrary::runStatefulAsyncTaskWithCallbackInSeparateThread(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound(task);

			UAsyncTasksManager::get()->removeTask(task);
		});
}

bool UUnsafeAsyncLibrary::runStatelessAsyncTaskWithManagerWithCallbackInSeparateThread(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	task->Rename(nullptr, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound();

			UAsyncTasksManager::get()->removeTask(task);
		});
}

bool UUnsafeAsyncLibrary::runStatefulAsyncTaskWithManagerWithCallbackInSeparateThread(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	task->Rename(nullptr, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound(task);

			UAsyncTasksManager::get()->removeTask(task);
		});
}

bool UUnsafeAsyncLibrary::createAndRunStatelessAsyncTaskWithCallbackInSeparateThread(TSubclassOf<UAsyncTask> taskClass, const FStatelessAsyncTaskCallback& callback)
{
	UAsyncTask* task = UAsyncLibrary::makeAsyncTask(taskClass, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound();

			UAsyncTasksManager::get()->removeTask(task);
		});
}

bool UUnsafeAsyncLibrary::createAndRunStatefulAsyncTaskWithCallbackInSeparateThread(TSubclassOf<UAsyncTask> taskClass, const FStatefulAsyncTaskCallback& callback)
{
	UAsyncTask* task = UAsyncLibrary::makeAsyncTask(taskClass, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			callback.ExecuteIfBound(task);

			UAsyncTasksManager::get()->removeTask(task);
		});
}
