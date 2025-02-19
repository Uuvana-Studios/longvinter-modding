// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistTabHandler.h"

#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistUtils.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/LazySingleton.h"
#include "Widgets/Docking/SDockTab.h"

FBATabHandler& FBATabHandler::Get()
{
	return TLazySingleton<FBATabHandler>::Get();
}

void FBATabHandler::TearDown()
{
	TLazySingleton<FBATabHandler>::TearDown();
}

void FBATabHandler::Init()
{
	if (FSlateApplication::IsInitialized())
	{
		TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();

		// Add delegate for active tab changed
		const auto& ActiveTabChangedDelegate = FOnActiveTabChanged::FDelegate::CreateRaw(this, &FBATabHandler::OnActiveTabChanged);
		OnActiveTabChangedDelegateHandle = TabManager->OnActiveTabChanged_Subscribe(ActiveTabChangedDelegate);

		// Add delegate for tab foregrounded
		const auto& TabForegroundedDelegate = FOnActiveTabChanged::FDelegate::CreateRaw(this, &FBATabHandler::OnTabForegrounded);
		OnTabForegroundedDelegateHandle = TabManager->OnTabForegrounded_Subscribe(TabForegroundedDelegate);
	}
	else
	{
		UE_LOG(LogBlueprintAssist, Error, TEXT("FBlueprintAssistTabHandler::Constructor: SlateApp is not initialized"));
	}
}

void FBATabHandler::Tick(const float DeltaTime)
{
	RemoveInvalidTabs();

	CheckWindowFocusChanged();

	if (ActiveGraphHandler.IsValid())
	{
		ActiveGraphHandler.Pin()->Tick(DeltaTime);
	}
}

FBATabHandler::~FBATabHandler()
{
	ActiveGraphHandler.Reset();
	GraphHandlerMap.Empty();
	TabsToProcess.Empty();
	LastMajorTab.Reset();
}

void FBATabHandler::OnTabForegrounded(TSharedPtr<SDockTab> NewTab, TSharedPtr<SDockTab> PreviousTab)
{
	if (!NewTab.IsValid())
	{
		return;
	}

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Tab foregrounded: %s %d"), *NewTab->GetTabLabel().ToString(), NewTab->GetTabRole());

	TWeakPtr<SDockTab> NewTabObserver(NewTab);
	TabsToProcess.Emplace(NewTabObserver);

	if (!ProcessTabsTimerHandle.IsValid())
	{
		ProcessTabsTimerHandle = GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FBATabHandler::ProcessTabs));
	}
}

void FBATabHandler::OnActiveTabChanged(TSharedPtr<SDockTab> PreviousTab, TSharedPtr<SDockTab> NewTab)
{
	if (!NewTab.IsValid())
	{
		return;
	}

	// UE_LOG(LogBlueprintAssist, Warning, TEXT("Active tab changed: %s %d"), *NewTab->GetTabLabel().ToString(), NewTab->GetTabRole());

	TWeakPtr<SDockTab> NewTabObserver(NewTab);
	TabsToProcess.Emplace(NewTabObserver);

	if (!ProcessTabsTimerHandle.IsValid())
	{
		ProcessTabsTimerHandle = GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FBATabHandler::ProcessTabs));
	}
}

TSharedPtr<FBAGraphHandler> FBATabHandler::GetActiveGraphHandler()
{
	return ActiveGraphHandler.Pin();
}

TSharedPtr<SDockTab> FBATabHandler::GetLastMajorTab()
{
	return LastMajorTab.Pin();
}

void FBATabHandler::CheckWindowFocusChanged()
{
	// Changing window focus does not always activate the tab delegates
	// So we check when the window changed and process the current active tab
	TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	if (ActiveWindow != LastActiveWindow)
	{
		LastActiveWindow = ActiveWindow;

		if (TSharedPtr<SDockTab> ActiveTab = FGlobalTabmanager::Get()->GetActiveTab())
		{
			TSharedPtr<SWindow> ParentWindow = FBAUtils::GetParentWindow(ActiveTab);
			if (ParentWindow == ActiveWindow)
			{
				ProcessTab(ActiveTab);
			}
		}
	}
}

void FBATabHandler::SetGraphHandler(TSharedPtr<SDockTab> Tab, TSharedPtr<SGraphEditor> GraphEditor)
{
	TWeakPtr<SGraphEditor> GraphEditorObserver(GraphEditor);
	TWeakPtr<SDockTab> TabObserver(Tab);

	if (ActiveGraphHandler.IsValid())
	{
		ActiveGraphHandler.Pin()->ResetTransactions();
		ActiveGraphHandler.Pin()->OnLoseFocus();
	}

	if (GraphHandlerMap.Contains(Tab))
	{
		ActiveGraphHandler = GraphHandlerMap[Tab];
		check(ActiveGraphHandler.IsValid());
		ActiveGraphHandler.Pin()->OnGainFocus();
	}
	else
	{
		TSharedRef<FBAGraphHandler> NewGraphHandler(MakeShared<FBAGraphHandler>(TabObserver, GraphEditorObserver));
		NewGraphHandler->InitGraphHandler();
		GraphHandlerMap.Add(Tab, NewGraphHandler);
		ActiveGraphHandler = NewGraphHandler;
	}
}

void FBATabHandler::Cleanup()
{
	TSharedRef<FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
	TabManager->OnTabForegrounded_Unsubscribe(OnTabForegroundedDelegateHandle);
	TabManager->OnActiveTabChanged_Unsubscribe(OnActiveTabChangedDelegateHandle);

	for (auto& Elem : GraphHandlerMap)
	{
		Elem.Value->Cleanup();
	}

	GraphHandlerMap.Reset();
	ActiveGraphHandler.Reset();
	TabsToProcess.Reset();
	LastMajorTab.Reset();
	ProcessTabsTimerHandle.Invalidate();
}

void FBATabHandler::RemoveInvalidTabs()
{
	TArray<TWeakPtr<SDockTab>> InvalidTabs;
	for (auto& Elem : GraphHandlerMap)
	{
		TWeakPtr<SDockTab> Tab = Elem.Key;
		TSharedRef<FBAGraphHandler> GraphHandler = Elem.Value;

		if (!Tab.IsValid() || !FBAUtils::IsValidGraph(GraphHandler->GetFocusedEdGraph()))
		{
			InvalidTabs.Add(Tab);
			GraphHandler->Cleanup();

			if (ActiveGraphHandler == GraphHandler)
			{
				ActiveGraphHandler.Reset();
				ActiveGraphHandler = nullptr;
			}
		}
	}

	for (auto& Tab : InvalidTabs)
	{
		if (GraphHandlerMap.Contains(Tab))
		{
			GraphHandlerMap.Remove(Tab);
		}
	}
}

TSharedPtr<SDockTab> FBATabHandler::GetChildTabWithGraphEditor(TSharedPtr<SWidget> Widget) const
{
	if (!Widget.IsValid())
	{
		return nullptr;
	}

	if (Widget->GetVisibility() == EVisibility::Hidden || Widget->GetVisibility() == EVisibility::Collapsed)
	{
		return nullptr;
	}

	// check if widget is dock tab
	if (TSharedPtr<SDockTab> ChildDockTab = CAST_SLATE_WIDGET(Widget, SDockTab))
	{
		if (ChildDockTab->IsForeground())
		{
			TSharedPtr<SWidget> TabContent = ChildDockTab->GetContent();
			if (FBAUtils::IsWidgetOfType(TabContent, "SGraphEditor"))
			{
				return ChildDockTab;
			}
		}
	}
	else // recursively check children
	{
		if (FChildren* const Children = Widget->GetChildren())
		{
			for (int i = 0; i < Children->Num(); i++)
			{
				TSharedPtr<SDockTab> ReturnWidget = GetChildTabWithGraphEditor(Children->GetChildAt(i));
				if (ReturnWidget.IsValid())
				{
					return ReturnWidget;
				}
			}
		}
	}

	return nullptr;
}

void FBATabHandler::ProcessTabs()
{
	ProcessTabsTimerHandle.Invalidate();

	if (TabsToProcess.Num() == 0)
	{
		return;
	}

	for (const TWeakPtr<SDockTab>& Tab : TabsToProcess)
	{
		if (Tab.Pin())
		{
			if (ProcessTab(Tab.Pin()))
			{
				break;
			}
		}
	}

	TabsToProcess.Empty();
}

bool FBATabHandler::ProcessTab(TSharedPtr<SDockTab> Tab)
{
	if (!Tab.IsValid() || !Tab->IsForeground())
	{
		return false;
	}

	const bool bIsMajorTab = Tab->GetTabRole() == MajorTab;

	TSharedRef<SWidget> Widget = Tab->GetContent();

	if (bIsMajorTab)
	{
		LastMajorTab = Tab;
	}
	// grab the major tab from the owner
	else
	{
		TSharedPtr<SDockTab> OwnerTab = nullptr;

#if BA_UE_VERSION_OR_LATER(5, 0)
		if (TSharedPtr<FTabManager> TabManager = Tab->GetTabManagerPtr())
		{
			OwnerTab = TabManager->GetOwnerTab();
		}
#else
		OwnerTab = Tab->GetTabManager()->GetOwnerTab();
#endif

		if (OwnerTab.IsValid() && OwnerTab->GetTabRole() == MajorTab)
		{
			LastMajorTab = OwnerTab;
		}
	}

	TSharedPtr<SDockTab> TabWithGraphEditor
		= bIsMajorTab
		? GetChildTabWithGraphEditor(Tab->GetContent())
		: Tab;

	UnsupportedGraphEditor.Reset();
	if (TabWithGraphEditor.IsValid())
	{
		TSharedRef<SWidget> TabContent = TabWithGraphEditor->GetContent();
		TSharedPtr<SGraphEditor> GraphEditor = FBAUtils::GetChildWidgetCasted<SGraphEditor>(TabContent, "SGraphEditor");

		// use the tab if it contains a graph editor
		if (GraphEditor.IsValid())
		{
			if (FBAUtils::IsValidGraph(GraphEditor->GetCurrentGraph()))
			{
				SetGraphHandler(TabWithGraphEditor, GraphEditor);
				return true;
			}
			else
			{
				UnsupportedGraphEditor = GraphEditor;
			}
		}
	}

	// if we have reached this point, then the processed tab is not suitable for a graph handler
	// set our active graph handler to null
	if (ActiveGraphHandler.IsValid())
	{
		const bool bDifferentWindow = FBAUtils::GetParentWindow(Tab) != ActiveGraphHandler.Pin()->GetWindow();

		if (bDifferentWindow || Tab->GetTabRole() != PanelTab)
		{
			ActiveGraphHandler.Pin()->ResetTransactions();
			ActiveGraphHandler.Pin()->OnLoseFocus();
			ActiveGraphHandler = nullptr;
		}
	}

	return false;
}
