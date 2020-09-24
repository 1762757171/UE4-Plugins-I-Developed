#pragma once

#include "CoreMinimal.h"
#include "MapLoader.h"
#include "MapManagerUtils.h"
#include "SListView.h"
#include "STreeView.h"
#include "Engine/LevelStreaming.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"

class SMapLoader : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SMapLoader) {}
        SLATE_ARGUMENT(TSharedPtr<class FMapLoader>, Loader)
    SLATE_END_ARGS()

private:
    TSharedPtr<class FMapLoader> MapLoader;
public:
    void Construct(const FArguments& InArgs, const TSharedRef<class SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow);

private:
    TSharedRef<SWidget> Construct_Table_ContentPanel();
    /*        */TSharedRef<class SHeaderRow> Construct_Table_HeaderRow();
public:
    void OnFilterTextChanged(const FText& NewText);
    void RefreshVisibleRows();

    FText GetFilterText() const;
    bool IsDevelopMap() const;

    void Expand(TSet<FString>& Folders, const FMapLevelLoaderDataPtr& LoaderData);
private:
    TSharedRef<ITableRow> MakeRowWidget(FMapLevelLoaderDataPtr InRowDataPtr, const TSharedRef<STableViewBase>& OwnerTable);
    void GetChildrenForTree(FMapLevelLoaderDataPtr Item, TArray<FMapLevelLoaderDataPtr>& OutChildren);
    void RefreshWhenFlush();
    FReply Event_OnRefreshButtonClicked();

private:
    FString FilterString;
    TSharedPtr<STreeView<FMapLevelLoaderDataPtr>> ContentTreeView;
    TArray<FMapLevelLoaderDataPtr> VisibleRows;

public:
    static const FName Column_Load;
    static const FName Column_Map;
};

