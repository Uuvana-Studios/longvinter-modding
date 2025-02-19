// Copyright (c) 2022 Semyon Gritsenko

#include "Tasks/LongAsyncTask.h"

EAsyncExecution ULongAsyncTask::getAsyncExecutionType() const
{
	return EAsyncExecution::Thread;
}
