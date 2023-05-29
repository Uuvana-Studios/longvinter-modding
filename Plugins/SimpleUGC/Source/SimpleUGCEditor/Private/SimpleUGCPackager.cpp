// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleUGCPackager.h"
#include "SimpleUGCEditor.h"
#include "SimpleUGCEditorCommands.h"
#include "SimpleUGCEditorStyle.h"
#include "Editor.h"
#include "Widgets/SWindow.h"
#include "Widgets/SWidget.h"
#include "Interfaces/IPluginManager.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Editor/UATHelper/Public/IUATHelperModule.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"

#include "FileHelpers.h"
#include "Misc/PackageName.h"

#define LOCTEXT_NAMESPACE "SimpleUGCPackager"

FSimpleUGCPackager::FSimpleUGCPackager()
{
}

FSimpleUGCPackager::~FSimpleUGCPackager()
{
}

void FSimpleUGCPackager::OpenPluginPackager(TSharedRef<IPlugin> Plugin)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	FString DefaultDirectory = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir());
	FString OutputDirectory;

	// Prompt the user to save all dirty packages. We'll ensure that if any packages from the mod that the user wants to
	// package are dirty that they will not be able to save them.

	if (!IsAllContentSaved(Plugin))
	{
		FEditorFileUtils::SaveDirtyPackages( true, true,  true);
	}

	if (IsAllContentSaved(Plugin))
	{
		void* ParentWindowWindowHandle = nullptr;
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		if (DesktopPlatform->OpenDirectoryDialog(ParentWindowWindowHandle, LOCTEXT("SelectOutputFolderTitle", "Select UGC output directory:").ToString(), DefaultDirectory, OutputDirectory))
		{
			PackagePlugin(Plugin, OutputDirectory);
		}
	}
	else
	{
		FText PackageModError = FText::Format(LOCTEXT("PackageUGCError_UnsavedContent", "You must save all assets in {0} before you can share it."),
			FText::FromString(Plugin->GetName()));

		FMessageDialog::Open(EAppMsgType::Ok, PackageModError);
	}
}

bool FSimpleUGCPackager::IsAllContentSaved(TSharedRef<IPlugin> Plugin)
{
	bool bAllContentSaved = true;

	TArray<UPackage*> UnsavedPackages;
	FEditorFileUtils::GetDirtyContentPackages(UnsavedPackages);
	FEditorFileUtils::GetDirtyWorldPackages(UnsavedPackages);

	if (UnsavedPackages.Num() > 0)
	{
		FString PluginBaseDir = Plugin->GetBaseDir();

		for (UPackage* Package : UnsavedPackages)
		{
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename))
			{
				if (PackageFilename.Find(PluginBaseDir) == 0)
				{
					bAllContentSaved = false;
					break;
				}
			}
		}
	}

	return bAllContentSaved;
}

void FSimpleUGCPackager::PackagePlugin(TSharedRef<class IPlugin> Plugin, const FString& OutputDirectory)
{
#if PLATFORM_WINDOWS
	FText PlatformName = LOCTEXT("PlatformName_Windows", "Windows");
#elif PLATFORM_MAC
	FText PlatformName = LOCTEXT("PlatformName_Mac", "Mac");
#elif PLATFORM_LINUX
	FText PlatformName = LOCTEXT("PlatformName_Linux", "Linux");
#else
	FText PlatformName = LOCTEXT("PlatformName_Desktop", "Desktop");
#endif

	// Hard coded here for simplicity. You will probably want to read this from an ini file
	FString ReleaseVersion = TEXT("Longvinter");

	FString CommandLine = FString::Printf(TEXT("PackageUGC -Project=\"%s\" -PluginPath=\"%s\" -basedonreleaseversion=\"%s\" -StagingDirectory=\"%s\" -nocompile"),
		*FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()),
		*FPaths::ConvertRelativePathToFull(Plugin->GetDescriptorFileName()),
		*ReleaseVersion,
		*OutputDirectory);

	FText PackagingText = FText::Format(LOCTEXT("SimpleUGCEditor_PackagePluginTaskName", "Packaging {0}"), FText::FromString(Plugin->GetName()));

	FString FriendlyName = Plugin->GetDescriptor().FriendlyName;
	IUATHelperModule::Get().CreateUatTask(CommandLine, PlatformName, PackagingText,
		PackagingText, FSimpleUGCEditorStyle::Get().GetBrush(TEXT("SimpleUGCEditor.PackageUGCAction")),
		[ReleaseVersion, PlatformName, FriendlyName](FString TaskResult, double TimeSec) {});
}

void FSimpleUGCPackager::FindAvailableGameMods(TArray<TSharedRef<IPlugin>>& OutAvailableGameMods)
{
	OutAvailableGameMods.Empty();

	// Find available game mods from the list of discovered plugins

	for (TSharedRef<IPlugin> Plugin : IPluginManager::Get().GetDiscoveredPlugins())
	{
		// All game project plugins that are marked as mods are valid
		if (Plugin->GetLoadedFrom() == EPluginLoadedFrom::Project && Plugin->GetType() == EPluginType::Mod)
		{
			UE_LOG(LogTemp, Display, TEXT("Adding %s"), *Plugin->GetName());
			OutAvailableGameMods.AddUnique(Plugin);
		}
	}
}

void FSimpleUGCPackager::GeneratePackagerMenuContent_Internal(class FMenuBuilder& MenuBuilder, const TArray<TSharedPtr<FUICommandInfo>>& Commands)
{
	for (TSharedPtr<FUICommandInfo> Command : Commands)
	{
		MenuBuilder.AddMenuEntry(Command, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FSimpleUGCEditorStyle::GetStyleSetName(), "SimpleUGCEditor.Folder"));
	}
}

void FSimpleUGCPackager::GeneratePackagerMenuContent(class FMenuBuilder& MenuBuilder)
{
	TArray<TSharedRef<IPlugin>> AvailableGameMods;
	FindAvailableGameMods(AvailableGameMods);

	TArray<TSharedPtr<FUICommandInfo>> Commands;

	GeneratePackagerMenuContent_Internal(MenuBuilder, UGCCommands);
}

TSharedRef<SWidget> FSimpleUGCPackager::GeneratePackagerComboButtonContent()
{
	// Regenerate the game mod commands
	TArray<TSharedRef<IPlugin>> AvailableGameMods;
	FindAvailableGameMods(AvailableGameMods);

	GetAvailableUGCCommands(AvailableGameMods);

	// Regenerate the action list
	TSharedPtr<FUICommandList> GameModActionsList = MakeShareable(new FUICommandList);

	for (int32 Index = 0; Index < UGCCommands.Num(); ++Index)
	{
		GameModActionsList->MapAction(
			UGCCommands[Index],
			FExecuteAction::CreateRaw(this, &FSimpleUGCPackager::OpenPluginPackager, AvailableGameMods[Index]),
			FCanExecuteAction()
		);
	}

	// Show the drop down menu
	const bool bShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection, GameModActionsList);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("PackageUGC", "Share..."));
	{
		GeneratePackagerMenuContent_Internal(MenuBuilder, UGCCommands);
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FSimpleUGCPackager::GetAvailableUGCCommands(const TArray<TSharedRef<IPlugin>>& AvailableUGC)
{
	if (UGCCommands.Num() > 0)
	{
		// Unregister UI Commands
		FSimpleUGCEditorCommands::Get().UnregisterUGCCommands(UGCCommands);
	}
	UGCCommands.Empty(AvailableUGC.Num());

	UGCCommands = FSimpleUGCEditorCommands::Get().RegisterUGCCommands(AvailableUGC);
}

#undef LOCTEXT_NAMESPACE
