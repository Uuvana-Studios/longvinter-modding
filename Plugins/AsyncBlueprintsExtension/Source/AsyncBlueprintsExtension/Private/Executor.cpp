// Copyright (c) 2022 Semyon Gritsenko

#include "Executor.h"

#include "Tasks/AsyncTask.h"

Executor::Executor(TFunction<void()>&& function, TFunction<void()>&& callback, EAsyncExecution executionType, UAsyncTask* owner) :
	owner(owner)
{
	future = Async(executionType, MoveTemp(function), MoveTemp(callback));
}

void Executor::stop()
{
	future.Wait();
}
