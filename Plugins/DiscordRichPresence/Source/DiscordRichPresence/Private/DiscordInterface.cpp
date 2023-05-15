// Discord Rich Presence Plugin - (c) Jonathan Verbeek 2019

#include "DiscordInterface.h"
#include "Runtime/Core/Public/Misc/ConfigCacheIni.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <shellapi.h>
#include <ShlObj.h>
#include <LM.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include "Windows/HideWindowsPlatformTypes.h"
#include "Windows/WindowsPlatformMisc.h"

DEFINE_LOG_CATEGORY(LogDiscord);

/*
// Compiler fix
UDiscordInterface* UDiscordInterface::Singleton = NULL;

UDiscordInterface* UDiscordInterface::GetDiscord(UObject* WorldContextObject)
{
	if (!Singleton && WorldContextObject->GetWorld())
	{
		Singleton = NewObject<UDiscordInterface>(WorldContextObject);
		//Singleton->AddToRoot();
		Singleton->SetFlags(RF_Standalone);
	}

	return Singleton;
}
*/
bool UDiscordInterface::InitializeDiscord(FString ClientID)
{
	// Discord is only supported on Windows and on Clients
#if !UE_SERVER && PLATFORM_WINDOWS
	// If the Discord interface already exists, don't try initializing a new one
	if (DiscordCore)
	{
		return true;
	}

	UE_LOG(LogDiscord, Log, TEXT("Initializing Discord"));

	// We only want to initialize Discord when the Discord client is running
	if (!FPlatformProcess::IsApplicationRunning(TEXT("Discord")) && !FPlatformProcess::IsApplicationRunning(TEXT("DiscordPTB")) && !FPlatformProcess::IsApplicationRunning(TEXT("Discord Canary")))
	{
		UE_LOG(LogDiscord, Log, TEXT("Discord is not running, skipping initialize"));
		return false;
	}

	// Make sure the Discord installation is complete
	if (!CheckForCompleteInstallation())
	{
		UE_LOG(LogDiscord, Log, TEXT("Discord installation is not complete, skipping initialize"));
		return false;
	}

	// Convert the string client ID to a uint64_t
	int64_t DiscordClientID = FCString::Atoi64(*ClientID);

	// Initialize Discord
	discord::Result InitResult = discord::Core::Create(DiscordClientID, DiscordCreateFlags_Default, &DiscordCore);

	// Did it work?
	if (InitResult != discord::Result::Ok)
	{
		UE_LOG(LogDiscord, Error, TEXT("Error initializing Discord Core! Error code: %d"), (int32)InitResult);
		return false;
	}

	UE_LOG(LogDiscord, Log, TEXT("Initialized Discord Core! Client: %I64d"), DiscordClientID);

	// Set a log hook for Discord
	DiscordCore->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel DiscordLogLevel, const char* Message) {
		switch (DiscordLogLevel)
		{
		case discord::LogLevel::Error:
			UE_LOG(LogDiscord, Error, TEXT("%s"), UTF8_TO_TCHAR(Message));
			break;
		case discord::LogLevel::Warn:
			UE_LOG(LogDiscord, Warning, TEXT("%s"), UTF8_TO_TCHAR(Message));
			break;
		case discord::LogLevel::Info:
			UE_LOG(LogDiscord, Log, TEXT("%s"), UTF8_TO_TCHAR(Message));
			break;
		case discord::LogLevel::Debug:
			UE_LOG(LogDiscord, Log, TEXT("%s"), *FString::Printf(TEXT("[DEBUG] %s"), UTF8_TO_TCHAR(Message)));
			break;
		default:
			break;
		}
	});

	// Hook OnActivityJoin
	DiscordCore->ActivityManager().OnActivityJoin.Connect([=](const char* JoinSecret) {
		UE_LOG(LogDiscord, Log, TEXT("Activity Join: %s"), UTF8_TO_TCHAR(JoinSecret));
		DiscordUserJoin.ExecuteIfBound(UTF8_TO_TCHAR(JoinSecret));
	});

	// Hook OnActivitySpectate
	DiscordCore->ActivityManager().OnActivitySpectate.Connect([=](const char* SpectateSecret) {
		UE_LOG(LogDiscord, Log, TEXT("Activity Spectate: %s"), UTF8_TO_TCHAR(SpectateSecret));
		DiscordUserSpectate.ExecuteIfBound(UTF8_TO_TCHAR(SpectateSecret));
	});

	// Hook OnActivityJoinRequest
	DiscordCore->ActivityManager().OnActivityJoinRequest.Connect([=](discord::User User) {
		UE_LOG(LogDiscord, Log, TEXT("Activity Join Reqest: %s#%s"), UTF8_TO_TCHAR(User.GetUsername()), UTF8_TO_TCHAR(User.GetDiscriminator()));
		DiscordUserJoinRequest.ExecuteIfBound(FDiscordUser(User));
	});

	// Hook OnActivityInvite
	DiscordCore->ActivityManager().OnActivityInvite.Connect([=](discord::ActivityActionType Type, discord::User User, discord::Activity NA) {
		UE_LOG(LogDiscord, Log, TEXT("Activity Invite: %d: %s#%s"), (int32)Type, UTF8_TO_TCHAR(User.GetUsername()), UTF8_TO_TCHAR(User.GetDiscriminator()));
		DiscordInvite.ExecuteIfBound((EDiscordPresenceActivityActionType)Type, FDiscordUser(User), FDiscordActivityInfo(NA));
	});

	return true;
#else
	UE_LOG(LogDiscord, Error, TEXT("Error initializing Discord because this is running on a server and/or not on Windows, which is not supported"));
	return false;
#endif
}

bool UDiscordInterface::CheckForCompleteInstallation()
{
	// Get the %localappdata% path
	FString LocalAppDataPath;
	TCHAR* UserPath;

	// Get the local AppData directory
	HRESULT Ret = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &UserPath);
	if (SUCCEEDED(Ret))
	{
		// Sanitize the path
		LocalAppDataPath = FString(UserPath).Replace(TEXT("\\"), TEXT("/")) + TEXT("/");
		CoTaskMemFree(UserPath);

		// Format various paths
		FString DiscordPath = LocalAppDataPath / TEXT("Discord");
		FString PackagesPath = DiscordPath / TEXT("packages");
		FString NUPKGPath = PackagesPath / TEXT("Discord-*.nupkg");

		// Search for the Discord-*.nupkg file which contains the version of the client
		TArray<FString> FoundPackageFiles;
		IFileManager::Get().FindFiles(FoundPackageFiles, *NUPKGPath, true, false);

		if (FoundPackageFiles.Num() <= 0)
		{
			UE_LOG(LogDiscord, Log, TEXT("Didn't find .nupkg file!"));
			return false;
		}
		else
		{
			// The file looks like this: Discord-0.0.306-full.nupkg, so split it by the - char
			TArray<FString> DelimittedByDash;
			if (FoundPackageFiles[0].ParseIntoArray(DelimittedByDash, TEXT("-")) == 3)
			{
				// The version is the second element in the array
				FString VersionString = DelimittedByDash[1];

				// Construct the path of the build_info.json file
				FString BuildInfoJSONPath = DiscordPath / FString::Printf(TEXT("app-%s"), *VersionString) / TEXT("resources") / TEXT("build_info.json");

				// Check if it exists
				if (FPaths::FileExists(BuildInfoJSONPath))
				{
					return true;
				}
				else
				{
					UE_LOG(LogDiscord, Log, TEXT("build_info.json does not exist!"));
					return false;
				}
			}
			else
			{
				UE_LOG(LogDiscord, Log, TEXT("Error finding .nupkg file: %s"), *FoundPackageFiles[0]);
			}

			return false;
		}
	}
	else
	{
		UE_LOG(LogDiscord, Log, TEXT("Error retrieving local app data path!"));
		return false;
	}
}

void UDiscordInterface::ShutdownDiscord()
{
	if (DiscordCore)
	{
		ClearActivity();

		DiscordCore = NULL;
	}
}

void UDiscordInterface::TickDiscord()
{
	if (DiscordCore)
	{
		DiscordCore->RunCallbacks();
	}
}

void UDiscordInterface::SetActivity(FDiscordActivityInfo Activity)
{
	if (DiscordCore)
	{
		discord::Activity NativeActivity = Activity.ConvertToNative();

		// Update the activity
		DiscordCore->ActivityManager().UpdateActivity(NativeActivity, [](discord::Result result) {
			UE_LOG(LogDiscord, Log, TEXT("Updated activity: %s"), (result == discord::Result::Ok) ? TEXT("Success") : TEXT("Failed"));
		});
	}
	else
	{
		UE_LOG(LogDiscord, Error, TEXT("Discord not initialized!"));
	}
}

void UDiscordInterface::ClearActivity()
{
	if (DiscordCore)
	{
		DiscordCore->ActivityManager().ClearActivity([](discord::Result result) {
			UE_LOG(LogDiscord, Log, TEXT("Cleared activity: %s"), (result == discord::Result::Ok) ? TEXT("Success") : TEXT("Failed"));
		});
	}
	else
	{
		UE_LOG(LogDiscord, Error, TEXT("Discord not initialized!"));
	}
}

void UDiscordInterface::RegisterApplication(FString Command)
{
	if (DiscordCore)
	{
		DiscordCore->ActivityManager().RegisterCommand(TCHAR_TO_UTF8(*Command));
		UE_LOG(LogDiscord, Log, TEXT("Registered launch command \"%s\""), *Command);
	}
	else
	{
		UE_LOG(LogDiscord, Error, TEXT("Discord not initialized!"));
	}
}

void UDiscordInterface::RegisterApplicationSteam()
{
	if (DiscordCore)
	{
		int32 SteamAppID = 0;
		if (GConfig->GetInt(TEXT("OnlineSubsystemSteam"), TEXT("SteamDevAppId"), SteamAppID, GEngineIni) && SteamAppID > 0)
		{
			DiscordCore->ActivityManager().RegisterSteam((uint32_t)SteamAppID);
			UE_LOG(LogDiscord, Log, TEXT("Registered steam app %d"), SteamAppID);
		}
		else
		{
			UE_LOG(LogDiscord, Error, TEXT("Error registering Steam app: Couldn't retrieve Steam app id!"));
		}
	}
	else
	{
		UE_LOG(LogDiscord, Error, TEXT("Discord not initialized!"));
	}
}

void UDiscordInterface::BindEvents(FOnDiscordUserJoin OnDiscordUserJoin, FOnDiscordUserSpectate OnDiscordUserSpectate, FOnDiscordUserJoinRequest OnDiscordUserJoinRequest, FOnDiscordInvite OnDiscordInvite)
{
	DiscordUserJoin = OnDiscordUserJoin;
	DiscordUserSpectate = OnDiscordUserSpectate;
	DiscordUserJoinRequest = OnDiscordUserJoinRequest;
	DiscordInvite = OnDiscordInvite;
}

void UDiscordInterface::BindOnUserJoin(FOnDiscordUserJoin OnDiscordUserJoin)
{
	DiscordUserJoin = OnDiscordUserJoin;
}

void UDiscordInterface::BindOnUserSpectate(FOnDiscordUserSpectate OnDiscordUserSpectate)
{
	DiscordUserSpectate = OnDiscordUserSpectate;
}

void UDiscordInterface::BindOnUserJoinRequest(FOnDiscordUserJoinRequest OnDiscordUserJoinRequest)
{
	DiscordUserJoinRequest = OnDiscordUserJoinRequest;
}

void UDiscordInterface::BindOnUserInvite(FOnDiscordInvite OnDiscordInvite)
{
	DiscordInvite = OnDiscordInvite;
}

