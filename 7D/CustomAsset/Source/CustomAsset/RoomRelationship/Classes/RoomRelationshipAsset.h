// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "EdGraph/EdGraph.h"
#include "RoomRelationshipAsset.generated.h"


UCLASS()
class CUSTOMASSET_API URoomRelationshipAsset : public UObject
{
    GENERATED_BODY()

public:

#ifdef WITH_EDITOR
    void SetGraph(UEdGraph* NewGraph);
    UEdGraph* GetGraph() const;
#endif

    bool ContainRoom(const FName& RoomName) const;

    void AddRoom(class URoomNode* NewRoom);

    URoomNode* GetNodeFromName(const FName& RoomName) const;

    void ClearAllRooms();

    FName GetEntryRoom(uint8 Day) const;

    uint8 GetIsEntryRoom(const FName& RoomName) const;

    bool GetRoomDistance(const FName& RoomAName, const FName& RoomBName, uint8 Day, TArray<FName>& OutPath) const;

private:
#if WITH_EDITORONLY_DATA
    UPROPERTY()
    UEdGraph* Graph;
#endif

    UPROPERTY()
    TArray<URoomNode*> Rooms;

	//8 Days = 6 Days + double endings
    UPROPERTY(EditAnywhere, Category = "Entry", meta=(AllowPrivateAccess = "true"))
    FName EntryRooms[8];
};
