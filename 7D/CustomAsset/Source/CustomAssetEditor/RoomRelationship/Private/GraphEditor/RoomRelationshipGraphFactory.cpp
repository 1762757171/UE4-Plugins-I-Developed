
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphFactory.h"


#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphConnectionDrawingPolicy.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphSchema.h"
#include "RoomRelationship/Public/GraphEditor/SRoomRelationshipGraphNode.h"

TSharedPtr<SGraphNode> FRoomRelationshipGraphNodeFactory::CreateNode(UEdGraphNode* InNode) const
{
    if (auto Node = Cast<URoomRelationshipGraphNode>(InNode))
    {
        return SNew(SRoomRelationshipGraphNode, Node);
    }
    return nullptr;
}

TSharedPtr<SGraphPin> FRoomRelationshipGraphPinFactory::CreatePin(UEdGraphPin* Pin) const
{
    if (Pin->PinType.PinCategory == TEXT("Transition"))
    {
        return SNew(SRoomRelationshipGraphPin, Pin);
    }

    return nullptr;
}

FConnectionDrawingPolicy* FRoomRelationshipGraphPinConnectionFactory::CreateConnectionPolicy(
    const UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor,
    const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj) const
{
    if (Schema->IsA(URoomRelationshipGraphSchema::StaticClass())) {
        return new FRoomRelationshipGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements, InGraphObj);
    }
    return nullptr;
}
