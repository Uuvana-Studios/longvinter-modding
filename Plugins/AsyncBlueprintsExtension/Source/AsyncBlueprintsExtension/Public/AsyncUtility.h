// Copyright (c) 2022 Semyon Gritsenko

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "Tasks/AsyncTask.h"

#include "AsyncUtility.generated.h"

DECLARE_DYNAMIC_DELEGATE(FStatelessAsyncTaskCallback);

DECLARE_DYNAMIC_DELEGATE_OneParam(FStatefulAsyncTaskCallback, UAsyncTask*, task);

DECLARE_DYNAMIC_DELEGATE(FGameThreadFunction);

DECLARE_LOG_CATEGORY_EXTERN(LogAsyncBlueprintsExtension, Display, All);

UCLASS()
class ASYNCBLUEPRINTSEXTENSION_API UAsyncUtility : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

};
