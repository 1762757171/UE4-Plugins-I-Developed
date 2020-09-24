

#include "RoomRelationship/Public/RoomRelationshipAssetEditor.h"
#include "AssetEditorManager.h"
#include "BlueprintEditorUtils.h"
#include "EdGraphNode_Comment.h"
#include "GenericCommands.h"
#include "GraphEditor.h"
#include "GraphEditorActions.h"
#include "Kismet2NameValidators.h"
#include "ScopedTransaction.h"
#include "SDockTab.h"
#include "GraphEditor/SRoomInfoDetail.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "RoomRelationship/Public/RoomRelationshipAssetCommand.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphSchema.h"
#include "Logging/TokenizedMessage.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"

#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"

const FName FRoomRelationshipAssetEditor::TabId_GraphCanvas(TEXT("TabId_GraphCanvas"));

FRoomRelationshipAssetEditor::FRoomRelationshipAssetEditor()
    : AssetEdit(nullptr)
{
}

void FRoomRelationshipAssetEditor::InitRoomRelationshipAssetEditor(
    const TSharedPtr<IToolkitHost>& InitToolkitHost, URoomRelationshipAsset* ObjectToEdit)
{

    FRoomRelationsAssetCommands::Register();
    
    // close all other editors editing this asset
    UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
    AssetEditorSubsystem->CloseOtherEditors(ObjectToEdit, this);
    
	EToolkitMode::Type Mode = InitToolkitHost.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
    AssetEdit = ObjectToEdit;


    FAssetEditorToolkit::InitAssetEditor(
        Mode,
        InitToolkitHost,
        TEXT("RoomRelationshipAssetEditor"),
        CreateLayout(),
        true,
        true,
        ObjectToEdit
    );
}

void FRoomRelationshipAssetEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu", "Room Relationship Asset Editor"));
    const TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

    FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

    InTabManager->RegisterTabSpawner(
        TabId_GraphCanvas,
        FOnSpawnTab::CreateSP(this, &FRoomRelationshipAssetEditor::SpawnTab_GraphCanvas))
            .SetDisplayName(LOCTEXT("GraphCanvasTab", "Viewport"))
            .SetGroup(WorkspaceMenuCategoryRef)
            .SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "GraphEditor.EventGraph_16x"))
    ;
}

void FRoomRelationshipAssetEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
    FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

    InTabManager->UnregisterTabSpawner(TabId_GraphCanvas);
}

FName FRoomRelationshipAssetEditor::GetToolkitFName() const
{
    return TEXT("RoomRelationshipAssetToolkit");
}

FText FRoomRelationshipAssetEditor::GetBaseToolkitName() const
{
    return LOCTEXT("BaseToolKitName", "RoomRelationshipAssetToolkit");
}

FString FRoomRelationshipAssetEditor::GetWorldCentricTabPrefix() const
{
    return TEXT("GameplayConstomized");
}

FLinearColor FRoomRelationshipAssetEditor::GetWorldCentricTabColorScale() const
{
    return FLinearColor::Green;
}


TSharedRef<FTabManager::FLayout> FRoomRelationshipAssetEditor::CreateLayout() const
{
    return
    FTabManager::NewLayout("AssetEditorLayout")
    ->AddArea
    (
        //PrimaryArea Begin
        FTabManager::NewPrimaryArea()
        ->SetOrientation(Orient_Vertical)
        ->Split
        (
        //--Toolbar Begin
            FTabManager::NewStack()
            ->SetSizeCoefficient(0.1f)
            ->SetHideTabWell(true)
            ->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
        //--Toolbar End
        )
        ->Split
        (
        //--Editor Begin
            FTabManager::NewStack()
            ->SetSizeCoefficient(0.9f)
            ->SetHideTabWell(true)
            ->AddTab(TabId_GraphCanvas, ETabState::OpenedTab)
        //--Editor End
        )
        //PrimaryArea End
    );
}

TSharedRef<SDockTab> FRoomRelationshipAssetEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
    check(Args.GetTabId() == TabId_GraphCanvas)
    
    UEdGraph* Graph = AssetEdit->GetGraph();
    
    if(!Graph)
    {
        Graph = FBlueprintEditorUtils::CreateNewGraph(
            AssetEdit,
            NAME_None,
            URoomRelationshipGraph::StaticClass(),
            URoomRelationshipGraphSchema::StaticClass()
            );
        AssetEdit->SetGraph(Graph);
    }

    // Customize the appereance of the graph.
    FGraphAppearanceInfo AppearanceInfo;
    // The text that appears on the bottom right corner in the graph view.
    AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "Room Relationship");
    AppearanceInfo.InstructionText = LOCTEXT("AppearanceInstructionText", "Right Click to add new nodes.");

    // Bind graph events actions from the editor
    SGraphEditor::FGraphEditorEvents InEvents;
    InEvents.OnVerifyTextCommit = FOnNodeVerifyTextCommit::CreateSP(this, &FRoomRelationshipAssetEditor::OnNodeVerifyTitleCommit);
    InEvents.OnTextCommitted = FOnNodeTextCommitted::CreateSP(this, &FRoomRelationshipAssetEditor::OnNodeTitleCommitted);
    InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FRoomRelationshipAssetEditor::OnSelectedNodesChanged);
    //InEvents.OnCreateActionMenu = SGraphEditor::FOnCreateActionMenu::CreateSP(this, &FFSMAssetEditor::OnCreateGraphActionMenu);

    //TODO: add graph editor commands here
    /** Command list for the graph editor */
    if(!GraphEditorCommands.IsValid())
    {
        GraphEditorCommands = MakeShareable(new FUICommandList);
        
        /*GraphEditorCommands->MapAction(FRoomRelationsAssetCommands::Get().SetAsEntry,
            FExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::SetAsEntry),
            FCanExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::CanSetAsEntry)
            );*/
    
        GraphEditorCommands->MapAction(FGenericCommands::Get().SelectAll,
            FExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::SelectAllNodes),
            FCanExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::CanSelectAllNodes)
            );

        GraphEditorCommands->MapAction(FGenericCommands::Get().Delete,
            FExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::DeleteSelectedNodes),
            FCanExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::CanDeleteNodes)
            );

        GraphEditorCommands->MapAction(FGenericCommands::Get().Rename,
            FExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::RenameSelectedNode),
            FCanExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::CanRenameNode)
            );

        GraphEditorCommands->MapAction(
            FGraphEditorCommands::Get().CreateComment,
            FExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::OnCreateComment),
            FCanExecuteAction::CreateRaw(this, &FRoomRelationshipAssetEditor::CanCreateComment)
        );
    }
    
    return
    SNew(SDockTab)
        .Label(LOCTEXT("EditorGraphCanvasViewport", "Viewport"))
        [
            SNew(SSplitter)
                .Orientation(Orient_Horizontal)
                +SSplitter::Slot()
                    .Value(0.8f)
                    [
                        SAssignNew(GraphView, SGraphEditor)
                            .AdditionalCommands(GraphEditorCommands)
                            .IsEditable(true)
                            .Appearance(AppearanceInfo)
                            .GraphToEdit(Graph)
                            .GraphEvents(InEvents)
                            .ShowGraphStateOverlay(true)
                    ]
                +SSplitter::Slot()
                    .Value(0.2f)
                    [
                        SNew(SBorder)
                            .HAlign(HAlign_Fill)
                            .VAlign(VAlign_Fill)
                            [
                                SAssignNew(InfoDetail, SRoomInfoDetail, AssetEdit)
                            ]
                    ]
        ]
    ;
}

void FRoomRelationshipAssetEditor::DeleteSelectedNodes()
{
    const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
    GraphView->GetCurrentGraph()->Modify();

    const FGraphPanelSelectionSet SelectedNodes = GraphView->GetSelectedNodes();
    GraphView->ClearSelectionSet();

    for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
    {
        if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
        {
            if (Node->CanUserDeleteNode())
            {
                Node->Modify();
                Node->DestroyNode();
            }
        }
    }
}

bool FRoomRelationshipAssetEditor::CanDeleteNodes() const
{
    return true;
}

void FRoomRelationshipAssetEditor::SelectAllNodes()
{
    GraphView->SelectAllNodes();
}

bool FRoomRelationshipAssetEditor::CanSelectAllNodes() const
{
    return true;
}

void FRoomRelationshipAssetEditor::RenameSelectedNode()
{
    const FScopedTransaction Transaction(FGenericCommands::Get().Delete->GetDescription());
    GraphView->GetCurrentGraph()->Modify();

    const FGraphPanelSelectionSet SelectedNodes = GraphView->GetSelectedNodes();
    GraphView->ClearSelectionSet();

    for (FGraphPanelSelectionSet::TConstIterator NodeIt(SelectedNodes); NodeIt; ++NodeIt)
    {
        if (UEdGraphNode* Node = Cast<UEdGraphNode>(*NodeIt))
        {
            if (Node->GetCanRenameNode())
            {
                Node->Modify();
                GraphView->IsNodeTitleVisible(Node, true);
                return;
            }
        }
    }
}

bool FRoomRelationshipAssetEditor::CanRenameNode() const
{
    return true;
}

void FRoomRelationshipAssetEditor::OnCreateComment()
{
    if (UEdGraph* EdGraph = GraphView->GetCurrentGraph())
    {
        TSharedPtr<FEdGraphSchemaAction> Action = EdGraph->GetSchema()->GetCreateCommentAction();
        if (Action.IsValid())
        {
            Action->PerformAction(EdGraph, nullptr, FVector2D());
        }
    }
}

bool FRoomRelationshipAssetEditor::CanCreateComment() const
{
    return GraphView->GetNumberOfSelectedNodes() != 0;
}

bool FRoomRelationshipAssetEditor::OnNodeVerifyTitleCommit(const FText& NewText, UEdGraphNode* NodeBeingChanged,
    FText& OutErrorMessage)
{
    //Reference: FBlueprintEditor::OnNodeVerifyTitleCommit
    bool bValid(false);
    TSharedPtr<INameValidatorInterface> NameEntryValidator = nullptr;

    if (NodeBeingChanged && NodeBeingChanged->GetCanRenameNode())
    {
        // Clear off any existing error message 
        NodeBeingChanged->ErrorMsg.Empty();
        NodeBeingChanged->bHasCompilerMessage = false;

        if (!NameEntryValidator.IsValid())
        {
            NameEntryValidator = FNameValidatorFactory::MakeValidator(NodeBeingChanged);
        }

        EValidatorResult VResult = NameEntryValidator->IsValid(NewText.ToString(), true);
        if (VResult == EValidatorResult::Ok)
        {
            bValid = true;
        }
        else if (GraphView.IsValid())
        {
            const EValidatorResult Valid = NameEntryValidator->IsValid(NewText.ToString(), false);

			OutErrorMessage = NameEntryValidator->GetErrorText(NewText.ToString(), Valid);

            NodeBeingChanged->bHasCompilerMessage = true;
            NodeBeingChanged->ErrorMsg = NameEntryValidator->GetErrorString(NewText.ToString(), Valid);
            NodeBeingChanged->ErrorType = EMessageSeverity::Error;
        }
    }
    NameEntryValidator.Reset();

    return bValid;
}

void FRoomRelationshipAssetEditor::OnSelectedNodesChanged(const FGraphPanelSelectionSet& NewSelections)
{
    TSet<UObject*> RoomNodes;
    for(auto s : NewSelections)
    {
        URoomRelationshipGraphNode* n = Cast<URoomRelationshipGraphNode>(s);
        if(n)
        {
            RoomNodes.Add(n->GetNode());
        }else
        {
            UEdGraphNode_Comment* Comment = Cast<UEdGraphNode_Comment>(s);
            if(Comment)
            {
                RoomNodes.Add(Comment);
            }
        }
    }
    InfoDetail->SetSelection(RoomNodes);
}

void FRoomRelationshipAssetEditor::OnNodeTitleCommitted(const FText& NewText, ETextCommit::Type CommitInfo,
    UEdGraphNode* NodeBeingChanged)
{
    if (NodeBeingChanged)
    {
        static const FText TransactionTitle = FText::FromString(FString(TEXT("Rename Node")));
        const FScopedTransaction Transaction(TransactionTitle);
        NodeBeingChanged->Modify();
        NodeBeingChanged->OnRenameNode(NewText.ToString());
    }
}

#undef LOCTEXT_NAMESPACE
