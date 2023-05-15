// Copyright Pandores Marketplace 2021. All Rights Reserved.


#include "DiscordManagerUtils.h"

void FDiscordManagerUtils::DiscordResultCallback_AutoDelete(void* Data, FRawDiscord::EDiscordResult Result)
{
	FDiscordResultCallback* const Cb = (FDiscordResultCallback*)Data;

	Cb->ExecuteIfBound(FDiscordResult::ToEDiscordResult(Result));

	delete Cb;
}
