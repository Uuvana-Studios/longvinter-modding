// Copyright 2021 fpwong. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintAssistDelayedDelegate.h"
#include "BlueprintAssistNodeSizeChangeData.h"
#include "BlueprintAssist/GraphFormatters/GraphFormatterTypes.h"

class SBlueprintAssistGraphOverlay;
class SMyBlueprint;
class FBANodeSizeChangeData;
struct FFormatterInterface;
struct FBAGraphData;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNodeFormatted, UEdGraphNode*, const FFormatterInterface&);

class BLUEPRINTASSIST_API FBAGraphHandler
	: public TSharedFromThis<FBAGraphHandler>
{
public:
	UEdGraphNode* NodeToReplace = nullptr;

	FOnNodeFormatted OnNodeFormatted;

	FBAGraphHandler(TWeakPtr<SDockTab> InTab, TWeakPtr<SGraphEditor> InGraphEditor);

	~FBAGraphHandler();

	void InitGraphHandler();

	void AddGraphPanelOverlay();

	void OnGainFocus();

	void OnLoseFocus();

	void Cleanup();

	void Tick(float DeltaTime);

	void UpdateSelectedNode();

	TSharedPtr<SWindow> GetWindow();

	bool IsWindowActive();

	bool IsGraphReadOnly();

	bool TryAutoFormatNode(UEdGraphNode* Node, TSharedPtr<FScopedTransaction> PendingTransaction = TSharedPtr<FScopedTransaction>(), FEdGraphFormatterParameters Parameters = FEdGraphFormatterParameters());

	void AddPendingFormatNodes(
		UEdGraphNode* Node,
		TSharedPtr<FScopedTransaction> PendingTransaction = TSharedPtr<FScopedTransaction>(),
		FEdGraphFormatterParameters FormatterParameters = FEdGraphFormatterParameters());

	void SetReplaceNewNodeTransaction(TSharedPtr<FScopedTransaction> Transaction)
	{
		ReplaceNewNodeTransaction = Transaction;
	}

	void ResetSingleNewNodeTransaction();

	void ResetReplaceNodeTransaction();

	float GetPinY(const UEdGraphPin* Pin);

	void UpdateCachedNodeSize(float DeltaTime);

	void UpdateNodesRequiringFormatting();

	void SimpleFormatAll();

	void SmartFormatAll();

	void SetSelectedPin(UEdGraphPin* Pin, bool bLerpIntoView = false);

	void UpdateLerpViewport(float DeltaTime);

	void BeginLerpViewport(FVector2D TargetView, bool bCenter = true);

	TSharedPtr<FFormatterInterface> FormatNodes(UEdGraphNode* Node, bool bUsingFormatAll = false);

	void CancelProcessingNodeSizes();

	void CancelCachingNotification();

	void CancelFormattingNodes();

	void CancelSizeTimeoutNotification();

	TSharedPtr<SDockTab> GetTab() const { return CachedTab.Pin(); }

	UEdGraph* GetFocusedEdGraph();

	TSharedPtr<SGraphEditor> GetGraphEditor();

	TSharedPtr<SGraphPanel> GetGraphPanel();

	UBlueprint* GetBlueprint();

	UEdGraphNode* GetSelectedNode(bool bAllowCommentNodes = false);

	TSet<UEdGraphNode*> GetSelectedNodes(bool bAllowCommentNodes = false);

	FSlateRect GetCachedNodeBounds(UEdGraphNode* Node, bool bWithCommentBubble = true);

	UEdGraphPin* GetSelectedPin();

	TSharedPtr<SGraphNode> GetGraphNode(UEdGraphNode* Node);

	bool IsCalculatingNodeSize() const { return PendingSize.Num() > 0; }

	void RefreshNodeSize(UEdGraphNode* Node);

	void RefreshAllNodeSizes();

	void ResetTransactions();

	void FormatAllEvents();

	void ApplyGlobalCommentBubblePinned();

	void ApplyCommentBubblePinned(UEdGraphNode* Node);

	int32 GetNumberOfPendingNodesToCache() const;

	float GetPendingNodeSizeProgress() const;

	void ClearCache();

	void ClearFormatters();

	bool FilterSelectiveFormatting(UEdGraphNode* Node, const TArray<UEdGraphNode*>& NodesToFormat);

	bool FilterDelegatePin(const FPinLink& PinLink, const TArray<UEdGraphNode*>& NodesToFormat);

	UEdGraphNode* GetRootNode(UEdGraphNode* InitialNode, const TArray<UEdGraphNode*>& NodesToFormat, bool bCheckSelectedNode = true);

	TSharedPtr<FFormatterInterface> MakeFormatter();

	bool HasActiveTransaction() const;

	void SelectNode(UEdGraphNode* Node, bool bLerpIntoView = true);

	void LerpNodeIntoView(UEdGraphNode* Node, bool bOnlyWhenOffscreen);

	TSharedPtr<SBlueprintAssistGraphOverlay> GetGraphOverlay() { return GraphOverlay; }

	void PreFormatting();

	void PostFormatting(TArray<TSharedPtr<FFormatterInterface>> Formatters);

private:
	TSharedPtr<SBlueprintAssistGraphOverlay> GraphOverlay;

	TWeakPtr<SGraphPanel> CachedGraphPanel;
	TWeakPtr<SGraphEditor> CachedGraphEditor;
	TWeakPtr<SDockTab> CachedTab;

	TWeakObjectPtr<UEdGraph> CachedEdGraph;

	FEdGraphFormatterParameters FormatterParameters;

	FBAGraphPinHandle SelectedPinHandle;

	FBAGraphData& GetGraphData();

	FBADelayedDelegate DelayedGraphInitialized;
	FBADelayedDelegate DelayedViewportZoomIn;
	FBADelayedDelegate DelayedClearReplaceTransaction;
	FBADelayedDelegate DelayedDetectGraphChanges;

	FBADelayedDelegate DelayedCacheSizeTimeout;
	FBADelayedDelegate DelayedCacheSizeFinished;

	bool bInitialZoomFinished;
	FVector2D LastGraphView;
	float LastZoom;

	// update node size
	float NodeSizeTimeout;
	TSet<UEdGraphNode*> PendingFormatting;
	UEdGraphNode* FocusedNode;
	bool bFullyZoomed;
	FVector2D ViewCache;
	float ZoomCache;

	bool bDeferredGraphChanged;

	TMap<UEdGraphNode*, FVector2D> CommentBubbleSizeCache;

	UEdGraphNode* LastSelectedNode;

	// lerp viewport position
	bool bLerpViewport;
	bool bCenterWhileLerping;
	FVector2D TargetLerpLocation;

	int32 InitialPendingSize = 0;
	TArray<UEdGraphNode*> PendingSize;

	TArray<TArray<UEdGraphNode*>> FormatAllColumns;
	TMap<UEdGraphNode*, TSharedPtr<FFormatterInterface>> FormatterMap;

	TSharedPtr<FScopedTransaction> PendingTransaction;
	TSharedPtr<FScopedTransaction> ReplaceNewNodeTransaction;
	TSharedPtr<FScopedTransaction> FormatAllTransaction;

	TArray<UEdGraphNode*> LastNodes;

	FDelegateHandle OnGraphChangedHandle;

	TWeakPtr<SNotificationItem> CachingNotification;
	TWeakPtr<SNotificationItem> SizeTimeoutNotification;

	void OnGraphInitializedDelayed();

	TMap<FGuid, FBANodeSizeChangeData> NodeSizeChangeDataMap;

	TMap<UEdGraphNode*, FDelegateHandle> LerpDelegateHandle;

	void OnSelectionChanged(UEdGraphNode* PreviousNode, UEdGraphNode* NewNode);

	void LinkExecWhenCreatedFromParameter(UEdGraphNode* NodeCreated);

	void AutoInsertExecNode(UEdGraphNode* NodeCreated);

	void AutoInsertParameterNode(UEdGraphNode* NodeCreated);

	void ResetGraphEditor(TWeakPtr<SGraphEditor> NewGraphEditor);

	void ReplaceSavedSelectedNode(UEdGraphNode* NewNode);

	void MoveUnrelatedNodes(TSharedPtr<FFormatterInterface> Formatter);

	void OnGraphChanged(const FEdGraphEditAction& Action);

	void DetectGraphChanges();

	void OnNodesAdded(const TArray<UEdGraphNode*>& NewNodes);

	void CacheNodeSizes(const TArray<UEdGraphNode*>& Nodes);

	void FormatNewNodes(const TArray<UEdGraphNode*>& NewNodes);

	void AutoAddParentNode(UEdGraphNode* NewNode);

	void ShowCachingNotification();

	FText GetCachingMessage() const;

	void ShowSizeTimeoutNotification();

	FText GetSizeTimeoutMessage() const;

	void OnObjectTransacted(UObject* Object, const FTransactionObjectEvent& Event);

	bool CacheNodeSize(UEdGraphNode* Node);

	bool UpdateNodeSizesChanges(const TArray<UEdGraphNode*>& Nodes);

	void AutoLerpToNewlyCreatedNode(UEdGraphNode* Node, const FFormatterInterface& Formatter);

	void AutoZoomToNode(UEdGraphNode* Node);

	bool DoesNodeWantAutoFormatting(UEdGraphNode* Node);

	void OnBeginNodeCaching();

	void OnEndNodeCaching();

	void OnDelayedCacheSizeFinished();
};
