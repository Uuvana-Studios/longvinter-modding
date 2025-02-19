// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "BlueprintAssistSettings.h"
#include "Framework/Commands/Commands.h"

class FAssetEditorToolkit;
class SDockTab;
class FToolBarBuilder;
class FMenuBuilder;
class FExtender;
enum class ECheckBoxState : unsigned char;
class SWidget;
class FUICommandList;

class FBAToolbarCommandsImpl final : public TCommands<FBAToolbarCommandsImpl>
{
public:
	FBAToolbarCommandsImpl();

	virtual ~FBAToolbarCommandsImpl() override { }

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> AutoFormatting_Never;
	TSharedPtr<FUICommandInfo> AutoFormatting_FormatNewlyCreated;
	TSharedPtr<FUICommandInfo> AutoFormatting_FormatAll;

	TSharedPtr<FUICommandInfo> FormattingStyle_Compact;
	TSharedPtr<FUICommandInfo> FormattingStyle_Expanded;

	TSharedPtr<FUICommandInfo> ParameterStyle_LeftHandSide;
	TSharedPtr<FUICommandInfo> ParameterStyle_Helixing;

	TSharedPtr<FUICommandInfo> FormatAllStyle_Simple;
	TSharedPtr<FUICommandInfo> FormatAllStyle_Smart;
	TSharedPtr<FUICommandInfo> FormatAllStyle_NodeType;

	TSharedPtr<FUICommandInfo> BlueprintAssistSettings;
};

class BLUEPRINTASSIST_API FBAToolbarCommands
{
public:
	static void Register();

	static const FBAToolbarCommandsImpl& Get();

	static void Unregister();
};

class FBAToolbar
{
public:
	TSharedPtr<FUICommandList> BlueprintAssistToolbarActions;

	TMap<TWeakPtr<FAssetEditorToolkit>, TSharedPtr<FExtender>> ToolbarExtenderMap;

	static FBAToolbar& Get();
	static void TearDown();

	void Init();

	void Cleanup();

	void OnAssetOpenedInEditor(UObject* Asset, class IAssetEditorInstance* AssetEditor);

	static void SetAutoFormattingStyle(EBAAutoFormatting FormattingStyle);
	static bool IsAutoFormattingStyleChecked(EBAAutoFormatting FormattingStyle);

	static void SetParameterStyle(EBAParameterFormattingStyle Style);
	static bool IsParameterStyleChecked(EBAParameterFormattingStyle Style);

	static void SetNodeFormattingStyle(EBANodeFormattingStyle Style);
	static bool IsNodeFormattingStyleChecked(EBANodeFormattingStyle Style);

	static void SetFormatAllStyle(EBAFormatAllStyle Style);
	static bool IsFormatAllStyleChecked(EBAFormatAllStyle Style);

	void SetUseCommentBoxPadding(ECheckBoxState NewCheckedState);
	void SetGraphReadOnly(ECheckBoxState NewCheckedState);

	static void OpenBlueprintAssistSettings();

	void BindToolbarCommands();

	TSharedRef<SWidget> CreateToolbarWidget();

	void MakeAutoFormattingSubMenu(FMenuBuilder& InMenuBuilder);
	void MakeParameterStyleSubMenu(FMenuBuilder& InMenuBuilder);
	void MakeFormattingStyleSubMenu(FMenuBuilder& InMenuBuilder);
	void MakeFormatAllStyleSubMenu(FMenuBuilder& InMenuBuilder);

	void ExtendToolbarAndProcessTab(FToolBarBuilder& ToolbarBuilder, TWeakPtr<SDockTab> Tab);
	void ExtendToolbar(FToolBarBuilder& ToolbarBuilder);

	static FBAFormatterSettings* GetCurrentFormatterSettings();
};
