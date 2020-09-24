
#include "SSevenDaysDeveloper.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "MessageDialog.h"
#include "SButton.h"
#include "SDGameInstance.h"
#include "SEditableTextBox.h"
#include "SSearchBox.h"
#include "SSevenDaysDeveloperListViewRow.h"
#include "SSevenDaysDeveloper_Gameplay.h"
#include "Engine/LevelStreaming.h"
#include "SevenDaysDeveloperRuntime/Private/SevenDaysDeveloperRuntimeUtils.h"
#include "SevenDaysDeveloperRuntime/Public/SevenDaysDeveloperRuntimeModule.h"

#define LOCTEXT_NAMESPACE "SevenDaysDeveloperModule"

void SSevenDaysDeveloper::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                                   const TSharedPtr<SWindow>& ConstructUnderWindow)
{
    RequireRefreshThisFrame();
    
    this->ChildSlot
    [
        SNew(SBorder)
            .Padding(3.0f)
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            [
                SNew(SSplitter)
                    .Orientation(EOrientation::Orient_Vertical)
                    .IsEnabled(this, &SSevenDaysDeveloper::IsAtRuntime)
                    + SSplitter::Slot()
                        .Value(0.5f)
                        [
                            Construct_Region_RuntimeLevel()
                        ]
                    +SSplitter::Slot()
                        .Value(0.5f)
                        [
                            Construct_Region_RuntimeGameplay()
                        ]

            ]
    ];
}

void SSevenDaysDeveloper::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    Tick_LevelRows();
    Tick_VisibleRows();

    if(bRequireRefresh)
    {
        ContentListView->RequestListRefresh();
        bRequireRefresh = false;
    }
}

FReply SSevenDaysDeveloper::LoadLevelButtonClick()
{
    TArray<FSevenDaysDeveloperRowDataPtr> Selections;
    ContentListView->GetSelectedItems(Selections);
    if(Selections.Num() == 0)
    {
        FMessageDialog::Open(
            EAppMsgType::Ok,
            LOCTEXT("Prompt_LoadLevelNoSelection", "Please select levels to load." )
        );
        return FReply::Handled();
    }
    TArray<FName> RoomNames;
    for(const auto& s : Selections)
    {
        RoomNames.Add(s->RoomName);
    }
    
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.LoadLevel(RoomNames);

    return FReply::Handled();
}

FReply SSevenDaysDeveloper::TeleportRoomButtonClick()
{
    TArray<FSevenDaysDeveloperRowDataPtr> Selections;
    ContentListView->GetSelectedItems(Selections);
    if(Selections.Num() != 1)
    {
        FMessageDialog::Open(
            EAppMsgType::Ok,
            LOCTEXT("Prompt_TransportNoSelection", "Please select only one level to transport." )
        );
        return FReply::Handled();
    }

    FSevenDaysDeveloperRowDataPtr Selection = Selections[0];
    
    FName RoomId = Selection->RoomName;
    
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.TeleportLevel(RoomId);

    return FReply::Handled();
}

TSharedRef<SWidget> SSevenDaysDeveloper::Construct_Region_RuntimeLevel()
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
                            .Text( LOCTEXT("RuntimeLevel","Runtime Level") )
                            .Font( FEditorStyle::GetFontStyle( "BoldFont" ) )
                            .ShadowOffset( FVector2D( 1.0f, 1.0f ) )
                    ]
            ]
        + SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SBorder)
                    .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
                    .BorderBackgroundColor( FLinearColor( .4, .4, .4, 1.0f ) )
                    .Padding( 3.0f )
                    .HAlign(HAlign_Fill)
                    [
                        SNew(SSplitter)
                            .Orientation(Orient_Horizontal)
                            +SSplitter::Slot()
                                .Value(0.5f)
                                [
                                    Construct_Panel_LoadedLevel()
                                ]
                            +SSplitter::Slot()
                                .Value(0.5f)
                                [
                                    Construct_Panel_OperateLevel()
                                ]
                    ]
            ]
    ;
}

TSharedRef<SWidget> SSevenDaysDeveloper::Construct_Panel_LoadedLevel()
{
    return
    SNew(SVerticalBox)
        +SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SSearchBox)
                    .OnTextChanged(this, &SSevenDaysDeveloper::OnLevelsFilterTextChanged)
            ]
        +SVerticalBox::Slot()
            .FillHeight(1.0f)
            [
                SNew(SBorder)
                    .Padding( 3.0f )
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                    [
                      Construct_Table_LoadedLevel()
                    ]
            ]
    
    ;
}

TSharedRef<SWidget> SSevenDaysDeveloper::Construct_Table_LoadedLevel()
{
    TSharedRef<SScrollBar> VerticalScrollBar =
        SNew(SScrollBar)
            .Orientation(Orient_Vertical)
            .Thickness(FVector2D(12.0f, 12.0f))
    ;

    TSharedRef<SHeaderRow> Header =
        SNew(SHeaderRow)
        +SHeaderRow::Column(TEXT("LoadedRooms"))
            .DefaultLabel(LOCTEXT("RoomLable", "Rooms loaded"))
            .FillWidth(1.0f)
    ;

    ContentListView =
        SNew(SListView<FSevenDaysDeveloperRowDataPtr>)
            .HeaderRow(Header)
            .ListItemsSource(&VisibleRows)
            .OnGenerateRow(this, &SSevenDaysDeveloper::MakeRowWidget)
            //.OnSelectionChanged(this, &FDataTableEditor::OnRowSelectionChanged)
            .ExternalScrollbar(VerticalScrollBar)
            .ConsumeMouseWheel(EConsumeMouseWheel::Always)
            .SelectionMode(ESelectionMode::Multi)
            .AllowOverscroll(EAllowOverscroll::No)
    ;

    return 
    SNew(SHorizontalBox)
        +SHorizontalBox::Slot()
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .FillWidth(1.0f)
            [
                ContentListView.ToSharedRef()
            ]
        +SHorizontalBox::Slot()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Fill)
            .AutoWidth()
            [
                VerticalScrollBar
            ]
    ;
}

TSharedRef<SWidget> SSevenDaysDeveloper::Construct_Panel_OperateLevel()
{
    return
    SNew(SBorder)
        .Padding( 3.0f )
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Top)
                    [
                        SNew( SButton )
                            .OnClicked(this, &SSevenDaysDeveloper::LoadLevelButtonClick)
                            .ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
                            .ContentPadding(FMargin(0,10))
                            .ToolTipText(LOCTEXT("LoadRoomToolTip", "Load selected rooms and show them."))
                            [
                                SNew(STextBlock)
                                .Justification(ETextJustify::Center)
                                .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                                .Text(LOCTEXT("LoadRoom", "Load rooms"))
                            ]
                    ]
                + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Top)
                    [
                        SNew( SButton )
                            .OnClicked(this, &SSevenDaysDeveloper::TeleportRoomButtonClick)
                            .ButtonStyle(FEditorStyle::Get(), "FlatButton.Primary")
                            .ContentPadding(FMargin(0,10))
                            .ToolTipText(LOCTEXT("TransportRoomToolTip", "Transport to selected room."))
                            [
                                SNew(STextBlock)
                                .Justification(ETextJustify::Center)
                                .TextStyle(FEditorStyle::Get(), "ContentBrowser.TopBar.Font")
                                .Text(LOCTEXT("TransportRoom", "Transport to room."))
                            ]
                    ]
        ]
    ;
}

TSharedRef<SWidget> SSevenDaysDeveloper::Construct_Region_RuntimeGameplay()
{
    return
    SNew(SSevenDaysDeveloper_Gameplay)
    ;
            
}

void SSevenDaysDeveloper::Tick_LevelRows()
{
    //Collect levels loaded now
    //visible      -> green
    //invisible    -> yellow

	//@TODO: Fix this with new api

    UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
    if(!PlayWorld)
    {
        LevelRows.Empty();
        Room_Transport.Empty();
        RequireRefreshThisFrame();
        return;
    }

    USDGameInstance* SDGI = PlayWorld->GetGameInstance<USDGameInstance>();
    if(!SDGI)
    {
        LevelRows.Empty();
        Room_Transport.Empty();
        RequireRefreshThisFrame();
        return;
    }
    
    const TArray<ULevelStreaming*>& StreamingLevels = PlayWorld->GetStreamingLevels();

    TArray<FName> Level_Loaded_Visible;
    TArray<FName> Level_Loaded_Invisible;
    TArray<FName> Level_Unloaded;
    
    for(const auto& l : StreamingLevels)
    {
        FString LevelNameStr = l->GetWorldAssetPackageName();
        if(LevelNameStr.Contains(TEXT("Dynamic")))
        {
            continue;
        }
        bool bLoaded = l->IsLevelLoaded();
        bool bVisible = l->IsLevelVisible();
        LevelNameStr = LevelNameStr.Replace(TEXT("UEDPIE_0_"), TEXT(""));
        FName LevelName(*LevelNameStr);
        if(bLoaded && bVisible)
        {
            Level_Loaded_Visible.Add(LevelName);
        }else if(bLoaded)
        {
            Level_Loaded_Invisible.Add(LevelName);
        }else
        {
            Level_Unloaded.Add(LevelName);
        }
    }

    if(t_Level_Unloaded == Level_Unloaded && t_Level_Loaded_Invisible == Level_Loaded_Invisible && t_Level_Loaded_Visible == Level_Loaded_Visible)
    {
        return;
    }
    t_Level_Unloaded = Level_Unloaded;
    t_Level_Loaded_Invisible = Level_Loaded_Invisible;
    t_Level_Loaded_Visible = Level_Loaded_Visible;
    
    LevelRows.Empty();
    
    static const auto ConvertLevelNameToRoomName =
        [](const TArray<FName>& InNames, TArray<FName>& OutNames)
        {

            OutNames.Empty();
            
            for(auto& li : InNames)
                {
                    FString LongPath = li.ToString();
                    int32 SlashIndex;
                    LongPath.FindLastChar(TEXT('/'), SlashIndex);

                    OutNames.Add( * LongPath.Mid(SlashIndex + 1) );
                }

        };

    TArray<FName> Room_Loaded_Visible;
    TArray<FName> Room_Loaded_Invisible;
    TArray<FName> Room_Unloaded;
    

    ConvertLevelNameToRoomName(Level_Loaded_Visible, Room_Loaded_Visible);
    ConvertLevelNameToRoomName(Level_Loaded_Invisible, Room_Loaded_Invisible);
    ConvertLevelNameToRoomName(Level_Unloaded, Room_Unloaded);

    static const auto RefreshFromRoomData =
        [this](const TArray<FName>& Rooms, bool bLoaded, bool bVisible)
        {
            for(const auto& n : Rooms)
            {
                FSevenDaysDeveloperRowDataPtr DataPtr = MakeShareable(new FSevenDaysDeveloperRowData(n, bLoaded, bVisible));
                LevelRows.Add(DataPtr);
            }
        };

    RefreshFromRoomData(Room_Loaded_Visible, true, true);
    RefreshFromRoomData(Room_Loaded_Invisible, true, false);
    RefreshFromRoomData(Room_Unloaded, false, false);
    
    RequireRefreshThisFrame();
}

void SSevenDaysDeveloper::Tick_VisibleRows()
{
    if(FilterString.IsEmpty())
    {
        VisibleRows = LevelRows;
    }else
    {
        VisibleRows.Reset();
        for(const auto& lr : LevelRows)
        {
            if(lr->RoomName.ToString().Contains(FilterString))
            {
                VisibleRows.Add(lr);
            }
        }
    }
}

TSharedRef<ITableRow> SSevenDaysDeveloper::MakeRowWidget(FSevenDaysDeveloperRowDataPtr InRowDataPtr,
                                                         const TSharedRef<STableViewBase>& OwnerTable)
{
    return
    SNew(SSevenDaysDeveloperListViewRow, OwnerTable)
        .RowDataPtr(InRowDataPtr)
        .Parent(SharedThis(this))
    ;
}

void SSevenDaysDeveloper::OnLevelsFilterTextChanged(const FText& NewText)
{
    FilterString = NewText.ToString();
    RequireRefreshThisFrame();
}

void SSevenDaysDeveloper::RequireRefreshThisFrame()
{
    bRequireRefresh = true;
}

bool SSevenDaysDeveloper::IsAtRuntime() const
{
	return SevenDaysDeveloperRuntimeUtils::IsAtRuntime();
}

FText SSevenDaysDeveloper::GetFilterText() const
{
    return FText::FromString(FilterString);
}

#undef LOCTEXT_NAMESPACE
	