// Copyright (c) 2022 Semyon Gritsenko

#include "AsyncLibrary.h"

#include "HAL/ThreadManager.h"

#include "AsyncTasksManager.h"

DEFINE_LOG_CATEGORY(LogAsyncBlueprintsExtension);

void UAsyncLibrary::runCallbackInGameThread(const TFunction<void()>& callback)
{
	AsyncTask(ENamedThreads::Type::GameThread, [callback]() { callback(); });
}

bool UAsyncLibrary::runStatelessAsyncTask(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound();

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

bool UAsyncLibrary::runStatefulAsyncTask(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);
		
		return false;
	}

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound(task);

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

bool UAsyncLibrary::runStatelessAsyncTaskWithManager(UAsyncTask* task, const FStatelessAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	task->Rename(nullptr, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound();

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

bool UAsyncLibrary::runStatefulAsyncTaskWithManager(UAsyncTask* task, const FStatefulAsyncTaskCallback& callback)
{
	if (!IsValid(task))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Task instance is not valid"), ELogVerbosity::Type::Warning);

		return false;
	}

	task->Rename(nullptr, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound(task);

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

bool UAsyncLibrary::createAndRunStatelessAsyncTask(TSubclassOf<UAsyncTask> taskClass, const FStatelessAsyncTaskCallback& callback)
{
	UAsyncTask* task = UAsyncLibrary::makeAsyncTask(taskClass, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound();

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

bool UAsyncLibrary::createAndRunStatefulAsyncTask(TSubclassOf<UAsyncTask> taskClass, const FStatefulAsyncTaskCallback& callback)
{
	UAsyncTask* task = UAsyncLibrary::makeAsyncTask(taskClass, UAsyncTasksManager::get());

	return task->run([callback, task]()
		{
			UAsyncLibrary::runCallbackInGameThread([callback, task]()
				{
					callback.ExecuteIfBound(task);

					UAsyncTasksManager::get()->removeTask(task);
				});
		});
}

void UAsyncLibrary::runOnGameThread(const FGameThreadFunction& delegate)
{
	AsyncTask(ENamedThreads::Type::GameThread, [delegate]() { delegate.Execute(); });
}

void UAsyncLibrary::sleep(float seconds)
{
	FPlatformProcess::Sleep(seconds);
}

void UAsyncLibrary::wait(float seconds)
{
	FPlatformProcess::Sleep(seconds);
}

int64 UAsyncLibrary::getCurrentThreadId()
{
	return StaticCast<int64>(FPlatformTLS::GetCurrentThreadId());
}

FString UAsyncLibrary::getCurrentThreadName()
{
	return FThreadManager::Get().GetThreadName(UAsyncLibrary::getCurrentThreadId());
}

UAsyncTask* UAsyncLibrary::makeAsyncTask(TSubclassOf<UAsyncTask> taskClass, UObject* outer)
{
	if (!IsValid(outer))
	{
		FDebug::LogFormattedMessageWithCallstack("LogAsyncBlueprintsExtension", __FILE__, __LINE__, TEXT("Warning"), TEXT("Outer is not valid"), ELogVerbosity::Type::Warning);

		return nullptr;
	}

	return NewObject<UAsyncTask>(outer, taskClass);
}

UAsyncTasksManager* UAsyncLibrary::getAsyncTasksManager()
{
	return UAsyncTasksManager::get();
}

void UAsyncLibrary::stopTasks()
{
	UAsyncTasksManager::get()->clearAllTasks();
}
