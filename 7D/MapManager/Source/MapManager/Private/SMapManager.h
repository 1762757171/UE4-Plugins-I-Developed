#pragma once

#include "CoreMinimal.h"
#include "MapManagerUtils.h"
#include "SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SMapManager : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMapManager) {}
        SLATE_ARGUMENT(TSharedPtr<class FMapManager>, MapManager)
        SLATE_ARGUMENT(TSharedPtr<class FMapLoader>, MapLoader)
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, const TSharedRef<class SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

    /** SWidget interface */
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
    void SaveDirtyMapTable();
    FText GetFilterText() const;
private:
    TSharedRef<SWidget> Construct_Region_Functions();
    /*    */TSharedRef<SWidget> Construct_Panel_Function();
    TSharedRef<SWidget> Construct_Region_MapTable();
    /*    */TSharedRef<SWidget> Construct_Table();
    /*        */TSharedRef<SWidget> Construct_Table_ContentPanel();
    /*            */TSharedRef<class SHeaderRow> Construct_Table_HeaderRow();
private:
    FReply RunDesignerLevel();
    FReply CommitAllDesign();
    FReply CommitDesignSelected();
    FReply GenerateWorld();
    FReply EditDesign();
    FReply ResetEditDesign();
private:
    void OnFilterTextChanged(const FText& NewText);
    TSharedRef<ITableRow> MakeRowWidget(FMapManagerRowListViewDataPtr InRowDataPtr, const TSharedRef<STableViewBase>& OwnerTable);

    void MeasureWidth();
    //Will NOT refresh list view, only refresh data
    void RefreshAvailableRows();
	//Will NOT refresh list view, only refresh data
    void RefreshVisibleRows();

    void ConstructFromDataArray(const TArray<FMapManagerRowListViewData>& NewData);

    bool IsMapManagerPanelCanUse() const;

    FReply AddNewRoom();
    FReply Reimport();
private:
    
    /** Array of the rows that are available for editing */
    TArray<FMapManagerRowListViewDataPtr> AvailableRows;

    /** Array of the rows that match the active filter(s) */
    TArray<FMapManagerRowListViewDataPtr> VisibleRows;

    float RoomWidth = 5.f;
    float DesignWidth = 5.f;
    float DevWidth = 5.f;

    FString FilterString;
    
    TSharedPtr<SListView<FMapManagerRowListViewDataPtr>> ContentListView;

    TSharedPtr<class FMapManager> MapManager;

    TSharedPtr<class FMapLoader> MapLoader;
};
