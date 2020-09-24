
#include "SLightViewerListViewRow.h"

#include "EditorStyleSet.h"
#include "SCheckBox.h"
#include "STextBlock.h"

void SLightViewerListViewRow::Construct(const FArguments& InArgs,
                                        const TSharedRef<STableViewBase>& InOwnerTableView)
{
    RowDataPtr = InArgs._RowDataPtr;
    SearchText = InArgs._SearchText;
    OnCheckStateChanged = InArgs._OnCheckStateChanged;
	
    FSuperRowType::Construct(
        FSuperRowType::FArguments()
            .Style(FEditorStyle::Get(), "DataTableEditor.CellListViewRow"),

        InOwnerTableView
    );
}

TSharedRef<SWidget> SLightViewerListViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
    if(ColumnName == FLightViewerMap::COLUMN_USING)
    {
        return
            SNew(SCheckBox)
                .OnCheckStateChanged(OnCheckStateChanged)
                .IsChecked(this, &SLightViewerListViewRow::GetCheckState)
        ;
    }
    if(ColumnName == FLightViewerMap::COLUMN_LEVEL)
    {
        return
        SNew(SBox)
            .Padding(FMargin(4, 2, 4, 2))
            [
                SNew(STextBlock)
                    .Text(FText::FromString( RowDataPtr->Level.GetAssetName() ))
                    .HighlightText(SearchText)
                    .ToolTipText(FText::FromString( RowDataPtr->Level.GetLongPackageName() ))
            ]
        ;
    }

    return SNullWidget::NullWidget;
}

ECheckBoxState SLightViewerListViewRow::GetCheckState() const
{
    return
        RowDataPtr->bUsing ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}
