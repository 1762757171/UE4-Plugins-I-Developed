  
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphSchemaAction.h"
#include "UnrealEd.h"
#include "UnrealMath.h"
#include "ScopedTransaction.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"

#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"

UEdGraphNode * FRoomRelationshipGraphSchemaAction_NewRoom::PerformAction(UEdGraph * ParentGraph, UEdGraphPin * FromPin, const FVector2D Location, bool bSelectNewNode)
{
    const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
    
    URoomRelationshipGraphNode* GraphNode = NewObject<URoomRelationshipGraphNode>();
    
    ParentGraph->Modify();
    if (FromPin)
    {
        FromPin->Modify();
    }

    GraphNode->SetFlags(RF_Transactional);

    // set outer to be the graph so it doesn't go away
    GraphNode->Rename(nullptr, ParentGraph);
    ParentGraph->AddNode(GraphNode, true);

    GraphNode->CreateNewGuid();

    GraphNode->SPECIAL_FLAG_IS_DOOR = false;
    
    GraphNode->PostPlacedNewNode();

    /*
    // For input pins, new node will generally overlap node being dragged off
    // Work out if we want to visually push away from connected node
    int32 XLocation = Location.X;
    if (FromPin)
    {
        UEdGraphNode* PinNode = FromPin->GetOwningNode();
        const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

        if (XDelta < NodeDistance)
        {
            // Set location to edge of current node minus the max move distance
            // to force node to push off from connect node enough to give selection handle
            XLocation = PinNode->NodePosX - NodeDistance;
        }
    }
    */
    
    GraphNode->NodePosX = Location.X;
    GraphNode->NodePosY = Location.Y;
    GraphNode->SnapToGrid(0.5);

    // setup pins after placing node in correct spot, since pin sorting will happen as soon as link connection change occurs
    GraphNode->AllocateDefaultPins();
    GraphNode->AutowireNewNode(FromPin);

    return GraphNode;
}

UEdGraphNode* FRoomRelationshipGraphSchemaAction_NewDoor::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
    const FVector2D Location, bool bSelectNewNode)
{
    const FScopedTransaction Transaction(LOCTEXT("AddDoor", "Add Door"));
    
    URoomRelationshipGraphNode* GraphNode = NewObject<URoomRelationshipGraphNode>();
    
    ParentGraph->Modify();
    if (FromPin)
    {
        FromPin->Modify();
    }

    GraphNode->SetFlags(RF_Transactional);

    // set outer to be the graph so it doesn't go away
    GraphNode->Rename(nullptr, ParentGraph);
    ParentGraph->AddNode(GraphNode, true);

    GraphNode->CreateNewGuid();

    GraphNode->SPECIAL_FLAG_IS_DOOR = true;

    GraphNode->PostPlacedNewNode();

    /*
    // For input pins, new node will generally overlap node being dragged off
    // Work out if we want to visually push away from connected node
    int32 XLocation = Location.X;
    if (FromPin)
    {
    UEdGraphNode* PinNode = FromPin->GetOwningNode();
    const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);

    if (XDelta < NodeDistance)
    {
    // Set location to edge of current node minus the max move distance
    // to force node to push off from connect node enough to give selection handle
    XLocation = PinNode->NodePosX - NodeDistance;
    }
    }
    */
    
    GraphNode->NodePosX = Location.X;
    GraphNode->NodePosY = Location.Y;
    GraphNode->SnapToGrid(0.5);

    // setup pins after placing node in correct spot, since pin sorting will happen as soon as link connection change occurs
    GraphNode->AllocateDefaultPins();
    GraphNode->AutowireNewNode(FromPin);

    return GraphNode;
}

UEdGraphNode* FRoomRelationshipGraphSchemaAction_NewComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin,
                                                                           const FVector2D Location, bool bSelectNewNode)
{
    UEdGraphNode_Comment* const CommentTemplate = NewObject<UEdGraphNode_Comment>();

    FVector2D SpawnLocation = Location;
    FSlateRect Bounds;

    TSharedPtr<SGraphEditor> GraphEditorPtr = SGraphEditor::FindGraphEditorForGraph(ParentGraph);
    if (GraphEditorPtr.IsValid() && GraphEditorPtr->GetBoundsForSelectedNodes(/*out*/ Bounds, 50.0f))
    {
        CommentTemplate->SetBounds(Bounds);
        SpawnLocation.X = CommentTemplate->NodePosX;
        SpawnLocation.Y = CommentTemplate->NodePosY;
    }

    UEdGraphNode* const NewNode = FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation, bSelectNewNode);

    return NewNode;
}
#undef LOCTEXT_NAMESPACE
