
#include "SMapLoaderTreeViewRow.h"

#include "EditorStyleSet.h"
#include "FileHelpers.h"
#include "IContentBrowserSingleton.h"
#include "MultiBoxBuilder.h"
#include "SCheckBox.h"
#include "SlateApplication.h"
#include "SMapLoader.h"
#include "WidgetPath.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"

void SMapLoaderTreeViewRow::Construct(const FArguments& InArgs,
                                      const TSharedRef<STableViewBase>& InOwnerTableView)
{
    Data = InArgs._RowDataPtr;
    Loader = InArgs._Loader;
    OuterWidget = InArgs._OuterWidget;

    FSuperRowType::Construct(
      FSuperRowType::FArguments()
          .Style(FEditorStyle::Get(), "DataTableEditor.CellListViewRow"),

      InOwnerTableView
    );

}

TSharedRef<SWidget> SMapLoaderTreeViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if(ColumnName == SMapLoader::Column_Load)
    {
        return
        SNew(SBorder)
            [
                SNew(SCheckBox)
                    .IsChecked(this, &SMapLoaderTreeViewRow::GetCheckBoxState)
                    .OnCheckStateChanged(this, &SMapLoaderTreeViewRow::OnCheckStateChanged)
            ]
        ;
    }
    if(ColumnName == SMapLoader::Column_Map)
    {
        return
        SNew(SBorder)
            [
                SNew(SHorizontalBox)
                    +SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SExpanderArrow, SharedThis(this))
                                .ShouldDrawWires(true)
                        ]
                    +SHorizontalBox::Slot()
                        .FillWidth(1.0f)
                        [
                            SNew(STextBlock)
                                .Text(FText::FromString( FPaths::GetBaseFilename( Data->LevelStreaming_or_HierarchyName ) ))
                                .HighlightText(OuterWidget.ToSharedRef(), &SMapLoader::GetFilterText)
                                .ColorAndOpacity(this, &SMapLoaderTreeViewRow::GetTextColor)
                        ]
            ]
        ;
    }

    return SNullWidget::NullWidget;
}

FReply SMapLoaderTreeViewRow::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
    STableRow::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);

    if(!Data->IsFolder)
    {
        FEditorFileUtils::LoadMap(Data->LevelStreaming_or_HierarchyName, false, true);
    }

    return FReply::Handled();
}

FReply SMapLoaderTreeViewRow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if(MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        ShowMenu(MouseEvent);
    }
    return STableRow::OnMouseButtonUp(MyGeometry, MouseEvent);
}

void SMapLoaderTreeViewRow::ShowMenu(const FPointerEvent& MouseEvent)
{
    //if(!Data->bLoaded) { return; }
    
	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

    MenuBuilder.AddMenuEntry(
         LOCTEXT("MapManagerTreeRowMenuActions_HideUCX", "Hide all UCX"),
         LOCTEXT("MapManagerTreeRowMenuActions_AssignMapAssetTooltip", "Hide all UCX inside child maps of selection."),
         FSlateIcon(),
         FUIAction(FExecuteAction::CreateSP(this, &SMapLoaderTreeViewRow::HideUCX))
        );
    MenuBuilder.AddMenuEntry(
     LOCTEXT("MapManagerTreeRowMenuActions_ShowUCX", "Show all UCX"),
     LOCTEXT("MapManagerTreeRowMenuActions_AssignMapAssetTooltip", "Show all UCX inside child maps of selection."),
     FSlateIcon(),
     FUIAction(FExecuteAction::CreateSP(this, &SMapLoaderTreeViewRow::ShowUCX))
     );
    
    FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
    TSharedPtr<IMenu> Menu =
        FSlateApplication::Get().PushMenu(
        AsShared(),
        WidgetPath,
        MenuBuilder.MakeWidget(),
        MouseEvent.GetScreenSpacePosition(),
        FPopupTransitionEffect::ContextMenu
        );
    //Menu->GetOnMenuDismissed().AddRaw(this, &SMapLoaderTreeViewRow::Event_OnMenuDismissed);
}

void SMapLoaderTreeViewRow::HideUCX()
{
    TArray<FString> ChildrenLeaves = Data->GetAllChildLeaves();
    
    for(const FString& cl : ChildrenLeaves)
    {
        Loader->HideUCX(cl);
    }
}

void SMapLoaderTreeViewRow::ShowUCX()
{
    TArray<FString> ChildrenLeaves = Data->GetAllChildLeaves();
    
    for(const FString& cl : ChildrenLeaves)
    {
        Loader->ShowUCX(cl);
    }
}

ECheckBoxState SMapLoaderTreeViewRow::GetCheckBoxState() const
{
    return Data->GetLoadedState();
}

void SMapLoaderTreeViewRow::OnCheckStateChanged(ECheckBoxState NewCheckState)
{
    TArray<FString> Leaves = Data->GetAllChildLeaves();
    
    switch (NewCheckState)
    {
    case ECheckBoxState::Checked:
        Loader->LoadLevels(Leaves);
        return;
        
    case ECheckBoxState::Unchecked:
        Loader->UnloadLevels(Leaves);
        return;
        
    default:
        return;
    }
}

FSlateColor SMapLoaderTreeViewRow::GetTextColor() const
{
    return FSlateColor(
        (!Data->IsFolder && Data->bLoaded) ? FLinearColor::Green : FLinearColor::White
            );
}

#undef LOCTEXT_NAMESPACE