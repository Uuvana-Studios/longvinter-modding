// Copyright June Rhodes. All Rights Reserved.

#include "Types/OnlineErrorInfo.h"

FOnlineErrorInfo FOnlineErrorInfo::FromNative(const FOnlineError &InObj)
{
    FOnlineErrorInfo Result;
    Result.Successful = InObj.bSucceeded;
    Result.ErrorRaw = InObj.ErrorRaw;
    Result.ErrorCode = InObj.ErrorCode;
    Result.ErrorMessage = InObj.ErrorMessage;
    return Result;
}

FOnlineError FOnlineErrorInfo::ToNative()
{
    FOnlineError Result;
    Result.bSucceeded = this->Successful;
    Result.ErrorRaw = this->ErrorRaw;
    Result.ErrorCode = this->ErrorCode;
    Result.ErrorMessage = this->ErrorMessage;
    return Result;
}