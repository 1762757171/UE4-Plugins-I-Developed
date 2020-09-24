#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "RoomNodeInfo.h"

#include "RoomRelationshipNode.generated.h"

UCLASS()
class CUSTOMASSET_API URoomNode : public UObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    FName RoomName;
    UPROPERTY()
    TArray<class URoomNode*> Connection;
    UPROPERTY(EditAnywhere, Category = "Info", meta=(AllowPrivateAccess = "true"))
    FRoomNodeInfo RoomInfo;
    
public:
    
    void SetLabelName(const FName& NewLabelName);
    FName GetLabelName() const;

    void SetIsDoor(bool bNewIsDoor);
    bool GetIsDoor() const;
    
    TSet<class URoomNode*> GetConnectedRooms(uint8 Day = 0) const;
    TSet<class URoomNode*> GetConnectedDoors() const;

    void BreakConnectionWith(class URoomNode* node);
    void EstablishConnectionWith(class URoomNode* node);

    FRoomNodeInfo GetNodeInfo() const;
    
    void ReestablishConnection(const TArray<class URoomNode*>& NewConnection);

    bool IsAvailableWhen(uint8 Day = 0) const;
};