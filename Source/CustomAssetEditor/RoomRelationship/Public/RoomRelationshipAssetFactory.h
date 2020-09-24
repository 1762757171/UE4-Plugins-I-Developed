// Copyright 

#pragma once

#include "CoreMinimal.h"
#include "Object.h"
#include "Factories/Factory.h"

#include "RoomRelationshipAssetFactory.generated.h"

UCLASS()
class CUSTOMASSETEDITOR_API URoomRelationshipAssetFactory : public UFactory
{
    GENERATED_BODY()

public:
    URoomRelationshipAssetFactory();    
    
protected:
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    virtual FText GetDisplayName() const override;
};
