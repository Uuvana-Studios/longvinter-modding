// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistBlueprintActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistGlobalActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistGraphActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistNodeActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistPinActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistTabActions.h"
#include "BlueprintAssist/BlueprintAssistActions/BlueprintAssistToolkitActions.h"
#include "Framework/Application/IInputProcessor.h"

class UEdGraphNode;

class BLUEPRINTASSIST_API FBAInputProcessor final
	: public TSharedFromThis<FBAInputProcessor>
	, public IInputProcessor
{
public:
	virtual ~FBAInputProcessor() override;

	static void Create();

	static FBAInputProcessor& Get();

	void Cleanup();

	//~ Begin IInputProcessor Interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	bool OnMouseDrag(FSlateApplication& SlateApp, const FVector2D& MousePos, const FVector2D& Delta);

	bool OnKeyOrMouseDown(FSlateApplication& SlateApp, const FKey& Key);
	bool OnKeyOrMouseUp(FSlateApplication& SlateApp, const FKey& Key);
	//~ End IInputProcessor Interface

	void HandleSlateInputEvent(const FSlateDebuggingInputEventArgs& EventArgs);

	FVector2D LastMousePos;

	/* Anchor node for usage in group movement */ 
	TWeakObjectPtr<UEdGraphNode> AnchorNode;
	FVector2D LastAnchorPos;

	bool bIsDisabled = false;

	bool CanExecuteCommand(TSharedRef<const FUICommandInfo> Command) const;
	bool TryExecuteCommand(TSharedRef<const FUICommandInfo> Command);

	bool IsDisabled() const;

	void UpdateGroupMovement();
	void GroupMoveSelectedNodes(const FVector2D& Delta);

	TSet<FKey> KeysDown;
	TMap<FKey, double> KeysDownStartTime; 

private:
	FBAGlobalActions GlobalActions;
	FBATabActions TabActions;
	FBAToolkitActions ToolkitActions;
	FBAGraphActions GraphActions;
	FBANodeActions NodeActions;
	FBAPinActions PinActions;
	FBABlueprintActions BlueprintActions;
	TArray<TSharedPtr<FUICommandList>> CommandLists;

	TArray<FAssetData> CutAssets;

	FBAInputProcessor();

#if ENGINE_MINOR_VERSION >= 26 || ENGINE_MAJOR_VERSION >= 5
	virtual const TCHAR* GetDebugName() const override { return TEXT("BlueprintAssistInputProcessor"); }
#endif

	bool IsInputChordDown(const FInputChord& Chord);

	bool IsAnyInputChordDown(const TArray<FInputChord>& Chords);

	bool IsInputChordDown(const FInputChord& Chord, const FKey Key);

	bool IsAnyInputChordDown(const TArray<FInputChord>& Chords, const FKey Key);

	double GetKeyDownDuration(const FKey Key);

	bool ProcessFolderBookmarkInput();

	bool ProcessContentBrowserInput();
};
