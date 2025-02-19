// Copyright (c) 2022 Semyon Gritsenko

#include "Tasks/AsyncTask.h"

#include "AsyncTasksManager.h"

EAsyncExecution UAsyncTask::getAsyncExecutionType() const
{
	PURE_VIRTUAL(UAsyncTask::getAsyncExecutionType, return EAsyncExecution::Thread;);
}

UAsyncTask::UAsyncTask() :
	isStillRunning(false),
	executor(nullptr)
{

}

bool UAsyncTask::run(TFunction<void()>&& callback)
{
	if (executor)
	{
		return false;
	}

	this->initialization();

	UAsyncTasksManager::get()->addTask(this);

	isStillRunning = true;

	executor = new Executor([this]() { this->execute(); }, MoveTemp(callback), this->getAsyncExecutionType(), this);

	return true;
}

void UAsyncTask::stop()
{
	executor->stop();

	delete executor;

	executor = nullptr;

	isStillRunning = false;
}

UAsyncTask::~UAsyncTask()
{
	if (executor)
	{
		this->stop();
	}
}
