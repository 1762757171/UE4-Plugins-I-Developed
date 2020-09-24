

#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"


void URoomRelationshipAsset::SetGraph(UEdGraph* NewGraph)
{
    Graph = NewGraph;
}

UEdGraph* URoomRelationshipAsset::GetGraph() const
{
    return Graph;
}

bool URoomRelationshipAsset::ContainRoom(const FName& RoomName) const
{
	return GetNodeFromName(RoomName) != nullptr;
}

void URoomRelationshipAsset::AddRoom(URoomNode* NewRoom)
{
    Rooms.Add(NewRoom);
}

URoomNode* URoomRelationshipAsset::GetNodeFromName(const FName& RoomName) const
{
	for (const auto& r : Rooms) {
		if (r->GetLabelName() == RoomName) {
			return r;
		}
	}
    return nullptr;
}

void URoomRelationshipAsset::ClearAllRooms()
{
    Rooms.Empty();
}

uint8 URoomRelationshipAsset::GetIsEntryRoom(const FName& RoomName) const
{
    for(int32 i = 0; i < 8; ++i)
    {
        auto n = EntryRooms[i];
        if(n == RoomName)
        {
            return i + 1;
        }
    }
    return 0;
}

bool URoomRelationshipAsset::GetRoomDistance(const FName& RoomAName, const FName& RoomBName, uint8 Day,
    TArray<FName>& OutPath) const
{
    if(RoomAName == RoomBName)
    {
        OutPath.Add(RoomAName);
        return true;
    }
    
    URoomNode* RoomNodeA = GetNodeFromName(RoomAName);
    URoomNode* RoomNodeB = GetNodeFromName(RoomBName);
    if(!RoomNodeA || !RoomNodeB || !RoomNodeA->IsAvailableWhen(Day) || !RoomNodeB->IsAvailableWhen(Day))
    {
        return false;
    }

    //DFS
    if(OutPath.Contains(RoomAName) || OutPath.Contains(RoomBName))
    {
        return false;
    }
    TSet<URoomNode*> AdjacentRooms = RoomNodeA->GetConnectedRooms(Day);
    TArray<FName> ShortestPaths = OutPath;
    bool bFoundAtLeastOnePath = false;
    
    for(URoomNode* r : AdjacentRooms)
    {
        TArray<FName> CurrentPath = OutPath;
        CurrentPath.Add(RoomNodeA->GetLabelName());
        
        if(GetRoomDistance(r->GetLabelName(), RoomBName, Day, CurrentPath))
        {
            if(!bFoundAtLeastOnePath || CurrentPath.Num() < ShortestPaths.Num())
            {
                ShortestPaths = CurrentPath;
                bFoundAtLeastOnePath = true;
            }
        }
    }    

    if(bFoundAtLeastOnePath)
    {
        OutPath = ShortestPaths;
    }
    
    return bFoundAtLeastOnePath;
}

FName URoomRelationshipAsset::GetEntryRoom(uint8 ind) const
{
    return EntryRooms[ind - 1];
}
