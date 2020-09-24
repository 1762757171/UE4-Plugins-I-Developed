#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"


#include "RoomRelationshipGraph.generated.h"

UCLASS()
class URoomRelationshipGraph : public UEdGraph
{
    GENERATED_BODY()
public:
    URoomRelationshipAsset* GetRoomRelationshipAssetEdit() const;
    /**
    * Note that the object will be modified. If we are currently recording into the
    * transaction buffer (undo/redo), save a copy of this object into the buffer and
    * marks the package as needing to be saved.
    *
    * @param	bAlwaysMarkDirty	if true, marks the package dirty even if we aren't
    *								currently recording an active undo/redo transaction
    * @return true if the object was saved to the transaction buffer
    */
    //virtual bool Modify(bool bAlwaysMarkDirty = true) override;

    virtual void NotifyGraphChanged() override;
    virtual void NotifyGraphChanged(const FEdGraphEditAction& Action) override;

private:
    void UpdateAsset();
};
