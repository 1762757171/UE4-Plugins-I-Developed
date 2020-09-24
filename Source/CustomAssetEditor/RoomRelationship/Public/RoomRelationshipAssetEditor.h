#pragma once
#include "AssetEditorToolkit.h"
#include "EditorUndoClient.h"
#include "GraphEditor.h"
#include "NotifyHook.h"

class CUSTOMASSETEDITOR_API FRoomRelationshipAssetEditor : public FNotifyHook, public FEditorUndoClient, public FAssetEditorToolkit
{
public:
    FRoomRelationshipAssetEditor();
    
public:
    void InitRoomRelationshipAssetEditor(const TSharedPtr<IToolkitHost>& InitToolkitHost, class URoomRelationshipAsset* ObjectToEdit);

    virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
    virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;

    virtual FName GetToolkitFName() const override;
    virtual FText GetBaseToolkitName() const override;
    virtual FString GetWorldCentricTabPrefix() const override;
    virtual FLinearColor GetWorldCentricTabColorScale() const override;

private:
    TSharedRef<FTabManager::FLayout> CreateLayout() const;
    TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs & Args);

private:
    void DeleteSelectedNodes();
    bool CanDeleteNodes() const;
    
    void SelectAllNodes();
    bool CanSelectAllNodes() const;
    
    void RenameSelectedNode();
    bool CanRenameNode() const;
    
    void OnCreateComment();
    bool CanCreateComment() const;

    bool OnNodeVerifyTitleCommit(const FText& NewText, UEdGraphNode* NodeBeingChanged, FText& OutErrorMessage);
    void OnSelectedNodesChanged( const FGraphPanelSelectionSet& NewSelections);
    void OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);
private:
    //void OnRoomNodeRenamed(const FText& NewText, ETextCommit::Type CommitInfo, UEdGraphNode* NodeBeingChanged);

private:
    class URoomRelationshipAsset* AssetEdit;

    TSharedPtr<class SGraphEditor> GraphView;
    
    TSharedPtr<class FUICommandList> GraphEditorCommands;

    TSharedPtr<class SRoomInfoDetail> InfoDetail;
private:
    static const FName TabId_GraphCanvas;
};
