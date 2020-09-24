
#include "RoomRelationship/Public/RoomRelationshipAssetFactory.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"


#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"

URoomRelationshipAssetFactory::URoomRelationshipAssetFactory()
{
    bCreateNew = true;
    bEditAfterNew = true;
    SupportedClass = URoomRelationshipAsset::StaticClass();
}

UObject* URoomRelationshipAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                         EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    URoomRelationshipAsset* Asset =
        NewObject<URoomRelationshipAsset>(InParent, InClass, InName, Flags|RF_Transactional);
    return Asset;
}

FText URoomRelationshipAssetFactory::GetDisplayName() const
{
	return LOCTEXT("CreateAsset", "RoomRelationship");
}

#undef LOCTEXT_NAMESPACE
