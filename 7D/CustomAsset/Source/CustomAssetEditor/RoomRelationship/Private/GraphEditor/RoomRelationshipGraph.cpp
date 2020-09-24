  
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"

#include "GraphEditAction.h"
#include "TokenizedMessage.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"

URoomRelationshipAsset* URoomRelationshipGraph::GetRoomRelationshipAssetEdit() const
{
    return GetTypedOuter<URoomRelationshipAsset>();
}

void URoomRelationshipGraph::NotifyGraphChanged()
{
    Super::NotifyGraphChanged();
    UpdateAsset();
}

void URoomRelationshipGraph::NotifyGraphChanged(const FEdGraphEditAction& Action)
{
    Super::NotifyGraphChanged(Action);
    uint8 Actions = Action.Action;
    if(!(Actions & GRAPHACTION_AddNode))
    {
        UpdateAsset();
    }
}

void URoomRelationshipGraph::UpdateAsset()
{
    URoomRelationshipAsset* Asset = GetRoomRelationshipAssetEdit();
    //URoomNode* Entry = Asset->GetEntryRoom();
    Asset->ClearAllRooms();
    TArray<URoomRelationshipGraphNode*> RoomNodes;
    GetNodesOfClass(RoomNodes);
    for(const auto& n : RoomNodes)
    {
        if(!n->GetNode())
        {
            continue;
        }
        if(Asset->ContainRoom(n->GetNodeRoomName()))
        {
            n->ErrorType = EMessageSeverity::Error;
            n->ErrorMsg = TEXT("Conflict name!!!");
            continue;
        }
        
        URoomNode* r = n->GetNode();

        TArray<URoomNode*> con = n->GetConnectedRooms();

        r->ReestablishConnection(con);

        /*if(r == Entry)
        {
            Asset->SetEntryRoom(r);
        }*/
        
        Asset->AddRoom(r);
    }
}
