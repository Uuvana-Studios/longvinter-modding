// Copyright June Rhodes. All Rights Reserved.

#include "Types/OnlineRecentPlayerRef.h"

UOnlineRecentPlayerRef *UOnlineRecentPlayerRef::FromRecentPlayer(const TSharedPtr<FOnlineRecentPlayer> &RecentPlayer)
{
    auto Ref = NewObject<UOnlineRecentPlayerRef>();
    Ref->RecentPlayer = RecentPlayer;
    Ref->AssignFromUser(RecentPlayer);
    return Ref;
}

FDateTime UOnlineRecentPlayerRef::GetLastSeen()
{
    return this->RecentPlayer->GetLastSeen();
}