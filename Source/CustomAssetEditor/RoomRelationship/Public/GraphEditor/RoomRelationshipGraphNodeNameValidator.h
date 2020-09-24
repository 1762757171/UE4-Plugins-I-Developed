#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet2NameValidators.h"

class FRoomRelationshipGraphNodeNameValidator : public INameValidatorInterface
{
public:
    FRoomRelationshipGraphNodeNameValidator(const class UEdGraphNode* InNode);
    virtual EValidatorResult IsValid(const FString& Name, bool bOriginal) override;
    virtual EValidatorResult IsValid(const FName& Name, bool bOriginal) override;
protected:
    const class URoomRelationshipGraphNode* Node;
    const class URoomRelationshipAsset* NodeAsset;
};
