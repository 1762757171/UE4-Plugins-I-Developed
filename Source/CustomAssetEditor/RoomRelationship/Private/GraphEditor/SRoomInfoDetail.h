#pragma once
#include "DeclarativeSyntaxSupport.h"
#include "GraphEditor.h"
#include "SCompoundWidget.h"

class SRoomInfoDetail : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SRoomInfoDetail) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, class URoomRelationshipAsset* Asset);

    void SetSelection(FGraphPanelSelectionSet SelectedGraphNodes);
private:
    TSharedPtr<class IDetailsView> DetailView_Node;
    TSharedPtr<class IDetailsView> DetailView_Asset;
};
