// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "MapLoader.h"
#include "SMapLoader.h"

class SMapLoaderTreeViewRow : public SMultiColumnTableRow<FMapLevelLoaderDataPtr>
{
public:

	SLATE_BEGIN_ARGS(SMapLoaderTreeViewRow) {}
	/** The row we're working with to allow us to get naming information. */
		SLATE_ARGUMENT(FMapLevelLoaderDataPtr, RowDataPtr)
    /** The row we're working with to allow us to get naming information. */
		SLATE_ARGUMENT(TSharedPtr<FMapLoader>, Loader)
	/** The row we're working with to allow us to get naming information. */
	    SLATE_ARGUMENT(TSharedPtr<SMapLoader>, OuterWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	//virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	//virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	
	void ShowMenu(const FPointerEvent& MouseEvent);

private:
	void HideUCX();
	void ShowUCX();
private:
	ECheckBoxState GetCheckBoxState() const;
	void OnCheckStateChanged(ECheckBoxState NewCheckState);
	FSlateColor GetTextColor() const;
private:
	FMapLevelLoaderDataPtr Data;
	TSharedPtr<FMapLoader> Loader;
	TSharedPtr<SMapLoader> OuterWidget;
};

