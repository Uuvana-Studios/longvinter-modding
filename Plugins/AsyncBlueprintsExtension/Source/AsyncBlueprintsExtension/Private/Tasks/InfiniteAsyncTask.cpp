// Copyright (c) 2022 Semyon Gritsenko

#include "Tasks/InfiniteAsyncTask.h"

#include "AsyncTasksManager.h"

UInfiniteAsyncTask::UInfiniteAsyncTask() :
	isRunning(false)
{

}

bool UInfiniteAsyncTask::run(TFunction<void()>&& callback)
{
	if (executor)
	{
		return false;
	}

	UAsyncTasksManager::get()->addTask(this);

	TFunction<void()> functionToExecute = [this]()
	{
		while (isRunning && isStillRunning)
		{
			this->execute();
		}
	};

	isRunning = true;

	isStillRunning = true;

	executor = new Executor(MoveTemp(functionToExecute), MoveTemp(callback), this->getAsyncExecutionType(), this);

	return true;
}

void UInfiniteAsyncTask::stop()
{
	isRunning = false;

	Super::stop();
}

UInfiniteAsyncTask::~UInfiniteAsyncTask()
{
	if (executor)
	{
		this->stop();
	}
}
