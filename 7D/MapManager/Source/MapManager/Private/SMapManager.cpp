

#include "SMapManager.h"
#include "EditorStyleSet.h"
#include "FileHelpers.h"
#include "FontMeasure.h"
#include "GameMapsSettings.h"
#include "HorizontalBox.h"
#include "SButton.h"
#include "SSplitter.h"
#include "STextBlock.h"
#include "SHeaderRow.h"
#include "SScrollBox.h"
#include "SSearchBox.h"
#include "SListView.h"
#include "MapManagerUtils.h"
#include "ITypedTableView.h"
#include "MapLoader.h"
#include "MapManager.h"
#include "MapManagerModule.h"
#include "SImage.h"
#include "SlateApplication.h"
#include "SMapManagerListViewRow.h"
#include "MessageDialog.h"
#include "SlateIcon.h"
#include "MapManagerUtils.h"
#include "SDData.h"
#include "SDGameInstance.h"
#include "SMapLoader.h"
#include "WorldBrowserModule.h"
#include "Engine/LevelStreaming.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"


struct FDTSubLevelNode;

void SMapManager::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                            const TSharedPtr<SWindow>& ConstructUnderWindow)
{
    MapManager = InArgs._MapManager;
    MapLoader = InArgs._MapLoader;

    RefreshAvailableRows();
    
    this->ChildSlot
    [
        SNew(SSplitter)
            .Orientation(EOrientation::Orient_Vertical)
            .IsEnabled(this, &SMapManager::IsMapManagerPanelCanUse)
            + SSplitter::Slot()
                .Value(0.7f)
                [
                    Construct_Region_Functions()
                ]
            + SSplitter::Slot()
                .Value(0.3f)
                [
                    Construct_Region_MapTable()
                ]
    ];
}

void SMapManager::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
}

TSharedRef<SWidget> SMapManager::Construct_Region_Functions()
{
    return
    SNew(SVerticalBox)
        + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SBorder)
                    .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
                    .BorderBackgroundColor( FLinearColor( .6, .6, .6, 1.0f ) )
                    .Padding( 3.0f )
                    .HAlign(HAlign_Center)
                    [
                        SNew( STextBlock )
                            .Text( LOCTEXT("Functoins","Functions") )
                            .Font( FEditorStyle::GetFontStyle( "BoldFont" ) )
                            .ShadowOffset( FVector2D( 1.0f, 1.0f ) )
                    ]
                        
            ]
        + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SScrollBox)
                    .Orientation(Orient_Vertical)
                    +SScrollBox::Slot()
                        .VAlign(VAlign_Fill)
                        .HAlign(HAlign_Fill)
                        [
                            SNew(SBorder)
                                .Padding( 3.0f )
                                .HAlign(HAlign_Fill)
                                .VAlign(VAlign_Fill)
                                [
                                    Construct_Panel_Function()
                                ]
                        ]
            ]
    ;
}

TSharedRef<SWidget> SMapManager::Construct_Panel_Function()
{
    return
    
    SNew(SVerticalBox)
    + SVerticalBox::Slot()
        .Padding(FMargin(0.0f, 3.0f))
        .AutoHeight()
        [
            SNew( SButton )
                .OnClicked(this, &SMapManager::RunDesignerLevel)
                .ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
                .ContentPadding(FMargin(30,15))
                .ToolTipText(LOCTEXT("RunDesigner", "Run the game for level designers."))
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                    .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                    .Text(LOCTEXT("PlayDesignMap", "Play designers' game"))
                ]
        ]
    + SVerticalBox::Slot()
        .Padding(FMargin(0.0f, 3.0f))
        .AutoHeight()
        [
            SNew( SButton )
                .OnClicked(this, &SMapManager::CommitAllDesign)
                .ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
                .ContentPadding(FMargin(30,15))
                .ToolTipText(LOCTEXT("CommitDesignToolTip", "Commit all designer maps into develop maps."))
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                    .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                    .Text(LOCTEXT("CommitAllDesign", "Commit all design maps"))
                ]
        ]
    + SVerticalBox::Slot()
        .Padding(FMargin(0.0f, 3.0f))
        .AutoHeight()
        [
            SNew( SButton )
                .OnClicked(this, &SMapManager::CommitDesignSelected)
                .ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
                .ContentPadding(FMargin(30,15))
                .ToolTipText(LOCTEXT("CommitSelectedDesignToolTip", "Commit selected designer map into develop map."))
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                    .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                    .Text(LOCTEXT("CommitDesign", "Commit selected design map"))
                ]
        ]
    + SVerticalBox::Slot()
        .Padding(FMargin(0.0f, 3.0f))
        .AutoHeight()
        [
            SNew( SButton )
                .OnClicked(this, &SMapManager::GenerateWorld)
                .ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
                .ContentPadding(FMargin(30,15))
                .ToolTipText(LOCTEXT("CheckMapTableToolTip", "Recreate the world."))
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                    .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                    //VERSION 1.0
                    .Text(LOCTEXT("CheckMapTable", "CREATE THE WORLD! (v1.7)"))
                ]
        ]
    + SVerticalBox::Slot()
        .Padding(FMargin(0.0f, 3.0f))
        .AutoHeight()
        [
            SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                    .FillWidth(0.6f)
                    [
                        SNew( SButton )
                            .OnClicked(this, &SMapManager::SMapManager::EditDesign)
                            .ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
                            .ContentPadding(FMargin(30,15))
                            .ToolTipText(LOCTEXT("OpenEditDesignToolTip", "A fast villa loader."))
                            .VAlign(VAlign_Fill)
                            [
                                SNew(STextBlock)
                                .Justification(ETextJustify::Center)
                                .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                                .Text(LOCTEXT("OpenEditDesigningVilla", "Villa Loader"))
                            ]
                    ]
                +SHorizontalBox::Slot()
                    .FillWidth(0.4f)
                    [
                        SNew( SButton )
                            .OnClicked(this, &SMapManager::SMapManager::ResetEditDesign)
                            .ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
                            .ContentPadding(FMargin(30,5))
                            .ToolTipText(LOCTEXT("VillaResetToolTip", "A fast villa loader which only open minimal maps."))
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Justification(ETextJustify::Center)
                                .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                                .Text(LOCTEXT("ResetVilla", "Villa Loader [Minimal]"))
                            ]
                    ]
        ]
    ;
}

TSharedRef<SWidget> SMapManager::Construct_Region_MapTable()
{
    return
    
    SNew(SVerticalBox)
        + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SBorder)
                    .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
                    .BorderBackgroundColor( FLinearColor( .6, .6, .6, 1.0f ) )
                    .Padding( 3.0f )
                    .HAlign(HAlign_Center)
                    [
                        SNew( STextBlock )
                            .Text( LOCTEXT("MapTable","Map Table") )
                            .Font( FEditorStyle::GetFontStyle( "BoldFont" ) )
                            .ShadowOffset( FVector2D( 1.0f, 1.0f ) )
                    ]
            ]
        +SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                Construct_Table()
            ]
    ;
}

TSharedRef<SWidget> SMapManager::Construct_Table()
{
    return

    SNew(SBorder)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .Padding(4.0f)
        [
            Construct_Table_ContentPanel()
        ]
    ;
}

TSharedRef<SWidget> SMapManager::Construct_Table_ContentPanel()
{
    TSharedRef<SScrollBar> HorizontalScrollBar =
        SNew(SScrollBar)
            .Orientation(Orient_Horizontal)
            .Thickness(FVector2D(12.0f, 12.0f))
    ;

    TSharedRef<SScrollBar> VerticalScrollBar =
        SNew(SScrollBar)
            .Orientation(Orient_Vertical)
            .Thickness(FVector2D(12.0f, 12.0f))
    ;

    ContentListView =
        SNew(SListView<FMapManagerRowListViewDataPtr>)
            .ListItemsSource(&VisibleRows)
            .HeaderRow(Construct_Table_HeaderRow())
            .OnGenerateRow(this, &SMapManager::MakeRowWidget)
            //.OnSelectionChanged(this, &FDataTableEditor::OnRowSelectionChanged)
            .ExternalScrollbar(VerticalScrollBar)
            .ConsumeMouseWheel(EConsumeMouseWheel::Always)
            .SelectionMode(ESelectionMode::Single)
            .AllowOverscroll(EAllowOverscroll::No);

    return
    SNew(SVerticalBox)
        + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(SSearchBox)
                            .OnTextChanged(this, &SMapManager::OnFilterTextChanged)
                    ]
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(5.f, 0.f)
                        [
                            SNew(SButton)
                                .OnClicked(this, &SMapManager::AddNewRoom)
                                .ButtonStyle( FEditorStyle::Get(), "HoverHintOnly" )
                                .ToolTipText( LOCTEXT("AddNewRoom", "Add new room") )
                                .ContentPadding( 4.0f )
                                .ForegroundColor( FSlateColor::UseForeground() )
                                [    
                                    SNew( SImage )
                                    .Image( FSlateIcon(FEditorStyle::GetStyleSetName(), "DataTableEditor.Add").GetSmallIcon() )
                                    .ColorAndOpacity( FSlateColor::UseForeground() )
                                ]
                        ]
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(5.f, 0.f)
                        [
                            SNew(SButton)
                                .OnClicked(this, &SMapManager::Reimport)
                                .ButtonStyle( FEditorStyle::Get(), "HoverHintOnly" )
                                .ToolTipText( LOCTEXT("Reimport", "Reimport from config file") )
                                .ContentPadding( 4.0f )
                                .ForegroundColor( FSlateColor::UseForeground() )
                                [    
                                    SNew( SImage )
                                    .Image( FSlateIcon(FEditorStyle::GetStyleSetName(), "DataTableEditor.Copy").GetSmallIcon() )
                                    .ColorAndOpacity( FSlateColor::UseForeground() )
                                ]
                        ]
                    
            ]
        +SVerticalBox::Slot()
            [
                SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                        [
                            SNew(SScrollBox)
                                .Orientation(Orient_Horizontal)
                                .ExternalScrollbar(HorizontalScrollBar)
                                +SScrollBox::Slot()
                                    .HAlign(HAlign_Fill)
                                    [
                                        ContentListView.ToSharedRef()
                                    ]
                        ]
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            VerticalScrollBar
                        ]
            ]
        +SVerticalBox::Slot()
            .AutoHeight()
            [
                HorizontalScrollBar
            ]
    ;
}

TSharedRef<SHeaderRow> SMapManager::Construct_Table_HeaderRow()
{
    return
        SNew(SHeaderRow)
        +SHeaderRow::Column(FMapManagerRowListViewData::ColumnName_RoomId)
            .DefaultLabel(LOCTEXT("RoomLable", "Room"))
            .ManualWidth(RoomWidth)
        +SHeaderRow::Column(FMapManagerRowListViewData::ColumnName_Design)
            .DefaultLabel(LOCTEXT("DesignLable", "Design"))
            .ManualWidth(DesignWidth)
        +SHeaderRow::Column(FMapManagerRowListViewData::ColumnName_Develop)
            .DefaultLabel(LOCTEXT("DevelopLable", "Develop"))
            .ManualWidth(DevWidth)
    ;

}

FReply SMapManager::RunDesignerLevel()
{
    MapManager->RunDesignMap();

	return FReply::Handled();
}

FReply SMapManager::CommitAllDesign()
{
    MapManager->CommitAllDesign();
    
    return FReply::Handled();
}

FReply SMapManager::CommitDesignSelected()
{
    TArray<FMapManagerRowListViewDataPtr> Rows = ContentListView->GetSelectedItems();
    
    MapManager->CommitDesignSelected(Rows);
    
    return FReply::Handled();
}

FReply SMapManager::GenerateWorld()
{
    MapManager->RegenerateFromDevelopFolders();
    
    return FReply::Handled();
}

FReply SMapManager::EditDesign()
{
    FEditorFileUtils::SaveDirtyPackages(true, true, true);

    static const FName LoaderTabId("MapLoader");
    
    FGlobalTabmanager::Get()->InvokeTab(LoaderTabId);

    MapLoader->Initialize();
    MapLoader->OpenMap();
    
    return FReply::Handled();
}

FReply SMapManager::ResetEditDesign()
{
    UMapManagerPlaySettings* PlaySettings = GetMutableDefault<UMapManagerPlaySettings>();
    PlaySettings->LastLoadedMaps.Reset();
	
    FPropertyChangedEvent PropChangeEvent(UMapManagerPlaySettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMapManagerPlaySettings, LastLoadedMaps)));
    PlaySettings->PostEditChangeProperty(PropChangeEvent);
	
    PlaySettings->SaveConfig();

    return EditDesign();
}

void SMapManager::OnFilterTextChanged(const FText& NewText)
{
    FilterString = NewText.ToString();
    RefreshVisibleRows();
	ContentListView->RequestListRefresh();
}

FText SMapManager::GetFilterText() const
{
    return FText::FromString(FilterString);
}

TSharedRef<ITableRow> SMapManager::MakeRowWidget(FMapManagerRowListViewDataPtr InRowDataPtr,
                                                 const TSharedRef<STableViewBase>& OwnerTable)
{
    return
    SNew(SMapManagerListViewRow, OwnerTable)
        .RowDataPtr(InRowDataPtr)
        .MapManagerWidget(SharedThis(this))
    ;
}

void SMapManager::RefreshAvailableRows()
{
    const TArray<FMapManagerRowListViewData>& Map = MapManager->GetMapRows();
    ConstructFromDataArray(Map);
}

void SMapManager::RefreshVisibleRows()
{
	VisibleRows.Reset();
    if(FilterString.IsEmpty())
    {
		VisibleRows = AvailableRows;
    }
	else {
		for (const auto& ar : AvailableRows)
		{
			if (ar->RoomId.Contains(FilterString) ||
			    ar->DesignMap.Contains(FilterString) ||
			    ar->DevelopMap.Contains(FilterString))
			{
				VisibleRows.Add(ar);
			}
		}
	}

    MeasureWidth();
}

void SMapManager::ConstructFromDataArray(const TArray<FMapManagerRowListViewData>& NewData)
{
    AvailableRows.Reset();
    
    for(const auto& p : NewData)
    {
        AvailableRows.Add(MakeShareable(
            new FMapManagerRowListViewData(p)
            ));
    }

    RefreshVisibleRows();
}

bool SMapManager::IsMapManagerPanelCanUse() const
{
    return !FMapManagerModule::IsGamePlaying();
}

FReply SMapManager::AddNewRoom()
{
    TArray<FMapManagerRowListViewData> CollectedData;

    for(const auto& ar : AvailableRows)
    {
        CollectedData.Add(*(ar.Get()));
    }

    CollectedData.Add(FMapManagerRowListViewData(TEXT("NewRoom"), TEXT(""), TEXT("")));
    
    MapManager->WriteTableIntoFile(CollectedData);

    Reimport();

    return FReply::Handled();
}

FReply SMapManager::Reimport()
{
    MapManager->RefreshTableFromFile();
    RefreshAvailableRows();
	ContentListView->RequestListRefresh();

    return FReply::Handled();
}

void SMapManager::SaveDirtyMapTable()
{
    TArray<FMapManagerRowListViewData> CollectedData;

    for(const auto& ar : AvailableRows)
    {
		if (ar->RoomId == FMapManager::DELETED_ROOM_ID) {
			continue;
		}
        CollectedData.Add(*(ar.Get()));
    }
    
    /*if(MapManager->WriteTableIntoFile(CollectedData))
    {
        ConstructFromDataArray(CollectedData);
    }else
    {
        RefreshAvailableRows();
    }
	ContentListView->RequestListRefresh();*/

    //EDIT: 数据的 有效性和正确性 远比 运行效率 更重要
    
    MapManager->WriteTableIntoFile(CollectedData);

    Reimport();
}

void SMapManager::MeasureWidth()
{
    const float Padding = 10.f;
    
    TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FTextBlockStyle& CellTextStyle = FEditorStyle::GetWidgetStyle<FTextBlockStyle>("DataTableEditor.CellText");

    RoomWidth = FontMeasure->Measure(LOCTEXT("RoomLable", "Room"), CellTextStyle.Font).X + Padding/*reserve region for padding*/;
    DesignWidth = FontMeasure->Measure(LOCTEXT("DesignLable", "Design"), CellTextStyle.Font).X + Padding/*reserve region for padding*/;
    DevWidth = FontMeasure->Measure(LOCTEXT("DevelopLable", "Develop"), CellTextStyle.Font).X + Padding/*reserve region for padding*/;
    
    for(const auto& vr : VisibleRows)
    {
        const float CurrentRoomWidth = FontMeasure->Measure(vr->RoomId, CellTextStyle.Font).X + Padding;
        RoomWidth = FMath::Max(RoomWidth, CurrentRoomWidth);
        const float CurrentDesignWidth = FontMeasure->Measure(vr->DesignMap, CellTextStyle.Font).X + Padding;
        DesignWidth = FMath::Max(DesignWidth, CurrentDesignWidth);
        const float CurrentDevWidth = FontMeasure->Measure(vr->DevelopMap, CellTextStyle.Font).X + Padding;
        DevWidth = FMath::Max(DevWidth, CurrentDevWidth);
    }
}


#undef LOCTEXT_NAMESPACE
