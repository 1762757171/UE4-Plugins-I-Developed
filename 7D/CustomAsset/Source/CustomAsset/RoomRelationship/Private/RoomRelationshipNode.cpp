
#include "RoomRelationship/Classes/RoomRelationshipNode.h"

void URoomNode::SetLabelName(const FName& NewLabelName)
{
    RoomName = NewLabelName;
}

FName URoomNode::GetLabelName() const
{
    return RoomName;
}

void URoomNode::SetIsDoor(bool bNewIsDoor)
{
    RoomInfo.bIsDoor = bNewIsDoor;
}

bool URoomNode::GetIsDoor() const
{
    return RoomInfo.bIsDoor;
}

TSet<URoomNode*> URoomNode::GetConnectedRooms(uint8 Day) const
{
    TSet<URoomNode*> ConnectedRooms;
    for(const auto& con : Connection)
    {
        if(!con->GetIsDoor() )
        {
            if(con->IsAvailableWhen(Day))
            {
                ConnectedRooms.Add(con);
            }
        }else
        {
            //if this is a door
            //collect its adjacent room
            ConnectedRooms.Append( con->GetConnectedRooms(Day) );
        }
    }

    ConnectedRooms.Remove(const_cast<URoomNode*>(this));
    
    return ConnectedRooms;
}

TSet<URoomNode*> URoomNode::GetConnectedDoors() const
{
    if(GetIsDoor())
    {
        return {};
    }
    TSet<URoomNode*> ConnectedDoors;
    for(const auto& con : Connection)
    {
        if(con->GetIsDoor())
        {
            ConnectedDoors.Add(con);
        }
    }
    return ConnectedDoors;
}

void URoomNode::BreakConnectionWith(URoomNode* node)
{
    if(Connection.Contains(node))
    {
        Connection.Remove(node);
        node->BreakConnectionWith(this);
    }
}

void URoomNode::EstablishConnectionWith(URoomNode* node)
{
    if(!Connection.Contains(node))
    {
        Connection.Add(node);
        node->EstablishConnectionWith(this);
    }
}

FRoomNodeInfo URoomNode::GetNodeInfo() const
{
    return RoomInfo;
}

void URoomNode::ReestablishConnection(const TArray<URoomNode*>& NewConnection)
{
    TArray<URoomNode*> Transient = Connection;
    for(URoomNode* r : Transient)	
    {
        r->BreakConnectionWith(this);
    }
    
    Connection = NewConnection;

    for(URoomNode* r : NewConnection)
    {
        r->EstablishConnectionWith(this);
    }
}

bool URoomNode::IsAvailableWhen(uint8 Day) const
{
    return RoomInfo.IsAvailableWhen(Day);
}
