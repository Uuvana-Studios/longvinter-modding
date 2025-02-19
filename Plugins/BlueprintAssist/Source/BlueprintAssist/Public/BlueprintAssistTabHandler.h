// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

struct FSlateDebuggingFocusEventArgs;
class FBAGraphHandler;
class SDockTab;
class SWindow;
class SGraphEditor;
class SWidget;

/**
 * Manages tabs and their associated GraphHandlers
 */
class BLUEPRINTASSIST_API FBATabHandler
{
public:
	static FBATabHandler& Get();
	static void TearDown();

	void Init();

	~FBATabHandler();

	void Tick(float DeltaTime);

	void Cleanup();

	TSharedPtr<FBAGraphHandler> GetActiveGraphHandler();

	TSharedPtr<SDockTab> GetLastMajorTab();

	TWeakPtr<SGraphEditor> GetUnsupportedGraphEditor() const { return UnsupportedGraphEditor; }

	bool ProcessTab(TSharedPtr<SDockTab> Tab);

private:
	TWeakPtr<FBAGraphHandler> ActiveGraphHandler;
	TMap<TWeakPtr<SDockTab>, TSharedRef<FBAGraphHandler>> GraphHandlerMap;

	TWeakPtr<SDockTab> LastMajorTab;

	FDelegateHandle OnTabForegroundedDelegateHandle;
	FDelegateHandle OnActiveTabChangedDelegateHandle;

	FTimerHandle ProcessTabsTimerHandle;
	TSet<TWeakPtr<SDockTab>> TabsToProcess;

	TWeakPtr<SGraphEditor> UnsupportedGraphEditor;

	TWeakPtr<SWindow> LastActiveWindow;

	void CheckWindowFocusChanged();

	void SetGraphHandler(TSharedPtr<SDockTab> Tab, TSharedPtr<SGraphEditor> GraphEditor);

	void OnTabForegrounded(TSharedPtr<SDockTab> NewTab, TSharedPtr<SDockTab> PreviousTab);

	void OnActiveTabChanged(TSharedPtr<SDockTab> PreviousTab, TSharedPtr<SDockTab> NewTab);

	void RemoveInvalidTabs();

	TSharedPtr<SDockTab> GetChildTabWithGraphEditor(TSharedPtr<SWidget> Widget) const;

	void ProcessTabs();
};
