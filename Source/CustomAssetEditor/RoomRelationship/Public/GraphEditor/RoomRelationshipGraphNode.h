  
#pragma once

#include "CoreTypes.h"
#include "UObject/ObjectMacros.h"
#include "EdGraph/EdGraphNode.h"
#include "SGraphNode.h"

#include "RoomRelationshipGraphNode.generated.h"

UCLASS()
class CUSTOMASSETEDITOR_API URoomRelationshipGraphNode : public UEdGraphNode
{
    GENERATED_BODY()

public:

public:
    /** If true, this node can be renamed in the editor */
    virtual bool GetCanRenameNode() const override;
    virtual bool CanUserDeleteNode() const override;

    /**
    * Autowire a newly created node.
    *
    * @param	FromPin	The source pin that caused the new node to be created (typically a drag-release context menu creation).
    */
    virtual void AutowireNewNode(UEdGraphPin* FromPin) override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual void AllocateDefaultPins() override;
	virtual void PostPlacedNewNode() override;
    virtual TSharedPtr<class INameValidatorInterface> MakeNameValidator() const override;
    virtual void OnRenameNode(const FString& NewName) override;
    
    //Implement in Graph
    //virtual void DestroyNode() override;
	virtual void NodeConnectionListChanged() override;

    FName GetNodeRoomName() const;

    UEdGraphPin* GetPin() const;

    bool IsRoom() const;

    class URoomNode* GetNode() const;
    TArray<class URoomNode*> GetConnectedRooms() const;

private:
    
    void UpdateRoomNode();

    class URoomRelationshipAsset* GetAsset() const;
    
private:
    UPROPERTY()
    class URoomNode* HandlingNode;

public:
    bool SPECIAL_FLAG_IS_DOOR = false;
};
