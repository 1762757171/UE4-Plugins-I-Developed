
#include "SSevenDaysDeveloperListViewRow.h"


#include "Editor.h"
#include "EditorStyleSet.h"
#include "PlayerRoomSubsystem.h"
#include "SDBlueprintFunctionLibrary.h"
#include "SDStatisticsSubsystem.h"
#include "STextBlock.h"
#include "SevenDaysDeveloperRuntime/Private/SevenDaysDeveloperRuntimeUtils.h"

#define LOCTEXT_NAMESPACE "FSevenDaysDeveloperModule"

void SSevenDaysDeveloperListViewRow::Construct(const FArguments& InArgs,
    const TSharedRef<STableViewBase>& InOwnerTableView)
{
    Data = InArgs._RowDataPtr;
    Parent = InArgs._Parent;
    
    FSuperRowType::Construct(
        FSuperRowType::FArguments()
            .Style(FEditorStyle::Get(), "DataTableEditor.CellListViewRow"),
            
        InOwnerTableView
    );
}

TSharedRef<SWidget> SSevenDaysDeveloperListViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    return
    SNew(SBorder)
        .Padding(FMargin(4, 2, 4, 2))
        .ToolTipText(this, &SSevenDaysDeveloperListViewRow::GetToolTipText)
        .HAlign(HAlign_Fill)
        [
            SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                    .FillWidth(1.0f)
                    .HAlign(HAlign_Left)
                    [
                        SNew(STextBlock)
                            .Text(this, &SSevenDaysDeveloperListViewRow::GetDisplayText)
                            .HighlightText(this, &SSevenDaysDeveloperListViewRow::GetFilterText)
                            .ColorAndOpacity( this, &SSevenDaysDeveloperListViewRow::GetCurrentTextColor)
                    ]
                +SHorizontalBox::Slot()
                    .AutoWidth()
                    .HAlign(HAlign_Right)
                    [
                        SNew(STextBlock)
                            .Text(this, &SSevenDaysDeveloperListViewRow::GetStateText)
                            .ColorAndOpacity( this, &SSevenDaysDeveloperListViewRow::GetCurrentTextColor)
                            .Justification(ETextJustify::Right)
                            .HighlightText(this, &SSevenDaysDeveloperListViewRow::GetStateText)
                            .HighlightColor(this, &SSevenDaysDeveloperListViewRow::GetCurrentBackgroundColor)
                    ]
        ]
    ;
}

FText SSevenDaysDeveloperListViewRow::GetDisplayText() const
{
    return FText::FromName(GetRoomName());
}

FText SSevenDaysDeveloperListViewRow::GetStateText() const
{
    return
        Data->IsVisible ? LOCTEXT("State_Visible", "Visible")
      :(Data->IsLoaded ? LOCTEXT("State_Loaded", "Loaded")
      : LOCTEXT("State_Unloaded", "Unloaded"))
    ;
}

FText SSevenDaysDeveloperListViewRow::GetToolTipText() const
{
    return FText::Format(
        Data->IsVisible ? LOCTEXT("FormattedToolTip_Visible", "Room {0} is loaded and shown in the world.")
      :(Data->IsLoaded ? LOCTEXT("FormattedToolTip_Loaded", "Room {0} is loaded but invisible in the world")
      : LOCTEXT("FormattedToolTip_Unload", "Room {0} is not loaded."))
      , GetDisplayText()
      )
    ;
}

FSlateColor SSevenDaysDeveloperListViewRow::GetCurrentTextColor() const
{
    return FSlateColor(
        Data->IsVisible ? FLinearColor::Green
      :(Data->IsLoaded ? FLinearColor::Yellow
      : FLinearColor::Gray)
    );
}

FLinearColor SSevenDaysDeveloperListViewRow::GetCurrentBackgroundColor() const
{
    FLinearColor RoomColor = IsCurrentRoom() ? FLinearColor::Red : FLinearColor::Black;
    return RoomColor;
}

FText SSevenDaysDeveloperListViewRow::GetFilterText() const
{
    return Parent->GetFilterText();
}

bool SSevenDaysDeveloperListViewRow::IsCurrentRoom() const
{
    UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
    if(PlayWorld)
    {
        FName CurrentRoom = PlayWorld->GetSubsystem<UPlayerRoomSubsystem>(PlayWorld)->GetPlayerRoom();
		if (CurrentRoom == GetRoomName()) {
			return true;
		}
    }
    return false;
}

FName SSevenDaysDeveloperListViewRow::GetRoomName() const
{
    return Data->RoomName;
}

#undef LOCTEXT_NAMESPACE
    