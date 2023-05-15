// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBFileTypes.h"

#include "Interfaces/OnlineUserCloudInterface.h"

UFileData *UFileData::FromNative(const TArray<uint8> &InData)
{
    auto Ref = NewObject<UFileData>();
    Ref->Data = InData;
    return Ref;
}

FCloudFileHeaderBP FCloudFileHeaderBP::FromNative(const FCloudFileHeader &FileHeader)
{
    FCloudFileHeaderBP Result;
    Result.Hash = FileHeader.Hash;
    Result.HashType = FileHeader.HashType;
    Result.DLName = FileHeader.DLName;
    Result.FileName = FileHeader.FileName;
    Result.FileSize = FileHeader.FileSize;
    Result.URL = FileHeader.URL;
    Result.ChunkID = FileHeader.ChunkID;
    return Result;
}

FPagedQuery FPagedQueryBP::ToNative() const
{
    FPagedQuery Result;
    Result.Start = this->Start;
    Result.Count = this->Count;
    return Result;
}