

#pragma once
#include "LightViewer.h"
#include "SCompoundWidget.h"
#include "SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"


class SLightViewer : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLightViewer) {}
    SLATE_END_ARGS()

public:
    
    void Construct(const FArguments& InArgs, const TSharedRef<class SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

    ~SLightViewer();
    
private:

    void OnFilterTextChanged(const FText& NewText);

    FReply Event_OnRefreshButtonClicked();
    
    //Will NOT refresh list view, only refresh data
    void RefreshAvailableRows();
    
    //Will NOT refresh list view, only refresh data
    void RefreshVisibleRows();
    
    TSharedRef<ITableRow> MakeRowWidget(FLightViewerMapPtr InRowDataPtr, const TSharedRef<STableViewBase>& OwnerTable);

    FText GetSearchText() const;

    void Event_OnCheckStateChanged(ECheckBoxState NewState, FLightViewerMapPtr pData);
    
    void Event_OnBeginGame(const bool IsSimulating);
    
    void Event_OnEndGame(const bool IsSimulating);
    
private:
    TSharedRef<SWidget> Construct_MapList();
    /*    */TSharedRef<SWidget> Construct_Table_ContentPanel();
    /*        */TSharedRef<SHeaderRow> Construct_Table_HeaderRow();

private:
    TSharedPtr<SListView<FLightViewerMapPtr>> ContentListView;
    
    /** Array of the rows that are available for editing */
    TArray<FLightViewerMapPtr> AvailableRows;

    /** Array of the rows that match the active filter(s) */
    TArray<FLightViewerMapPtr> VisibleRows;

    TSharedPtr<SButton> ConfigButton;

    FString FilterString;

    float LevelColumnWidth = 128.0f;

    TSharedPtr<class FLightViewer> LightViewer;
    
    FDelegateHandle BeginGameDelegateHandle;
    
    FDelegateHandle EndGameDelegateHandle;
    
};


