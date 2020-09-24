#pragma once

#include "CoreMinimal.h"
#include "SevenDaysDeveloperModule.h"
#include "SListView.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "SevenDaysDeveloperUtils.h"
#include "SMenuAnchor.h"

class SSevenDaysDeveloper : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SSevenDaysDeveloper) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, const TSharedRef<class SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

    /** SWidget interface */
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

public:
    FReply LoadLevelButtonClick();
    FReply TeleportRoomButtonClick();
    
private:
    TSharedRef<SWidget> Construct_Region_RuntimeLevel();
    /*    */TSharedRef<SWidget> Construct_Panel_LoadedLevel();
    /*        */TSharedRef<SWidget> Construct_Table_LoadedLevel();
    /*    */TSharedRef<SWidget> Construct_Panel_OperateLevel();
    TSharedRef<SWidget> Construct_Region_RuntimeGameplay();

private:
    void Tick_LevelRows();
    void Tick_VisibleRows();
    TSharedRef<ITableRow> MakeRowWidget(FSevenDaysDeveloperRowDataPtr InRowDataPtr, const TSharedRef<STableViewBase>& OwnerTable);

    void OnLevelsFilterTextChanged(const FText& NewText);

    void RequireRefreshThisFrame();
public:
    bool IsAtRuntime() const;

    FText GetFilterText() const;

private:
    TArray<FSevenDaysDeveloperRowDataPtr> LevelRows;
    TArray<FSevenDaysDeveloperRowDataPtr> VisibleRows;
    TSharedPtr<SListView<FSevenDaysDeveloperRowDataPtr>> ContentListView;
    TSharedPtr<SMenuAnchor> InputMenuAnchor;

    FString FilterString;
    bool bRequireRefresh = true;

    TArray<FName> t_Level_Loaded_Visible;
    TArray<FName> t_Level_Loaded_Invisible;
    TArray<FName> t_Level_Unloaded;

    TMap<FName, FVector> Room_Transport;
};

