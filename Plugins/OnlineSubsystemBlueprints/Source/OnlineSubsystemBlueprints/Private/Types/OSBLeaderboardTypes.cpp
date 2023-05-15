// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBLeaderboardTypes.h"

FOnlineStatsRowBP FOnlineStatsRowBP::FromNative(FOnlineStatsRow &Row)
{
    FOnlineStatsRowBP Result;
    Result.PlayerNickname = Row.NickName;
    Result.PlayerId = FUniqueNetIdRepl(Row.PlayerId);
    Result.Rank = Row.Rank;
    for (auto KV : Row.Columns)
    {
        Result.Columns.Add(KV.Key, FVariantDataBP::FromNative(KV.Value));
    }
    return Result;
}

FColumnMetaDataBP FColumnMetaDataBP::FromNative(FColumnMetaData &Column)
{
    FColumnMetaDataBP Result;
    Result.ColumnName = Column.ColumnName;
    Result.DataType = (EOnlineKeyValuePairDataType_)Column.DataType;
    return Result;
}

FColumnMetaData FColumnMetaDataBP::ToNative() const
{
    return FColumnMetaData(this->ColumnName, (EOnlineKeyValuePairDataType::Type)this->DataType);
}

void UOnlineLeaderboardRead::SetLeaderboardName(FName LeaderboardName)
{
    this->Leaderboard->LeaderboardName = LeaderboardName;
}

FName UOnlineLeaderboardRead::GetLeaderboardName() const
{
    return this->Leaderboard->LeaderboardName;
}

void UOnlineLeaderboardRead::SetSortedColumn(FName SortedColumn)
{
    this->Leaderboard->SortedColumn = SortedColumn;
}

FName UOnlineLeaderboardRead::GetSortedColumn() const
{
    return this->Leaderboard->SortedColumn;
}

EOnlineAsyncTaskState_ UOnlineLeaderboardRead::GetReadState() const
{
    return (EOnlineAsyncTaskState_)this->Leaderboard->ReadState;
}

void UOnlineLeaderboardRead::SetColumns(TArray<FColumnMetaDataBP> InColumns)
{
    this->Leaderboard->ColumnMetadata.Empty();
    for (auto Column : InColumns)
    {
        this->Leaderboard->ColumnMetadata.Add(Column.ToNative());
    }
}

TArray<FColumnMetaDataBP> UOnlineLeaderboardRead::GetColumns() const
{
    TArray<FColumnMetaDataBP> Columns;
    for (auto Column : this->Leaderboard->ColumnMetadata)
    {
        Columns.Add(FColumnMetaDataBP::FromNative(Column));
    }
    return Columns;
}

TArray<FOnlineStatsRowBP> UOnlineLeaderboardRead::GetRows() const
{
    TArray<FOnlineStatsRowBP> Rows;
    for (auto Row : this->Leaderboard->Rows)
    {
        Rows.Add(FOnlineStatsRowBP::FromNative(Row));
    }
    return Rows;
}

FOnlineStatsRowBP UOnlineLeaderboardRead::FindPlayerRecord(FUniqueNetIdRepl UserId, bool &OutFound) const
{
    if (!UserId.IsValid())
    {
        OutFound = false;
        return FOnlineStatsRowBP();
    }

    FOnlineStatsRow *Record = this->Leaderboard->FindPlayerRecord(*UserId.GetUniqueNetId());
    if (Record == nullptr)
    {
        OutFound = false;
        return FOnlineStatsRowBP();
    }

    OutFound = true;
    return FOnlineStatsRowBP::FromNative(*Record);
}

FOnlineLeaderboardReadRef &UOnlineLeaderboardRead::GetLeaderboard()
{
    return this->Leaderboard;
}

TSharedRef<FOnlineLeaderboardWrite> UOnlineLeaderboardWrite::ToNative()
{
    TSharedRef<FOnlineLeaderboardWrite> Leaderboard = MakeShared<FOnlineLeaderboardWrite>();
    return Leaderboard;
}