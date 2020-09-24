#include "SRoomInfoDetail.h"

#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"
#include "RoomRelationship/Classes/RoomRelationshipAsset.h"
#include "ModuleManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Private/SDetailsView.h"

void SRoomInfoDetail::Construct(const FArguments& InArgs, URoomRelationshipAsset* Asset)
{
    FDetailsViewArgs DetailsViewArgs(
        true,
        false,
        true,
        FDetailsViewArgs::ObjectsUseNameArea,
        false,
        GUnrealEd );
    
    // create the detail view widget
    DetailView_Node = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView( DetailsViewArgs );
    
    DetailView_Asset = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView( DetailsViewArgs );
    DetailView_Asset->SetObject(Asset, true);
    
    this->ChildSlot
    [
        SNew(SSplitter)
            .Orientation(Orient_Vertical)
            +SSplitter::Slot()
                .Value(.2)
                [
                    DetailView_Asset.ToSharedRef()
                ]
            +SSplitter::Slot()
                .Value(.3)
                [
                    DetailView_Node.ToSharedRef()
                ]
    ]
    ;
}

void SRoomInfoDetail::SetSelection(FGraphPanelSelectionSet SelectedGraphNodes)
{
    TArray<UObject*> Objs;
    for(auto n : SelectedGraphNodes)
    {
        Objs.Add(n);
    }
    DetailView_Node->SetObjects(Objs, true);
}
