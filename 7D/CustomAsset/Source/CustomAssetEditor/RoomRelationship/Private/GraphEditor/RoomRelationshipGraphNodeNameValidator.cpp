#pragma once

#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNodeNameValidator.h"
#include "Kismet2NameValidators.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"

FRoomRelationshipGraphNodeNameValidator::FRoomRelationshipGraphNodeNameValidator(const UEdGraphNode* InNode)
{
    Node = Cast<URoomRelationshipGraphNode>( InNode );
    URoomRelationshipGraph* Graph = Cast<URoomRelationshipGraph>(InNode->GetOuter());
    NodeAsset = Graph->GetRoomRelationshipAssetEdit();
}

EValidatorResult FRoomRelationshipGraphNodeNameValidator::IsValid(const FString& Name, bool bOriginal)
{
    return IsValid(FName(*Name), bOriginal);
}

EValidatorResult FRoomRelationshipGraphNodeNameValidator::IsValid(const FName& Name, bool bOriginal)
{
    if (Name.IsNone()) {
        return EValidatorResult::EmptyName;
    }
    if (!Name.IsValidXName()) {
        return EValidatorResult::ContainsInvalidCharacters;
    }
    if(Node->GetNodeRoomName() == Name)
    {
        return EValidatorResult::ExistingName;
    }
    if(NodeAsset->ContainRoom(Name))
    {
        return EValidatorResult::AlreadyInUse;
    }
    return EValidatorResult::Ok;
}
