// Copyright June Rhodes. All Rights Reserved.

#include "Types/BlockedQueryResultInfo.h"

FBlockedQueryResultInfo FBlockedQueryResultInfo::FromNative(const FBlockedQueryResult &InObj)
{
    FBlockedQueryResultInfo Result;
    Result.Blocked = InObj.bIsBlocked;
    Result.BlockedNonFriends = InObj.bIsBlockedNonFriends;
    Result.UserId = InObj.UserId;
    return Result;
}

FBlockedQueryResult FBlockedQueryResultInfo::ToNative()
{
    FBlockedQueryResult Result;
    Result.bIsBlocked = this->Blocked;
    Result.bIsBlockedNonFriends = this->BlockedNonFriends;
    Result.UserId = this->UserId;
    return Result;
}