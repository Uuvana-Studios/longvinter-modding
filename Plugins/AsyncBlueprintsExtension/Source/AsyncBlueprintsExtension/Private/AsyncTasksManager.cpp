// Copyright (c) 2022 Semyon Gritsenko

#include "AsyncTasksManager.h"

#include "Tasks/InfiniteAsyncTask.h"

UAsyncTasksManager* UAsyncTasksManager::get()
{
	static UAsyncTasksManager* tasksManager = nullptr;

	if (!tasksManager && StaticCast<bool>(GetTransientPackage()))
	{
		tasksManager = NewObject<UAsyncTasksManager>();

		tasksManager->AddToRoot();
	}

	return tasksManager;
}

void UAsyncTasksManager::addTask(UAsyncTask* task)
{
	FScopeLock lock(&tasksMutex);

	tasks.Add(task);
}

void UAsyncTasksManager::removeTask(UAsyncTask* task)
{
	FScopeLock lock(&tasksMutex);

	tasks.Remove(task);
}

void UAsyncTasksManager::stopAndWaitTask(UAsyncTask* task)
{
	task->stop();

	this->removeTask(task);
}

void UAsyncTasksManager::clearAllTasks()
{
	FScopeLock lock(&tasksMutex);

	for (UAsyncTask* task : tasks)
	{
		task->stop();
	}

	tasks.Empty();
}

void UAsyncTasksManager::clearInfiniteTasks()
{
	FScopeLock lock(&tasksMutex);

	TArray<UInfiniteAsyncTask*> tasksToRemove;

	for (UAsyncTask* task : tasks)
	{
		if (UInfiniteAsyncTask* infiniteTask = Cast<UInfiniteAsyncTask>(task))
		{
			tasksToRemove.Add(infiniteTask);

			task->stop();
		}
	}

	for (UInfiniteAsyncTask* task : tasksToRemove)
	{
		tasks.Remove(task);
	}
}
