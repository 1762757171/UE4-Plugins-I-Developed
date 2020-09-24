

#include "RoomRelationship/Public/RoomRelationshipAssetTypeAction.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "RoomRelationship/Public/RoomRelationshipAssetEditor.h"


#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"

FText FRoomRelationshipAssetTypeAction::GetName() const
{
    return LOCTEXT("AssetTypeAction", "7D Rooms");
}

FColor FRoomRelationshipAssetTypeAction::GetTypeColor() const
{
    return FColor(255, 192, 203);
}

UClass* FRoomRelationshipAssetTypeAction::GetSupportedClass() const
{
    return URoomRelationshipAsset::StaticClass();
}

uint32 FRoomRelationshipAssetTypeAction::GetCategories()
{
    return EAssetTypeCategories::Gameplay;
}

void FRoomRelationshipAssetTypeAction::OpenAssetEditor(const ::TArray<UObject*>& InObjects,
    ::TSharedPtr<IToolkitHost, ESPMode::Fast> EditWithinLevelEditor)
{
    for (auto Object : InObjects)
    {
        auto RoomRelationshipAsset = Cast<URoomRelationshipAsset>(Object);
        if (RoomRelationshipAsset != nullptr)
        {
            TSharedRef<FRoomRelationshipAssetEditor> Editor(new FRoomRelationshipAssetEditor);
            Editor->InitRoomRelationshipAssetEditor(EditWithinLevelEditor, RoomRelationshipAsset);
        }
    }
}


#undef LOCTEXT_NAMESPACE
