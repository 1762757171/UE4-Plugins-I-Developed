// Copyright 

#pragma once

#include "AssetTypeActions_Base.h"
#include "CoreMinimal.h"

struct FRoomRelationshipAssetTypeAction : public FAssetTypeActions_Base
{
public:
    // IAssetTypeActions Implementation
    virtual FText GetName() const override;
    virtual FColor GetTypeColor() const override;
    virtual UClass* GetSupportedClass() const override;
    virtual uint32 GetCategories() override;

    virtual void OpenAssetEditor(const ::TArray<UObject*>& InObjects, ::TSharedPtr<IToolkitHost, ESPMode::Fast> EditWithinLevelEditor) override;
};
