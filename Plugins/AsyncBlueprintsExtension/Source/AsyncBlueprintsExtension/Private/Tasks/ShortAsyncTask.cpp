// Copyright (c) 2022 Semyon Gritsenko

#include "Tasks/ShortAsyncTask.h"

EAsyncExecution UShortAsyncTask::getAsyncExecutionType() const
{
	return EAsyncExecution::ThreadPool;
}
