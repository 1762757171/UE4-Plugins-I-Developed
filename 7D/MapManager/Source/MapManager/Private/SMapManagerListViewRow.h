// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Views/STableRow.h"
#include "MapManagerUtils.h"
#include "AssetData.h"

class SMapManagerListViewRow : public SMultiColumnTableRow<FMapManagerRowListViewDataPtr>
{
public:

	SLATE_BEGIN_ARGS(SMapManagerListViewRow) {}
		/** The row we're working with to allow us to get naming information. */
		SLATE_ARGUMENT(FMapManagerRowListViewDataPtr, RowDataPtr)
		//SLATE_ARGUMENT(TSharedPtr<class FMapManager>, MapManager)
		SLATE_ARGUMENT(TSharedPtr<class SMapManager>, MapManagerWidget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	//virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
public:
	
	void ShowMenu(const FPointerEvent& MouseEvent);
private:

	void PopupMapSelector(FPointerEvent MouseEvent, FName Column);
	void ClosePopupSelector();
	void AssetSelected(const FAssetData& AssetData, FName Column);
	void TextCommitted(const FText& InText, ETextCommit::Type InCommitType, FName ColumnName);
	void TextCommitted(const FString& InString, FName ColumnName);

	void OpenChoosingLevel(FString Column);
	void CopyThis(FName Column);
	void DeleteThis();
	void Browse(FName Column);

	void CreateForDevFromDesign();
	void CreateForDesignFromDev();

	UObject* CreateMapWithDialog(const FString& SourceMapPath, FString DefaultPath, FString& NewMapPath);
private:
	FText GetRoomId() const;
	FText GetDesignMap() const;
	FText GetDevelopMap() const;

	FSlateColor GetCurrentTextColor(FName Column) const;
	bool IsActiveColumn(FName Column) const;

	const FName GetHoveringColumn() const;
	void SetActiveColumn(const FName HandlingColumn);
	void ClearActive();

	void Event_OnMenuDismissed(TSharedRef<class IMenu> DismissedMenu);
private:
	FMapManagerRowListViewDataPtr RowDataPtr;
	//TWeakPtr<class FMapManager> MapManager;
	TWeakPtr<class SMapManager> MapManagerWidget;

	TSharedPtr<class SInlineEditableTextBlock> RoomBlock;
	TSharedPtr<class STextBlock> DesignBlock;
	TSharedPtr<class STextBlock> DevBlock;
	
	TSharedPtr<class SMenuAnchor> PopupAssetSelector;
	FName ActiveColumn;
};
