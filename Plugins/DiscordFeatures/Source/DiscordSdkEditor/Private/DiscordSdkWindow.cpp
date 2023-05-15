// Copyright Charmillot Clement 2020. All Rights Reserved.

#include "DiscordSdkWindow.h"
#include "DiscordSdk.h"
#include "Misc/SlowTask.h"
#include "Misc/Paths.h"
#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Misc/FileHelper.h"
#include "DiscordSdkModule.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "DiscordSettings.h"
#include "HAL/FileManager.h"
#include "Framework/SlateDelegates.h"
#include "DiscordSdkEditorStyle.h"
#include "DiscordSdkSettingsEditor.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/SWindow.h"
#include "Launch/Resources/Version.h"

#if !PLATFORM_MAC && !PLATFORM_IOS && !PLATFORM_ANDROID
#	include "zip/MinZipWrapper.h"
#endif



struct FSdkInstaller
{
	FSdkInstaller(TSharedPtr<FSlowTask> InTask)
		: Task(InTask)
	{
		ZippedSdkPath		= FPaths::ProjectSavedDir() / TEXT("Temp/DiscordSdk/DiscordSdk.zip");
		PluginBaseDir		= IPluginManager::Get().FindPlugin(TEXT("DiscordFeatures"))->GetBaseDir();
		TargetSdkLocation	= PluginBaseDir		/ TEXT("ThirdParty/DiscordSdk/");
		BinariesLocation	= TargetSdkLocation / TEXT("lib/x86_64/");
		WindowsBinaries		= BinariesLocation	/ TEXT("discord_game_sdk.dll");
		MacOSBinaries		= BinariesLocation	/ TEXT("discord_game_sdk.dylib");
		LinuxBinaries		= BinariesLocation	/ TEXT("discord_game_sdk.so");
	}

	bool SaveSdk(const TArray<uint8>& Sdk)
	{
		Task->DefaultMessage = FText::FromString("Saving the Discord SDK...");
		Task->EnterProgressFrame(1.f);

		if (!FFileHelper::SaveArrayToFile(Sdk, *ZippedSdkPath))
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Failed to save the downloaded Discord SDK."));
			Error(FString::Printf(TEXT("Failed to write downloaded compressed SDK to \"%s\""), *FPaths::ConvertRelativePathToFull(ZippedSdkPath)));
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Zipped Discord SDK saved to \"%s\"."), *FPaths::ConvertRelativePathToFull(ZippedSdkPath));
		
		return true;
	}

	bool UnzipFileSync()
	{
		Task->DefaultMessage = FText::FromString("Unzipping the Discord SDK...");
		Task->EnterProgressFrame(5.f);

		UE_LOG(LogDiscordSdk, Log, TEXT("Unzipping the Discord SDK to \"%s\"."), *FPaths::ConvertRelativePathToFull(TargetSdkLocation));

		const FString AbsoluteFrom = FPaths::ConvertRelativePathToFull(ZippedSdkPath);
		const FString AbsoluteTo   = FPaths::ConvertRelativePathToFull(TargetSdkLocation);

#if !PLATFORM_MAC
		TUniquePtr<FMinZip> MinZipObj = MakeUnique<FMinZip>(AbsoluteFrom);

		const EZipResult Res = MinZipObj->Extract(AbsoluteTo, TEXT(""));
		if (Res != EZipResult::Ok)
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Unzip failed: code %d."), (int32)Res);
			return false;
		}
#else // PLATFORM_MAC

		const FString ScriptLocation = PluginBaseDir / TEXT("Source/DiscordSdkEditor/Scripts/")
			
#if PLATFORM_WINDOWS
			TEXT("unzipsdk.bat")
#elif PLATFORM_MAC || PLATFORM_LINUX
			TEXT("unzipsdk.sh")
#else
#	error "Unsupported platform"
#endif
			;
		if (!FPaths::FileExists(ScriptLocation))
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Script to extract DiscordSdk not found at \"%s\""), *FPaths::ConvertRelativePathToFull(ScriptLocation));
			return false;
		}
		
		const FString Parameters = FString::Printf(TEXT("\"%s\" \"%s\""), *FPaths::ConvertRelativePathToFull(ZippedSdkPath), *FPaths::ConvertRelativePathToFull(TargetSdkLocation));
		
		int32 ReturnCode = 0;
		FString StdText, StdErr;

		if (!FPlatformProcess::ExecProcess(*ScriptLocation, *Parameters, &ReturnCode, &StdText, &StdErr))
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Failed to start DiscordSdk unzipper script."));
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Unzip Script output: %s"), *StdText);
		if (!StdErr.IsEmpty())
		{
			UE_LOG(LogDiscordSdk, Warning, TEXT("Unzip Script Error output: %s"), *StdErr);
		}
#endif // PLATFORM_MAC

		return true;
	}

	bool CheckFiles()
	{
		FPlatformProcess::Sleep(1.f); // Cool off

		if (!FPaths::DirectoryExists(TargetSdkLocation))
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Failed to unzip Discord SDK: The resulting folder doesn't exist."));
			UE_LOG(LogDiscordSdk, Error, TEXT("Expected to find unzipped Discord SDK at \"%s\". Platform: %s"), *FPaths::ConvertRelativePathToFull(TargetSdkLocation), *UGameplayStatics::GetPlatformName());
			Error(TEXT("Failed to unzip the SDK"));
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK unzipped to \"%s\"."), *FPaths::ConvertRelativePathToFull(TargetSdkLocation));

		if (!FPaths::DirectoryExists(BinariesLocation))
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Discord SDK is missing the lib folder."));
			UE_LOG(LogDiscordSdk, Error, TEXT("At \"%s\""), *FPaths::ConvertRelativePathToFull(BinariesLocation));
			Error(TEXT("Failed to install the Discord SDk: "));
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Checking binaries for platforms..."));

		FString MissingBinaries;
#define CHECK_SDK_FOR_PLATFORM(Platform) { \
	if (!FPaths::FileExists(Platform ## Binaries)) \
	{ \
		MissingBinaries += (MissingBinaries.Len() < 1 ?  TEXT("") : TEXT(", ")) + FString(TEXT( # Platform )); \
	} \
}
		CHECK_SDK_FOR_PLATFORM(Windows);
		CHECK_SDK_FOR_PLATFORM(MacOS);
		CHECK_SDK_FOR_PLATFORM(Linux);
#undef CHECK_SDK_FOR_PLATFORM

		if (MissingBinaries.Len() > 0)
		{
			UE_LOG(LogDiscordSdk, Warning, TEXT("Binaries for OS: %s not found in downloaded SDK."), *MissingBinaries);
			Error(FString::Printf(TEXT("The binaries for the following OS are missing: %s"), *MissingBinaries));
			return false;
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("All binaries found."));
		
		return true;
	}

	bool CleanupFiles()
	{
		UE_LOG(LogDiscordSdk, Log, TEXT("Deleting unused folders..."));

		Task->DefaultMessage = FText::FromString(TEXT("Cleaning download files..."));
		Task->EnterProgressFrame(10.f);

		if (!IFileManager::Get().Delete(*FPaths::ConvertRelativePathToFull(ZippedSdkPath)))
		{
			UE_LOG(LogDiscordSdk, Warning, TEXT("Failed to delete downloaded Discord SDK zip file."));
		}

		Task->EnterProgressFrame(2.f);

		const TArray<FString> UnusedSdkFolders =
		{
			TEXT("examples/"),
			TEXT("csharp/"),
			TEXT("cpp/"),
			TEXT("c/")
		};

		for (const FString& UnusedFolder : UnusedSdkFolders)
		{
			const FString Target = TargetSdkLocation / UnusedFolder;
			if (!IFileManager::Get().DeleteDirectory(*Target, false, true))
			{
				UE_LOG(LogDiscordSdk, Warning, TEXT("Failed to delete unused Discord SDK directory \"%s\"."), *FPaths::ConvertRelativePathToFull(Target));
			}
			Task->EnterProgressFrame(1.f);
		}

		UE_LOG(LogDiscordSdk, Log, TEXT("Unused folders deleted."));
		
		return true;
	}

	void Finalize()
	{
		UDiscordSettings::UpdateLocations(WindowsBinaries, MacOSBinaries, LinuxBinaries);
		FDiscordSdk::Get()->SettingsUpdated();
	}

	void Error(const FString& Message)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(
			FString::Printf(TEXT("Failed to install the Discord SDk: \n\t%s.\n\nTry to install the SDK manually."), 
		*Message)));
	}

private:
	TSharedPtr<FSlowTask> Task;
	FString TargetSdkLocation;
	FString BinariesLocation;
	FString WindowsBinaries;
	FString MacOSBinaries;
	FString LinuxBinaries;
	FString ZippedSdkPath;
	FString PluginBaseDir;
};


const FString SConfigureSdkWindow::DiscordSdkLocation = TEXT("https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip");

/* static */ TSharedRef<SWindow> SConfigureSdkWindow::CreateWindow()
{
	const TSharedRef<SWindow> NewModuleWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Configure Discord Features")))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.ClientSize(FVector2D(800, 400))
		.SupportsMaximize(false)
		.SupportsMinimize(false)
	[
		SNew(SConfigureSdkWindow)
	];

	return NewModuleWindow;
}

SConfigureSdkWindow::SConfigureSdkWindow()
	: SCompoundWidget()
    , Blurple (RGBToSRGB(114, 137, 218))
    , Dark    (RGBToSRGB(044, 047, 051))
	, LightRed(RGBToSRGB(230, 0, 0))
	, MissingSdkBackgroundColor(LightRed)
	, BlurpleBrush(Blurple)
	, DarkBrush(Dark)
{
	HyperlinkStyle
		.SetNormal (FSlateColorBrush(FColor(0.f, 0.f, 0.f, 0.f)))
		.SetPressed(FSlateColorBrush(FColor(0.f, 0.f, 0.f, 0.f)))
		.SetHovered(FSlateColorBrush(FColor(0.f, 0.f, 0.f, 0.f)));
	
}


void SConfigureSdkWindow::Construct(const FArguments& InArgs)
{
	const FSlateFontInfo TitleFont		(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 22);
	const FSlateFontInfo SubtitleFont	(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14);
	const FSlateFontInfo InputFont		(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 14);
	const FSlateFontInfo OptionFont		(FPaths::EngineContentDir() / TEXT("Slate/Fonts/Roboto-Bold.ttf"), 12);

	const bool bIsSdkValid = FDiscordSdk::Get()->AreAllDllsValid();

	ChildSlot
	[
		SNew(SBorder)
			.BorderImage(&DarkBrush)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
		[
			SNew(SBox)
				.Padding(FMargin(10.f))
				.VAlign(VAlign_Fill)
				.HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Discord Configuration")))
						.Font(TitleFont)
						.ColorAndOpacity(Blurple)
				]
				
				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(SSpacer)
						.Size(FVector2D(700.f, 15.f))
				]

				+ SVerticalBox::Slot()
				[
					SAssignNew(InvalidSdkNotice, SBorder)
						.Visibility(bIsSdkValid ? EVisibility::Collapsed : EVisibility::Visible)
						.BorderBackgroundColor(FLinearColor::Red)
						.Padding(FMargin{2.f, 2.f, 3.f, 3.f})
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.BorderImage(&MissingSdkBackgroundColor)
					[
						SNew(SBorder)
							.BorderBackgroundColor(LightRed)
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							.BorderImage(&MissingSdkBackgroundColor)
						[
							SNew(SHorizontalBox)
							
							+ SHorizontalBox::Slot()
								.FillWidth(2.f)
								.VAlign(VAlign_Center)
								.Padding(10.f, 0.f)
							[
								SNew(STextBlock)
									.Text(FText::FromString(TEXT("The Discord SDK is invalid.")))
									.ColorAndOpacity(FLinearColor::White)
									.Font(InputFont)
							]

							+ SHorizontalBox::Slot()
								.VAlign(VAlign_Center)
								.Padding(10.f, 0.f)
								.AutoWidth()
							[
								SNew(SButton)
									.VAlign(VAlign_Center)
									.HAlign(HAlign_Center)
									.OnClicked(this, &SConfigureSdkWindow::DownloadSdk)
								[
									SNew(STextBlock)
										.Text(FText::FromString(TEXT("Download the SDK")))
										.ColorAndOpacity(Dark)
								]
							]
						]
					]
				]
				
				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(SSpacer)
						.Size(FVector2D(0.f, 10.f))
				]

				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(SHorizontalBox)
					
					+ SHorizontalBox::Slot()
						.FillWidth(1.f)
					[
						SNew(SVerticalBox)
						
						+ SVerticalBox::Slot()
							.AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Application ID")))
								.Font(SubtitleFont)
								.ColorAndOpacity(Blurple)
						]

				
						+ SVerticalBox::Slot()
						[
							SNew(SSpacer)
								.Size(FVector2D{0.f, 5.f})
						]
						
						+ SVerticalBox::Slot()
							.AutoHeight()
						[
							SNew(SEditableTextBox)
								.HintText(FText::FromString(TEXT("Application ID")))
								.BackgroundColor(Blurple)
								.Font(InputFont)
								.ForegroundColor(Dark)
								.Text(FText::FromString(UDiscordSettings::GetClientIdAsString()))
								.OnTextChanged_Lambda([](const FText& NewText) -> void
								{
										UDiscordSettings::SetApplicationId(NewText.ToString());
								})
						]
					]

					+ SHorizontalBox::Slot()
					[
						SNew(SSpacer)
							.Size(FVector2D{10.f, 0.f})
					]

					+ SHorizontalBox::Slot()
						.FillWidth(1.f)
					[
						SNew(SVerticalBox)
						
						+ SVerticalBox::Slot()
							.AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Bot Token")))
								.Font(SubtitleFont)
								.ColorAndOpacity(Blurple)
						]
				
						+ SVerticalBox::Slot()
						[
							SNew(SSpacer)
								.Size(FVector2D{0.f, 5.f})
						]
						
						
						+ SVerticalBox::Slot()
							.AutoHeight()
						[
							SNew(SEditableTextBox)
								.HintText(FText::FromString(TEXT("Bot Token")))
								.BackgroundColor(Blurple)
								.Font(InputFont)
								.IsPassword(true)
								.ForegroundColor(Dark)
								.Text(FText::FromString(UDiscordEditorSettings::GetBotToken()))
								.OnTextChanged_Lambda([](const FText& NewText) -> void
								{
									UDiscordEditorSettings::SaveBotToken(NewText.ToString());
								})
						]
					]
				]
				
				
				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(SSpacer)
						.Size(FVector2D(0.f, 50.f))
				]

				+ SVerticalBox::Slot()
					.FillHeight(2.f)
				[

					SNew(SBox)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
					[
						SNew(SImage)
							.Image(FDiscordSdkEditorStyle::Get().GetBrush(TEXT("DiscordSdkEditor.DiscordBranding")))
					]
				]
				
				+ SVerticalBox::Slot()
					.AutoHeight()
				[
					SNew(SSpacer)
						.Size(FVector2D(0.f, 25.f))
				]

				+ SVerticalBox::Slot()
					.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)
					
					+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.FillWidth(1.f)
					[
						SHyperlinkText(TEXT("Documentation"), TEXT("https://pandoa.github.io/DiscordFeatures/#/"))
					]
					+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.FillWidth(1.f)
					[
						SHyperlinkText(TEXT("Discord SDK"), TEXT("https://discord.com/developers/docs/game-sdk/discord"))
					]
					+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.FillWidth(1.f)
					[
						SHyperlinkText(TEXT("Marketplace"), TEXT("https://www.unrealengine.com/marketplace/en-US/product/game-sdk-features"))
					]
					+ SHorizontalBox::Slot()
						.HAlign(HAlign_Center)
						.FillWidth(1.f)
					[
						SHyperlinkText(TEXT("Support"), TEXT("mailto:pandores.marketplace+DiscordFeatures@gmail.com?Subject=Discord%20Features%20-%20"))
					]
				]
			]
		]
	];
}

TSharedRef<SButton> SConfigureSdkWindow::SHyperlinkText(const FString& DisplayText, const FString& Url)
{
	static const FLinearColor HyperlinkColor = RGBToSRGB(3, 102, 214);

	return SNew(SButton)
		.VAlign(VAlign_Center)
		.ButtonStyle(&HyperlinkStyle)
		.OnClicked_Lambda([Url]() -> FReply
		{
			FString Errors;
			FPlatformProcess::LaunchURL(*Url, nullptr, &Errors);
			return FReply::Handled();
		})
		.Cursor(EMouseCursor::Hand)
	[
		SNew(STextBlock)
			.ColorAndOpacity(Blurple)
			.Text(FText::FromString(DisplayText))
	];
}

FReply SConfigureSdkWindow::DownloadSdk()
{
	UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK installation requested for platform %s."), *UGameplayStatics::GetPlatformName());

	TSharedPtr<FSlowTask> Task = MakeShared<FSlowTask>(100.f, NSLOCTEXT("DiscordSdkEditor", "DownloadSdk" , "Downloading the Discord SDK."));
	
	Task->Initialize();

	Task->MakeDialog(true);

	FHttpRequestPtr Request = FHttpModule::Get().CreateRequest();
	
	Request->SetURL(DiscordSdkLocation);
	Request->SetVerb(TEXT("GET"));

	{
		TSharedPtr<int32> Downloaded = MakeShared<int32>(0);
		Request->OnRequestProgress().BindLambda([Task, Downloaded](FHttpRequestPtr Req, int32 Sent, int32 Received) -> void
		{
			if (Task->ShouldCancel())
			{
				Req->CancelRequest();
				return;
			}
			const int32 ReceivedSinceLastTick = Received - *Downloaded;
			const float TotalMbReceived = Received / 1024.f / 1024.f;
			const float MbReceived = ReceivedSinceLastTick / 1024. / 1024.f;

			*Downloaded = Received;

			Task->EnterProgressFrame(MbReceived * 2.8f, FText::FromString(FString::Printf(TEXT("Downloading the Discord SDK: %.2f MB downloaded."), TotalMbReceived)));
		});
	}

	TSharedPtr<SWidget> Self = AsShared();

	Request->OnProcessRequestComplete().BindLambda([Self, Task](FHttpRequestPtr Req, FHttpResponsePtr Response, bool bSuccess) -> void
	{
		if (!bSuccess)
		{
			UE_LOG(LogDiscordSdk, Error, TEXT("Failed to download SDK from %s."), *DiscordSdkLocation);
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to install the Discord SDk. Check your internet connection.")));
			Task->Destroy();
			return;
		}

		Task->CompletedWork = 70.f;

		UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK downloaded."));

		SaveSdk((SConfigureSdkWindow*)Self.Get(), Response->GetContent(), Task);

		Task->Destroy();
	});

	if (!Request->ProcessRequest())
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to launch SDK download."));
		Task->Destroy();
	}
	else
	{
		UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK download started."));
	}

	return FReply::Handled();
}

/* static */ void SConfigureSdkWindow::SaveSdk(SConfigureSdkWindow* const Self, const TArray<uint8> Sdk, TSharedPtr<FSlowTask> Task)
{
	FSdkInstaller Installer(Task);

	if (!Installer.SaveSdk(Sdk))
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to save Discord SDK. Canceling installation."));
		return;
	}

	if (!Installer.UnzipFileSync())
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to unzip Discord SDK. Canceling installation."));
		Installer.Error(TEXT("Failed to unzip the SDK. Check the output log for more details."));
		return;
	}

	if (!Installer.CheckFiles())
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to check Discord SDK. Canceling installation."));
		return;		
	}

	if (!Installer.CleanupFiles())
	{
		UE_LOG(LogDiscordSdk, Error, TEXT("Failed to clean up Discord SDK."));
	}

	Installer.Finalize();

	Self->InvalidSdkNotice->SetVisibility(EVisibility::Collapsed);

	UE_LOG(LogDiscordSdk, Log, TEXT("Discord SDK successfully installed."));
}

