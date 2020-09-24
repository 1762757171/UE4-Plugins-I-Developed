  
#pragma once

#include "EdGraph/EdGraphSchema.h"
#include "Templates/SubclassOf.h"

#include "RoomRelationshipGraphSchemaAction.generated.h"

/** Action to add a room to the graph */
USTRUCT()
struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphSchemaAction_NewRoom : public FEdGraphSchemaAction
{
    GENERATED_BODY()
    
public:
    FRoomRelationshipGraphSchemaAction_NewRoom() : FEdGraphSchemaAction() {}
    FRoomRelationshipGraphSchemaAction_NewRoom(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping)
        : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {}

public:
    virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

/** Action to add a door to the graph */
USTRUCT()
struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphSchemaAction_NewDoor : public FEdGraphSchemaAction
{
    GENERATED_BODY()
    
public:
    FRoomRelationshipGraphSchemaAction_NewDoor() : FEdGraphSchemaAction() {}
    FRoomRelationshipGraphSchemaAction_NewDoor(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping)
        : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {}

    public:
    virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};

/** Action to add a comment to the graph */
USTRUCT()
struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphSchemaAction_NewComment : public FEdGraphSchemaAction
{
    GENERATED_BODY()
    
    public:
    FRoomRelationshipGraphSchemaAction_NewComment() : FEdGraphSchemaAction() {}
    FRoomRelationshipGraphSchemaAction_NewComment(const FText& InNodeCategory, const FText& InMenuDesc, const FText& InToolTip, int32 InGrouping)
        : FEdGraphSchemaAction(InNodeCategory, InMenuDesc, InToolTip, InGrouping) {}

    public:
    virtual UEdGraphNode* PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode = true) override;
};