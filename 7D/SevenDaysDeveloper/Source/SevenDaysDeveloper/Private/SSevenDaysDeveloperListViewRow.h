// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SevenDaysDeveloperUtils.h"
#include "SSevenDaysDeveloper.h"
#include "Widgets/Views/STableRow.h"
#include "SSevenDaysDeveloperListViewRow.h"

class SSevenDaysDeveloperListViewRow : public SMultiColumnTableRow<FSevenDaysDeveloperRowDataPtr>
{
public:

    SLATE_BEGIN_ARGS(SSevenDaysDeveloperListViewRow) {}
    /** The row we're working with to allow us to get naming information. */
        SLATE_ARGUMENT(FSevenDaysDeveloperRowDataPtr, RowDataPtr)
        SLATE_ARGUMENT(TSharedPtr<SSevenDaysDeveloper>, Parent)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

    virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;
private:
    FText GetDisplayText() const;
    FText GetStateText() const;
    FText GetToolTipText() const;
    FSlateColor GetCurrentTextColor() const;
    FLinearColor GetCurrentBackgroundColor() const;
    FText GetFilterText() const;
    bool IsCurrentRoom() const;

    FName GetRoomName() const;
private:
    FSevenDaysDeveloperRowDataPtr Data;
    TSharedPtr<SSevenDaysDeveloper> Parent;
};
