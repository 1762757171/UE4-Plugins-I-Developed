
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"


#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNodeNameValidator.h"


bool URoomRelationshipGraphNode::GetCanRenameNode() const
{
    return true;
}

bool URoomRelationshipGraphNode::CanUserDeleteNode() const
{
    return true;
}

void URoomRelationshipGraphNode::AutowireNewNode(UEdGraphPin* FromPin)
{
    if(FromPin)
    {
		GetSchema()->TryCreateConnection(FromPin, GetPin());
        FromPin->GetOwningNode()->NodeConnectionListChanged();
        NodeConnectionListChanged();
    }
}

FText URoomRelationshipGraphNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromName(GetNodeRoomName());
}

void URoomRelationshipGraphNode::AllocateDefaultPins()
{
	UEdGraphPin* Pin = CreatePin(EGPD_Output, TEXT("Transition"), TEXT("Pin"));
}

void URoomRelationshipGraphNode::PostPlacedNewNode()
{
    Super::PostPlacedNewNode();
    
    URoomRelationshipAsset* Asset = GetAsset();
    
    HandlingNode = NewObject<URoomNode>(Asset);
    HandlingNode->SetFlags(RF_Transactional);
    TSharedPtr<INameValidatorInterface> Validator = MakeNameValidator();
    FString NewRoomName = SPECIAL_FLAG_IS_DOOR ? TEXT("NewDoor") : TEXT("NewRoom");
    HandlingNode->SetIsDoor(SPECIAL_FLAG_IS_DOOR);
    Validator->FindValidString(NewRoomName);

    FName NewRoomFName(*NewRoomName);
    HandlingNode->SetLabelName(NewRoomFName);
    Asset->AddRoom(HandlingNode);
}

TSharedPtr<INameValidatorInterface> URoomRelationshipGraphNode::MakeNameValidator() const
{
	return MakeShareable(new FRoomRelationshipGraphNodeNameValidator(this));
}

void URoomRelationshipGraphNode::OnRenameNode(const FString& NewName)
{
    Super::OnRenameNode(NewName);

	Modify();

    FName NewRoomName(*NewName);
	
    HandlingNode->SetLabelName(NewRoomName);

    //    Redundant
    //UpdateRoomNode();
}
/*
void URoomRelationshipGraphNode::DestroyNode()
{
    Super::DestroyNode();
    UpdateRoomNode();
}
*/
void URoomRelationshipGraphNode::NodeConnectionListChanged()
{
    Super::NodeConnectionListChanged();
    UpdateRoomNode();
}

FName URoomRelationshipGraphNode::GetNodeRoomName() const
{
    return HandlingNode->GetLabelName();
}

UEdGraphPin* URoomRelationshipGraphNode::GetPin() const
{
    return Pins[0];
}

bool URoomRelationshipGraphNode::IsRoom() const
{
    //@ PATCH : for door
    return !HandlingNode->GetIsDoor();
}

URoomNode* URoomRelationshipGraphNode::GetNode() const
{
    return HandlingNode;
}

TArray<URoomNode*> URoomRelationshipGraphNode::GetConnectedRooms() const
{
    UEdGraphPin* Pin = GetPin();

    TArray<URoomNode*> ConnectedRooms;
    
    for(const UEdGraphPin* p : Pin->LinkedTo)
    {
        URoomRelationshipGraphNode* ConnectedNode = Cast<URoomRelationshipGraphNode>( p->GetOwningNode() );
        ConnectedRooms.Add(ConnectedNode->HandlingNode);
    }

    return ConnectedRooms;
}

void URoomRelationshipGraphNode::UpdateRoomNode()
{
    TArray<URoomNode*> ConnectedRooms = GetConnectedRooms();

    HandlingNode->ReestablishConnection(ConnectedRooms);
}

URoomRelationshipAsset* URoomRelationshipGraphNode::GetAsset() const
{
    URoomRelationshipGraph* RoomGraph = Cast<URoomRelationshipGraph>(GetGraph());
    return RoomGraph->GetRoomRelationshipAssetEdit();
}
