
#include "SMapLoader.h"
#include "SSearchBox.h"
#include "EditorStyleSet.h"
#include "MapLoader.h"
#include "SButton.h"
#include "SlateIcon.h"
#include "SMapLoaderTreeViewRow.h"
#include "SImage.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"

const FName SMapLoader::Column_Load("Load");
const FName SMapLoader::Column_Map("Map");

void SMapLoader::Construct(const FArguments& InArgs, const TSharedRef<class SDockTab>& ConstructUnderMajorTab, const TSharedPtr<SWindow>& ConstructUnderWindow)
{
    MapLoader = InArgs._Loader;

    RefreshVisibleRows();

    this->ChildSlot
    [
        SNew(SBorder)
            .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
            .BorderBackgroundColor( FLinearColor( .6, .6, .6, 1.0f ) )
            .Padding( 3.0f )
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .IsEnabled(this, &SMapLoader::IsDevelopMap)
            [
                SNew(SVerticalBox)
                    +SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                    .FillWidth(1.0f)
                                    [
                                        SNew(SSearchBox)
                                            .OnTextChanged(this, &SMapLoader::OnFilterTextChanged)
                                    ]
                                +SHorizontalBox::Slot()
                                    .AutoWidth()
                                    [
                                        SNew(SButton)
                                            .ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
                                            .OnClicked(this, &SMapLoader::Event_OnRefreshButtonClicked)
                                            [
                                                SNew( SImage )
                                                    .Image( FSlateIcon(FEditorStyle::GetStyleSetName(), "DataTableEditor.Add").GetSmallIcon() )
                                                    .ColorAndOpacity( FSlateColor::UseForeground() )
                                            ]
                                    ]
                        ]

                    +SVerticalBox::Slot()
                        .FillHeight(1.0f)
                        [
                            Construct_Table_ContentPanel()
                        ]
            ]
        ]
    ;

	MapLoader->OnLoaderFlushed.AddSP(this, &SMapLoader::RefreshWhenFlush);
}

TSharedRef<SWidget> SMapLoader::Construct_Table_ContentPanel()
{
    TSharedRef<SScrollBar> VerticalScrollBar =
        SNew(SScrollBar)
            .Orientation(Orient_Vertical)
            .Thickness(FVector2D(12.0f, 12.0f))
    ;

    ContentTreeView =
        SNew(STreeView<FMapLevelLoaderDataPtr>)
            .TreeItemsSource(&VisibleRows)
            .HeaderRow(Construct_Table_HeaderRow())
            .OnGenerateRow(this, &SMapLoader::MakeRowWidget)
            .OnGetChildren(this, &SMapLoader::GetChildrenForTree)
            //.OnSelectionChanged(this, &FDataTableEditor::OnRowSelectionChanged)
            .ExternalScrollbar(VerticalScrollBar)
            .ConsumeMouseWheel(EConsumeMouseWheel::Always)
            .SelectionMode(ESelectionMode::Single)
            .AllowOverscroll(EAllowOverscroll::No);

    return
    SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                ContentTreeView.ToSharedRef()
            ]
        +SHorizontalBox::Slot()
            .AutoWidth()
            [
                VerticalScrollBar
            ]
    ;
}

TSharedRef<SHeaderRow> SMapLoader::Construct_Table_HeaderRow()
{
    return
        SNew(SHeaderRow)
        +SHeaderRow::Column(Column_Load)
            .DefaultLabel(LOCTEXT("Design_LoadLable", "Load?"))
            .ManualWidth(64.0f)
        +SHeaderRow::Column(Column_Map)
            .DefaultLabel(LOCTEXT("Design_MapLable", "Map Asset"))
            .FillWidth(1.0f)
    ;

}

void SMapLoader::OnFilterTextChanged(const FText& NewText)
{
    FilterString = NewText.ToString();
    MapLoader->SetFilter(FilterString);
    RefreshVisibleRows();
    ContentTreeView->RequestListRefresh();

}

void SMapLoader::RefreshVisibleRows()
{
    MapLoader->Initialize();
    VisibleRows = MapLoader->GenerateData();
    if(!FilterString.IsEmpty())
    {
        //Expand all
        TArray<FMapLevelLoaderDataPtr> Data = VisibleRows;
        TArray<FMapLevelLoaderDataPtr> Children = Data;
        while(true)
        {
            TArray<FMapLevelLoaderDataPtr> NewChildren;
            for(const auto& d : Children)
            {
                NewChildren.Append(d->Children);
            }
            
            Data.Append(NewChildren);
            Children = NewChildren;
            if(Children.Num() == 0)
            {
                break;
            }
        }
        for(const auto& c : Data)
        {
            ContentTreeView->SetItemExpansion(c, true);
        }
    }
}

FText SMapLoader::GetFilterText() const
{
    return FText::FromString(FilterString);
}

bool SMapLoader::IsDevelopMap() const
{
    return FMapLoader::IsInTargetLevel();
}

void SMapLoader::Expand(TSet<FString>& Folders, const FMapLevelLoaderDataPtr& LoaderData)
{
    for(const auto& c : LoaderData->Children)
    {
        Expand(Folders, c);
    }
    if(Folders.Contains(LoaderData->LevelStreaming_or_HierarchyName))
    {
        ContentTreeView->SetItemExpansion(LoaderData, true);
        Folders.Remove(LoaderData->LevelStreaming_or_HierarchyName);
    }
}

TSharedRef<ITableRow> SMapLoader::MakeRowWidget(FMapLevelLoaderDataPtr InRowDataPtr,
                                                const TSharedRef<STableViewBase>& OwnerTable)
{
    return
    SNew(SMapLoaderTreeViewRow, OwnerTable)
        .RowDataPtr(InRowDataPtr)
        .Loader(MapLoader)
        .OuterWidget(SharedThis(this))
    ;
}

void SMapLoader::GetChildrenForTree(FMapLevelLoaderDataPtr Item, TArray<FMapLevelLoaderDataPtr>& OutChildren)
{
    OutChildren = Item->Children;
}

void SMapLoader::RefreshWhenFlush()
{
	RefreshVisibleRows();
    //ContentTreeView->RequestListRefresh();
    //ContentTreeView->RequestTreeRefresh();
    TSet<FMapLevelLoaderDataPtr> Expanded;
    ContentTreeView->GetExpandedItems(Expanded);
    TSet<FString> LevelsExpanded;
    for(const auto& e : Expanded)
    {
        LevelsExpanded.Add(e->LevelStreaming_or_HierarchyName);
    }

    ContentTreeView->RequestTreeRefresh();
    
    for(const auto& vr : VisibleRows)
    {
        Expand(LevelsExpanded, vr);
    }
}

FReply SMapLoader::Event_OnRefreshButtonClicked()
{
    RefreshWhenFlush();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
