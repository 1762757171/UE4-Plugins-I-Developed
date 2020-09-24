// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "AssetData.h"
#include "SCheckBox.h"
#include "LightViewer.h"

class SLightViewerListViewRow : public SMultiColumnTableRow<FLightViewerMapPtr>
{
public:

    SLATE_BEGIN_ARGS(SLightViewerListViewRow) {}
        /** The row we're working with to allow us to get naming information. */
        SLATE_ARGUMENT(FLightViewerMapPtr, RowDataPtr)
        SLATE_ATTRIBUTE(FText, SearchText)
        /** Called when the checked state has changed */
        SLATE_EVENT( FOnCheckStateChanged, OnCheckStateChanged )
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

    //virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
    ECheckBoxState GetCheckState() const;
    
private:
    
	FLightViewerMapPtr RowDataPtr;
    
    TAttribute<FText> SearchText;
    
    /** Delegate called when the check box changes state */
    FOnCheckStateChanged OnCheckStateChanged;
    
};
