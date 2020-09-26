
#include "SLightViewer.h"


#include "Editor.h"
#include "EditorStyleSet.h"
#include "FontMeasure.h"
#include "LightViewerConfig.h"
#include "ModuleManager.h"
#include "SButton.h"
#include "SCheckBox.h"
#include "SlateApplication.h"
#include "SScrollBox.h"
#include "SSearchBox.h"
#include "STextBlock.h"
#include "VerticalBox.h"
#include "SLightViewerListViewRow.h"


#define LOCTEXT_NAMESPACE "FLightViewerModule"


void SLightViewer::Construct(const FArguments& InArgs, const TSharedRef<SDockTab>& ConstructUnderMajorTab,
                             const TSharedPtr<SWindow>& ConstructUnderWindow)
{
    
    this->ChildSlot
    [
        Construct_MapList()
    ];

    RefreshAvailableRows();
    RefreshVisibleRows();
    ContentListView->RequestListRefresh();
    
    BeginGameDelegateHandle = FEditorDelegates::PostPIEStarted.AddSP(this, &SLightViewer::Event_OnBeginGame);
    if(GEditor->PlayWorld)
    {
        Event_OnBeginGame(false);
    }
    
    EndGameDelegateHandle = FEditorDelegates::PrePIEEnded.AddSP(this, &SLightViewer::Event_OnEndGame);
}

SLightViewer::~SLightViewer()
{
    if (BeginGameDelegateHandle.IsValid()) {
        FEditorDelegates::PostPIEStarted.Remove(BeginGameDelegateHandle);
        BeginGameDelegateHandle.Reset();
    }
    if (EndGameDelegateHandle.IsValid()) {
        Event_OnEndGame(false);
        FEditorDelegates::PostPIEStarted.Remove(EndGameDelegateHandle);
        EndGameDelegateHandle.Reset();
    }
    LightViewer.Reset();
}

void SLightViewer::OnFilterTextChanged(const FText& NewText)
{
    FilterString = NewText.ToString();
    RefreshVisibleRows();
    ContentListView->RequestListRefresh();
}

FReply SLightViewer::Event_OnRefreshButtonClicked()
{
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    ULightViewerConfig* Config = GetMutableDefault<ULightViewerConfig>();
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bLockable = false;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bAllowFavoriteSystem = false;
    DetailsViewArgs.bShowOptions = false;
    DetailsViewArgs.bShowPropertyMatrixButton = false;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    TSharedRef<IDetailsView> DetailView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    DetailView->SetObject(Config);
	
    TSharedPtr<IMenu> Menu = FSlateApplication::Get().PushMenu(ConfigButton.ToSharedRef(),
        FWidgetPath(),
        SNew(SBorder)
            .Padding(2.0f)
            .BorderBackgroundColor(FLinearColor::Black)
            [
                DetailView
            ]
        ,
        FSlateApplication::Get().GetCursorPos(),
        FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu)
        );
    
    Menu->GetOnMenuDismissed().AddLambda([this](TSharedRef<IMenu>)
    {
        ULightViewerConfig* Config = GetMutableDefault<ULightViewerConfig>();
        Config->SaveConfig();
        RefreshAvailableRows();
        RefreshVisibleRows();
        ContentListView->RequestListRefresh();
    });
    
    return FReply::Handled();
}

void SLightViewer::RefreshAvailableRows()
{
    TArray<FLightViewerMapPtr> CurrentAvailable = AvailableRows;
    TArray<TAssetPtr<UWorld>> WorldList = GetMutableDefault<ULightViewerConfig>()->AdditionalWorlds;
    AvailableRows.Reset(WorldList.Num());

    LevelColumnWidth = 128.0f;
    TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    const FTextBlockStyle& CellTextStyle = FEditorStyle::GetWidgetStyle<FTextBlockStyle>("DataTableEditor.CellText");
    
    for(const auto& w : WorldList)
    {
        FLightViewerMapPtr p = MakeShareable(new FLightViewerMap);
        p->Level = w;
        p->bUsing = false;
        
        for(const auto& ca : CurrentAvailable)
        {
            if(ca->Level == p->Level)
            {
                p->bUsing = ca->bUsing;
                break;
            }
        }

        AvailableRows.Add(p);
        LevelColumnWidth = FMath::Max(FontMeasure->Measure(p->Level.GetAssetName(), CellTextStyle.Font).X, LevelColumnWidth);
    }

    //Hide all levels that not in config now
    for(auto ar : AvailableRows)
    {
        FLightViewerMapPtr* item = CurrentAvailable.FindByPredicate(
            [&](const FLightViewerMapPtr& i)
            {
                return (i->Level == ar->Level);
            }
        );
        
        if(!item && LightViewer)
        {
            ar->bUsing = false;
            LightViewer->ChangeLevelVisibility(*ar);
        }
    }
}

void SLightViewer::RefreshVisibleRows()
{
    VisibleRows.Reset();
    if(FilterString.IsEmpty())
    {
        VisibleRows = AvailableRows;
    }
    else {
        for (const auto& ar : AvailableRows)
        {
            if (ar->Level.GetAssetName().Contains(FilterString))
            {
                VisibleRows.Add(ar);
            }
        }
    }

}

TSharedRef<ITableRow> SLightViewer::MakeRowWidget(FLightViewerMapPtr InRowDataPtr,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    return
    SNew(SLightViewerListViewRow, OwnerTable)
        .RowDataPtr(InRowDataPtr)
        .SearchText(this, &SLightViewer::GetSearchText)
        .OnCheckStateChanged(this, &SLightViewer::Event_OnCheckStateChanged, InRowDataPtr)
    ;
}

FText SLightViewer::GetSearchText() const
{
    return FText::FromString(FilterString);
}

void SLightViewer::Event_OnCheckStateChanged(ECheckBoxState NewState, FLightViewerMapPtr pData)
{
    if(LightViewer)
    {
        pData->bUsing = ( NewState == ECheckBoxState::Checked );
        LightViewer->ChangeLevelVisibility(*pData);
    }
}

void SLightViewer::Event_OnBeginGame(const bool IsSimulating)
{
    LightViewer = MakeShareable(new FLightViewer);

    for(const auto& ar : AvailableRows)
    {
        if(ar->bUsing)
        {
            LightViewer->ChangeLevelVisibility(*ar);
        }
    }
}

void SLightViewer::Event_OnEndGame(const bool IsSimulating)
{
    LightViewer.Reset();
}

TSharedRef<SWidget> SLightViewer::Construct_MapList()
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
                SNew(SBorder)
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Fill)
                    .Padding(4.0f)
                    [
                        Construct_Table_ContentPanel()
                    ]
            ]
    ;
}

TSharedRef<SWidget> SLightViewer::Construct_Table_ContentPanel()
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
        SNew(SListView<FLightViewerMapPtr>)
            .ListItemsSource(&VisibleRows)
            .HeaderRow(Construct_Table_HeaderRow())
            .OnGenerateRow(this, &SLightViewer::MakeRowWidget)
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
                    +SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        [
                            SNew(SSearchBox)
                                .OnTextChanged(this, &SLightViewer::OnFilterTextChanged)
                        ]
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SAssignNew(ConfigButton, SButton)
                                .ContentPadding(2.0f)
                                .HAlign(HAlign_Center)
                                .OnClicked(this, &SLightViewer::Event_OnRefreshButtonClicked)
                                [
                                    SNew(STextBlock)
                                        .Text(LOCTEXT("RefreshFromConfigLabel", "Config"))
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

TSharedRef<SHeaderRow> SLightViewer::Construct_Table_HeaderRow()
{
    return
        SNew(SHeaderRow)
            +SHeaderRow::Column(FLightViewerMap::COLUMN_USING)
                .DefaultLabel(LOCTEXT("UsingLabel", "Using"))
            +SHeaderRow::Column(FLightViewerMap::COLUMN_LEVEL)
                .DefaultLabel(LOCTEXT("LevelLable", "Level"))
                .ManualWidth(LevelColumnWidth)
    ;
}

#undef LOCTEXT_NAMESPACE
