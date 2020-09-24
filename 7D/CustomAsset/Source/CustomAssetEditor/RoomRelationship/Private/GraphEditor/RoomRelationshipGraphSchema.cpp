

#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphSchema.h"
#include "BlueprintEditorUtils.h"
#include "GenericCommands.h"
#include "GraphEditorActions.h"
#include "MultiBoxBuilder.h"
#include "ToolMenu.h"
#include "RoomRelationship/Public/RoomRelationshipAssetCommand.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphSchemaAction.h"

#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"


void URoomRelationshipGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
    TSharedPtr<FRoomRelationshipGraphSchemaAction_NewRoom> NewRoomAction(new FRoomRelationshipGraphSchemaAction_NewRoom
    (
        LOCTEXT("NodeCategory_Room", "Room"),
        LOCTEXT("MenuDesc_NewRoom", "New Room"),
        LOCTEXT("Tooltip_NewRoom", "Add new room."),
        0
    ));
    ContextMenuBuilder.AddAction(NewRoomAction);
    TSharedPtr<FRoomRelationshipGraphSchemaAction_NewDoor> NewDoorAction(new FRoomRelationshipGraphSchemaAction_NewDoor
    (
        LOCTEXT("NodeCategory_Door", "Door"),
        LOCTEXT("MenuDesc_NewDoor", "New Door"),
        LOCTEXT("Tooltip_NewDoor", "Add new door."),
        0
    ));
    ContextMenuBuilder.AddAction(NewDoorAction);
    
    if (!ContextMenuBuilder.FromPin)
    {
        auto CurrentGraph = ContextMenuBuilder.CurrentGraph;
        // The rest of the comment actions are in the UEdGraphSchema::GetContextMenuActions
        const bool bIsManyNodesSelected = CurrentGraph ? GetNodeSelectionCount(CurrentGraph) > 0 : false;
        const FText MenuDescription = bIsManyNodesSelected ?
            LOCTEXT("CreateCommentAction", "Create Comment from Selection") : LOCTEXT("AddCommentAction", "Add Comment...");

        TSharedPtr<FRoomRelationshipGraphSchemaAction_NewComment> NewAction(new FRoomRelationshipGraphSchemaAction_NewComment(
            FText(),
            MenuDescription,
            LOCTEXT("CreateCommentToolTip", "Creates a comment."),
            1
            ));
        ContextMenuBuilder.AddAction(NewAction);
    }
}

void URoomRelationshipGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
    const UBlueprint* Blueprint = Context->Blueprint;
    const UEdGraph* Graph = Context->Graph;
    const UEdGraphNode* GraphNode = Context->Node;
    const UEdGraphPin* Pin = Context->Pin;
    const bool bIsDebugging = Context->bIsDebugging;
    
    check(Graph);

    const URoomRelationshipGraphNode* RoomNode = Cast<URoomRelationshipGraphNode>(GraphNode);
    
    if (RoomNode)
    {
        FToolMenuSection& Section = Menu->AddSection("RoomRelationNodeActions", LOCTEXT("NodeActionsMenuHeader", "Node Actions"));
        {
            if (!bIsDebugging)
            {
                //Section.AddMenuEntry(FRoomRelationsAssetCommands::Get().SetAsEntry);
                Section.AddMenuEntry(FGenericCommands::Get().SelectAll);
                Section.AddMenuEntry(FGenericCommands::Get().Delete);
                Section.AddMenuEntry(FGenericCommands::Get().Rename);
                Section.AddMenuEntry(FGraphEditorCommands::Get().BreakNodeLinks);
            }
        }
    }

    Super::GetContextMenuActions(Menu, Context);
}

TSharedPtr<FEdGraphSchemaAction> URoomRelationshipGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FRoomRelationshipGraphSchemaAction_NewComment));
}

const FPinConnectionResponse URoomRelationshipGraphSchema::CanCreateConnection(const UEdGraphPin* PinA,
                                                                               const UEdGraphPin* PinB) const
{
    FPinConnectionResponse Response;
    if(PinA == PinB)
    {
        Response.SetFatal();
        Response.Message = LOCTEXT("ConnectSelfDisallowed","Cannot connect with self");
    }
    return Response;
}

bool URoomRelationshipGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	return UEdGraphSchema::TryCreateConnection(PinA, PinB);
}
