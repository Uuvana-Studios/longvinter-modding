// Copyright June Rhodes. All Rights Reserved.

#include "Types/OnlineAccountCredential.h"

FOnlineAccountCredential FOnlineAccountCredential::FromNative(const FOnlineAccountCredentials &InCred)
{
    FOnlineAccountCredential Cred;
    Cred.Type = InCred.Type;
    Cred.Id = InCred.Id;
    Cred.Token = InCred.Token;
    return Cred;
}

FOnlineAccountCredentials FOnlineAccountCredential::ToNative()
{
    FOnlineAccountCredentials Cred;
    Cred.Type = this->Type;
    Cred.Id = this->Id;
    Cred.Token = this->Token;
    return Cred;
}