// Copyright June Rhodes. All Rights Reserved.

#include "UniqueNetIdConversion.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemBlueprintsModule.h"

FUniqueNetIdRepl ConvertDangerousUniqueNetIdToRepl(const FUniqueNetId &UniqueNetIdByRef)
{
    // Some online subsystems are poorly implemented, and they don't always
    // properly call MakeShared on the unique net IDs before they're returned
    // to the online subsystem callers. If they haven't had
    // MakeShared/MakeShareable called on them, then passing these references
    // to the FUniqueNetIdRepl will cause an assert failed, because no
    // underlying shared pointer exists.

    // If the pointer isn't shareable, we need to grab it's string representation, and
    // pass it back into the current identity subsystem to create a new, properly
    // shared pointer.
    if (!UniqueNetIdByRef.DoesSharedInstanceExist())
    {
        IOnlineSubsystem *LocalSubsystem = IOnlineSubsystem::Get(UniqueNetIdByRef.GetType());
        if (LocalSubsystem == nullptr)
        {
            UE_LOG(
                LogOnlineBlueprint,
                Error,
                TEXT("A bad unique net ID ref had type '%s', but there is no online subsystem with that name. Online "
                     "Subsystem Blueprints will attempt to fix this ID using the default subsystem, but this may not "
                     "result in correct behaviour."),
                *UniqueNetIdByRef.GetType().ToString());
            LocalSubsystem = IOnlineSubsystem::Get();
        }

        IOnlineIdentityPtr Identity = LocalSubsystem->GetIdentityInterface();
        if (Identity == nullptr)
        {
            UE_LOG(
                LogOnlineBlueprint,
                Error,
                TEXT("Unable to fix bad unique net ID ref! Your online subsystem implementation is incorrectly "
                     "allocated FUniqueNetId instances on the stack, and it doesn't provide an identity subsystem "
                     "implementation."));
            return FUniqueNetIdRepl();
        }

        TSharedPtr<const FUniqueNetId> Ptr = Identity->CreateUniquePlayerId(UniqueNetIdByRef.ToString());
        if (Ptr == nullptr)
        {
            UE_LOG(
                LogOnlineBlueprint,
                Error,
                TEXT("Unable to fix bad unique net ID ref! Your online subsystem implementation incorrectly allocated "
                     "FUniqueNetId instances on the stack and returns nullptr from CreateUniquePlayerId for unique net "
                     "IDs that it created itself."));
            return FUniqueNetIdRepl();
        }
        if (!Ptr->GetType().IsEqual(UniqueNetIdByRef.GetType()))
        {
            UE_LOG(
                LogOnlineBlueprint,
                Error,
                TEXT("Unable to fix bad unique net ID ref! Your online subsystem implementation returned an allocated "
                     "FUniqueNetId instance with type '%s', but the bad unique net ID had a type of '%s'."),
                *Ptr->GetType().ToString(),
                *UniqueNetIdByRef.GetType().ToString());
            return FUniqueNetIdRepl();
        }

        return FUniqueNetIdRepl(Ptr);
    }

    // Otherwise it's safe to call.
    return FUniqueNetIdRepl(UniqueNetIdByRef);
}