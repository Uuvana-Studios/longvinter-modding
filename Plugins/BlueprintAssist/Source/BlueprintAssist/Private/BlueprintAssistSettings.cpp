// Copyright 2021 fpwong. All Rights Reserved.

#include "BlueprintAssistSettings.h"

#include "BlueprintAssistCache.h"
#include "BlueprintAssistGlobals.h"
#include "BlueprintAssistGraphHandler.h"
#include "BlueprintAssistModule.h"
#include "BlueprintAssistTabHandler.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EdGraphSchema_K2.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"

FBAFormatterSettings::FBAFormatterSettings()
{
	FormatterDirection = EEdGraphPinDirection::EGPD_Output;
}

EBAAutoFormatting FBAFormatterSettings::GetAutoFormatting() const
{
	return GetDefault<UBASettings>()->bGloballyDisableAutoFormatting ? EBAAutoFormatting::Never : AutoFormatting; 
}

UBASettings::UBASettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UseBlueprintFormattingForTheseGraphs =
	{
		"EdGraph",
		"GameplayAbilityGraph",
		"AnimationTransitionGraph",
		"SMStateGraph",
		"SMTransitionGraph"
	};

	ShiftCameraDistance = 400.0f;

	bSaveBlueprintAssistCacheToFile = true;

	bAddToolbarWidget = true;

	SelectedPinHighlightColor = FLinearColor(0.6f, 0.6f, 0.6f, 0.33);

	SupportedAssetEditors = {
		"SoundCueEditor",
		"Niagara",
		"BlueprintEditor",
		"ControlRigEditor",
		"MaterialEditor",
		"MetaSoundEditor",
		"Behavior Tree",
		"AnimationBlueprintEditor",
		"Environment Query",
		"GameplayAbilitiesEditor",
		"FSMBlueprintEditor"
	};

	// ------------------- //
	// Format all settings //
	// ------------------- //

	FormatAllStyle = EBAFormatAllStyle::Simple;
	FormatAllHorizontalAlignment = EBAFormatAllHorizontalAlignment::RootNode;
	bAutoPositionEventNodes = false;
	bAlwaysFormatAll = false;
	FormatAllPadding = FVector2D(800, 250);

	ExecutionWiringStyle = EBAWiringStyle::AlwaysMerge;
	ParameterWiringStyle = EBAWiringStyle::AlwaysMerge;

	bGloballyDisableAutoFormatting = false;
	FormattingStyle = EBANodeFormattingStyle::Expanded;
	ParameterStyle = EBAParameterFormattingStyle::Helixing;

	BlueprintParameterPadding = FVector2D(40, 25);
	BlueprintKnotTrackSpacing = 26.f;
	VerticalPinSpacing = 26.f;
	ParameterVerticalPinSpacing = 26.f;

	bLimitHelixingHeight = true;
	HelixingHeightMax = 500;
	SingleNodeMaxHeight = 300;

	// ------------------ //
	// Formatter Settings //
	// ------------------ //

	BlueprintFormatterSettings.FormatterType = EBAFormatterType::Blueprint;
	BlueprintFormatterSettings.Padding = FVector2D(100, 100); 
	BlueprintFormatterSettings.AutoFormatting = EBAAutoFormatting::FormatAllConnected; 
	BlueprintFormatterSettings.FormatterDirection = EGPD_Output;
	BlueprintFormatterSettings.RootNodes = { "K2Node_Tunnel" }; 
	BlueprintFormatterSettings.ExecPinName = UEdGraphSchema_K2::PC_Exec; 
	BlueprintFormatterSettings.ExecPinName = "exec";

	FBAFormatterSettings BehaviorTreeSettings(
		FVector2D(100, 100),
		EBAAutoFormatting::FormatAllConnected,
		EGPD_Output,
		{ "BehaviorTreeGraphNode_Root" }
	);

	BehaviorTreeSettings.FormatterType = EBAFormatterType::BehaviorTree;

	NonBlueprintFormatterSettings.Add("BehaviorTreeGraph", BehaviorTreeSettings);

	FBAFormatterSettings SoundCueSettings(
		FVector2D(80, 150),
		EBAAutoFormatting::Never,
		EGPD_Input,
		{ "SoundCueGraphNode_Root" }
	);
	NonBlueprintFormatterSettings.Add("SoundCueGraph", SoundCueSettings);

	FBAFormatterSettings MaterialGraphSettings(
		FVector2D(80, 150),
		EBAAutoFormatting::Never,
		EGPD_Output,
		{ "MaterialGraphNode_Root" }
	);
	NonBlueprintFormatterSettings.Add("MaterialGraph", MaterialGraphSettings);

	FBAFormatterSettings AnimGraphSetting;
	AnimGraphSetting.Padding = 	FVector2D(80, 150); 
	AnimGraphSetting.AutoFormatting = EBAAutoFormatting::FormatAllConnected; 
	AnimGraphSetting.FormatterDirection = EGPD_Input;
	AnimGraphSetting.RootNodes = { "AnimGraphNode_Root", "AnimGraphNode_TransitionResult", "AnimGraphNode_StateResult" }; 
	AnimGraphSetting.ExecPinName = "PoseLink"; 
	NonBlueprintFormatterSettings.Add("AnimationGraph", AnimGraphSetting);
	NonBlueprintFormatterSettings.Add("AnimationStateGraph", AnimGraphSetting);

	FBAFormatterSettings NiagaraSettings;
	NiagaraSettings.Padding = FVector2D(80, 150);
	NiagaraSettings.AutoFormatting = EBAAutoFormatting::FormatAllConnected;
	NiagaraSettings.FormatterDirection = EGPD_Input;
	NiagaraSettings.RootNodes = { "NiagaraNodeInput" };
	NiagaraSettings.ExecPinName = "NiagaraParameterMap";

#if BA_UE_VERSION_OR_LATER(5, 0)
	NonBlueprintFormatterSettings.Add("NiagaraGraph", NiagaraSettings);
#else
	NonBlueprintFormatterSettings.Add("NiagaraGraphEditor", NiagaraSettings);
#endif

	// TODO: Reenable support for control rig after fixing issues
	FBAFormatterSettings ControlRigSettings;
#if BA_UE_VERSION_OR_LATER(5, 0)
	ControlRigSettings.bEnabled = true;
#else
	ControlRigSettings.bEnabled = false;
#endif
	ControlRigSettings.Padding = FVector2D(80, 150);
	ControlRigSettings.AutoFormatting = EBAAutoFormatting::FormatAllConnected;
	ControlRigSettings.FormatterDirection = EGPD_Output;
	ControlRigSettings.ExecPinName = "ControlRigExecuteContext";
	NonBlueprintFormatterSettings.Add("ControlRigGraph", ControlRigSettings);

	FBAFormatterSettings MetaSoundSettings(
		FVector2D(80, 150),
		EBAAutoFormatting::FormatAllConnected,
		EGPD_Output,
		{ "MetasoundEditorGraphInputNode" }
	);
	NonBlueprintFormatterSettings.Add("MetasoundEditorGraph", MetaSoundSettings);

	FBAFormatterSettings EnvironmentQuerySettings;
	EnvironmentQuerySettings.FormatterType = EBAFormatterType::BehaviorTree;
	EnvironmentQuerySettings.Padding = FVector2D(80, 150);
	EnvironmentQuerySettings.AutoFormatting = EBAAutoFormatting::FormatAllConnected;
	EnvironmentQuerySettings.FormatterDirection = EGPD_Output;
	EnvironmentQuerySettings.RootNodes = { "EnvironmentQueryGraphNode_Root" };
	NonBlueprintFormatterSettings.Add("EnvironmentQueryGraph", EnvironmentQuerySettings);

	FBAFormatterSettings LogicDriverStateMachineGraphK2Settings;
	LogicDriverStateMachineGraphK2Settings.FormatterType = EBAFormatterType::Simple;
	LogicDriverStateMachineGraphK2Settings.Padding = FVector2D(80, 150);
	LogicDriverStateMachineGraphK2Settings.AutoFormatting = EBAAutoFormatting::FormatAllConnected;
	LogicDriverStateMachineGraphK2Settings.FormatterDirection = EGPD_Input;
	LogicDriverStateMachineGraphK2Settings.RootNodes = { "SMGraphK2Node_StateMachineSelectNode" };
	NonBlueprintFormatterSettings.Add("SMGraphK2", LogicDriverStateMachineGraphK2Settings);

	bCreateKnotNodes = true;

	bBetterWiringForNewNodes = true;
	bAutoAddParentNode = true;

	bAutoRenameGettersAndSetters = true;
	bMergeGenerateGetterAndSetterButton = false;

	bEnableGlobalCommentBubblePinned = false;
	bGlobalCommentBubblePinnedValue = true;

	bDetectNewNodesAndCacheNodeSizes = false;
	bRefreshNodeSizeBeforeFormatting = true;

	bTreatDelegatesAsExecutionPins = false;

	bCenterBranches = false;
	NumRequiredBranches = 3;

	bCenterBranchesForParameters = false;
	NumRequiredBranchesForParameters = 2;

	bAddKnotNodesToComments = true;
	CommentNodePadding = FVector2D(30, 30);

	bEnableFasterFormatting = false;

	bUseKnotNodePool = false;

	bSlowButAccurateSizeCaching = false;

	bApplyCommentPadding = false;

	KnotNodeDistanceThreshold = 800.f;

	bExpandNodesAheadOfParameters = true;
	bExpandNodesByHeight = true;
	bExpandParametersByHeight = false;

	bSnapToGrid = false;
	bAlignExecNodesTo8x8Grid = false;

	// ------------------------ //
	// Create variable defaults //
	// ------------------------ //

	bEnableVariableDefaults = false;
	bDefaultVariableInstanceEditable = false;
	bDefaultVariableBlueprintReadOnly = false;
	bDefaultVariableExposeOnSpawn = false;
	bDefaultVariablePrivate = false;
	bDefaultVariableExposeToCinematics = false;
	DefaultVariableName = TEXT("VarName");
	DefaultVariableTooltip = FText::FromString(TEXT(""));
	DefaultVariableCategory = FText::FromString(TEXT(""));

	// ----------------- //
	// Function defaults //
	// ----------------- //

	bEnableFunctionDefaults = false;
	DefaultFunctionAccessSpecifier = EBAFunctionAccessSpecifier::Public;
	bDefaultFunctionPure = false;
	bDefaultFunctionConst = false;
	bDefaultFunctionExec = false;
	DefaultFunctionTooltip = FText::FromString(TEXT(""));
	DefaultFunctionKeywords = FText::FromString(TEXT(""));
	DefaultFunctionCategory = FText::FromString(TEXT(""));

	// ------------------------ //
	// Misc                     //
	// ------------------------ //

	DefaultGeneratedGettersCategory = INVTEXT("Generated|Getters");
	DefaultGeneratedSettersCategory = INVTEXT("Generated|Setters");
	bEnableDoubleClickGoToDefinition = true;
	bPlayLiveCompileSound = false;
	bEnableInvisibleKnotNodes = false;
	bHighlightBadComments = false;
	GroupMovementChords.Add(FInputChord(EKeys::LeftShift));
	FolderBookmarks.Add(EKeys::One);
	FolderBookmarks.Add(EKeys::Two);
	FolderBookmarks.Add(EKeys::Three);
	FolderBookmarks.Add(EKeys::Four);
	FolderBookmarks.Add(EKeys::Five);
	FolderBookmarks.Add(EKeys::Six);
	FolderBookmarks.Add(EKeys::Seven);
	FolderBookmarks.Add(EKeys::Eight);
	FolderBookmarks.Add(EKeys::Nine);
	FolderBookmarks.Add(EKeys::Zero);
	ClickTime = 0.35f;

	// ------------------------ //
	// Accessibility            //
	// ------------------------ //

	bShowOverlayWhenCachingNodes = true;
	RequiredNodesToShowOverlayProgressBar = 15;
}

void UBASettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	check(FBlueprintAssistModule::IsAvailable())

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	TSharedPtr<FBAGraphHandler> GraphHandler = FBATabHandler::Get().GetActiveGraphHandler();
	if (GraphHandler.IsValid())
	{
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bEnableGlobalCommentBubblePinned) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bGlobalCommentBubblePinnedValue))
		{
			GraphHandler->ApplyGlobalCommentBubblePinned();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, ParameterStyle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, FormattingStyle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, ParameterWiringStyle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, ExecutionWiringStyle)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bLimitHelixingHeight)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, HelixingHeightMax)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, SingleNodeMaxHeight)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, BlueprintKnotTrackSpacing)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, BlueprintParameterPadding)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, FormatAllPadding)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bTreatDelegatesAsExecutionPins)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bExpandNodesByHeight)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bExpandParametersByHeight)
			|| PropertyName == GET_MEMBER_NAME_CHECKED(UBASettings, bCreateKnotNodes)
			|| PropertyName == NAME_None) // if the name is none, this probably means we changed a property through the toolbar
			// TODO: maybe there's a way to change property externally while passing in correct info name
		{
			GraphHandler->ClearFormatters();
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

TSharedRef<IDetailCustomization> FBASettingsDetails::MakeInstance()
{
	return MakeShareable(new FBASettingsDetails);
}

void FBASettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<FName> CategoryOrder = {
		"General",
		"FormattingOptions",
		"FormatAll",
		"BlueprintFormatting",
		"OtherGraphs",
		"CommentSettings",
		"Misc",
		"Accessibility",
		"NewFunctionDefaults",
		"NewVariableDefaults",
	};

	for (int i = 0; i < CategoryOrder.Num(); ++i)
	{
		DetailBuilder.EditCategory(CategoryOrder[i]).SetSortOrder(i);
	}


	static TArray<FName> DefaultCollapsedCategories = { "OtherGraphs", "NewVariableDefaults", "NewFunctionDefaults" };
	for (FName& CategoryName : DefaultCollapsedCategories)
	{
		DetailBuilder.EditCategory(CategoryName).InitiallyCollapsed(true);
	}

	//--------------------
	// General
	// -------------------

	IDetailCategoryBuilder& MiscCategory = DetailBuilder.EditCategory("Misc");
	auto& BACache = FBACache::Get();

	const FString CachePath = BACache.GetCachePath(true);

	const auto DeleteSizeCache = [&BACache]()
	{
		static FText Title = FText::FromString("Delete cache file");
		static FText Message = FText::FromString("Are you sure you want to delete the cache file?");

		const EAppReturnType::Type Result = FMessageDialog::Open(EAppMsgType::YesNo, Message, &Title);
		if (Result == EAppReturnType::Yes)
		{
			BACache.DeleteCache();
		}

		return FReply::Handled();
	};

	MiscCategory.AddCustomRow(FText::FromString("Delete cache file"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Delete cache file"))
			.Font(BA_GET_FONT_STYLE(TEXT("PropertyWindow.NormalFont")))
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().Padding(5).AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Delete cache file"))
				.ToolTipText(FText::FromString(FString::Printf(TEXT("Delete cache file located at: %s"), *CachePath)))
				.OnClicked_Lambda(DeleteSizeCache)
			]
		];
}

FBAFormatterSettings UBASettings::GetFormatterSettings(UEdGraph* Graph)
{
	if (FBAFormatterSettings* Settings = FindFormatterSettings(Graph))
	{
		return *Settings;
	}

	return FBAFormatterSettings();
}

FBAFormatterSettings* UBASettings::FindFormatterSettings(UEdGraph* Graph)
{
	UBASettings* BASettings = GetMutableDefault<UBASettings>();

	if (FBAFormatterSettings* FoundSettings = BASettings->NonBlueprintFormatterSettings.Find(Graph->GetClass()->GetFName()))
	{
		if (FoundSettings->bEnabled)
		{
			return FoundSettings;
		}
	}

	if (FBAUtils::IsBlueprintGraph(Graph, false))
	{
		return &BASettings->BlueprintFormatterSettings;
	}

	return nullptr; 
}