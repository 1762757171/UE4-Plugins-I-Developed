
#include "SMapManagerListViewRow.h"


#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Editor.h"
#include "STextBlock.h"
#include "EditorStyleSet.h"
#include "FileHelper.h"
#include "FileHelpers.h"
#include "MapManager.h"
#include "MessageDialog.h"
#include "MultiBoxBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "SEditableTextBox.h"
#include "SInlineEditableTextBlock.h"
#include "SMapManager.h"
#include "EditorLevelUtils.h"
#include "IContentBrowserSingleton.h"
#include "MapManagerConfig.h"
#include "PlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"

class FContentBrowserModule;

void SMapManagerListViewRow::Construct(const FArguments& InArgs,
                                       const TSharedRef<STableViewBase>& InOwnerTableView)
{
    RowDataPtr = InArgs._RowDataPtr;
	//MapManager = InArgs._MapManager;
	MapManagerWidget = InArgs._MapManagerWidget;

	PopupAssetSelector =
		SNew(SMenuAnchor)
	;
	
    FSuperRowType::Construct(
        FSuperRowType::FArguments()
			.Style(FEditorStyle::Get(), "DataTableEditor.CellListViewRow"),

		InOwnerTableView
    );
}

TSharedRef<SWidget> SMapManagerListViewRow::GenerateWidgetForColumn(const FName& ColumnName)
{
	if(ColumnName == FMapManagerRowListViewData::ColumnName_RoomId)
	{
		return
        SNew(SBox)
            .Padding(FMargin(4, 2, 4, 2))
            [
                SAssignNew(RoomBlock, SInlineEditableTextBlock)
                    .Text(this, &SMapManagerListViewRow::GetRoomId)
                    .HighlightText(MapManagerWidget.Pin().ToSharedRef(), &SMapManager::GetFilterText)
                    .OnTextCommitted(this, &SMapManagerListViewRow::TextCommitted, ColumnName)
                    .ColorAndOpacity( this, &SMapManagerListViewRow::GetCurrentTextColor, ColumnName )
            ]
        ;
	}
	if(ColumnName == FMapManagerRowListViewData::ColumnName_Design)
	{
		return
        SNew(SBox)
            .Padding(FMargin(4, 2, 4, 2))
            [
                SAssignNew(DesignBlock, STextBlock)
                    .Text(this, &SMapManagerListViewRow::GetDesignMap)
                    .HighlightText(MapManagerWidget.Pin().ToSharedRef(), &SMapManager::GetFilterText)
                    .ColorAndOpacity( this, &SMapManagerListViewRow::GetCurrentTextColor, ColumnName )
            ]
        ;
	}
	
    return
    SNew(SBox)
        .Padding(FMargin(4, 2, 4, 2))
        [
            SAssignNew(DevBlock, STextBlock)
                .Text(this, &SMapManagerListViewRow::GetDevelopMap)
                .HighlightText(MapManagerWidget.Pin().ToSharedRef(), &SMapManager::GetFilterText)
                .ColorAndOpacity( this, &SMapManagerListViewRow::GetCurrentTextColor, ColumnName )
        ]
    ;
}

FReply SMapManagerListViewRow::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry,
	const FPointerEvent& InMouseEvent)
{
	if(RoomBlock->IsHovered())
	{
		RoomBlock->EnterEditingMode();
	}
	
	return FReply::Handled();
}

FReply SMapManagerListViewRow::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if(MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		ShowMenu(MouseEvent);
	}
	return STableRow::OnMouseButtonUp(MyGeometry, MouseEvent);
}

void SMapManagerListViewRow::ShowMenu(const FPointerEvent& MouseEvent)
{
	FName Column = FMapManagerRowListViewData::ColumnName_RoomId;
	FName Hovering = GetHoveringColumn();
	if(!Hovering.IsNone())
	{
		Column = Hovering;
	}
	
	SetActiveColumn(Column);
	
	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	
	/*MenuBuilder.AddSubMenu(
		LOCTEXT("MapManagerRowMenuActions_AssignMapAsset", "Assign map asset"),
		LOCTEXT("MapManagerRowMenuActions_AssignMapAssetTooltip", "Choose a map asset for this element."),
		FNewMenuDelegate::CreateRaw(this, &SMapManagerListViewRow::FillMapSelectorSubmenu)
	);*/
	if(Column != FMapManagerRowListViewData::ColumnName_RoomId)
	{
		MenuBuilder.AddMenuEntry(
         LOCTEXT("MapManagerRowMenuActions_AssignMapAsset", "Assign map asset"),
         LOCTEXT("MapManagerRowMenuActions_AssignMapAssetTooltip", "Choose a map asset for this element."),
         FSlateIcon(),
         FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::PopupMapSelector, MouseEvent, Column))
        );
		
		if(Column == FMapManagerRowListViewData::ColumnName_Design)
		{
			if(RowDataPtr->DevelopMap != FMapManager::EMPTY_PLACE_HOLDER)
			{
				MenuBuilder.AddMenuEntry(
                 LOCTEXT("MapManagerRowMenuActions_CreateDesignMapAsset", "Construct"),
                 LOCTEXT("MapManagerRowMenuActions_CreateDesignMapAssetTooltip", "Create a same asset for designer from develop."),
                 FSlateIcon(FEditorStyle::GetStyleSetName(), "DataTableEditor.Add"),
                 FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::CreateForDesignFromDev))
                );
			}
		}else if(Column == FMapManagerRowListViewData::ColumnName_Develop)
		{
			if(RowDataPtr->DesignMap != FMapManager::EMPTY_PLACE_HOLDER)
			{
				MenuBuilder.AddMenuEntry(
                 LOCTEXT("MapManagerRowMenuActions_CreateDevMapAsset", "Construct"),
                 LOCTEXT("MapManagerRowMenuActions_CreateDevMapAssetTooltip", "Create a same asset for develop from designer."),
                 FSlateIcon(FEditorStyle::GetStyleSetName(), "DataTableEditor.Add"),
                 FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::CreateForDevFromDesign))
                );
			}
		}
	}

	MenuBuilder.AddMenuEntry(
     LOCTEXT("MapManagerRowMenuActions_CopyPath", "Copy literally"),
     LOCTEXT("MapManagerRowMenuActions_CopyTooltip", "Copy the string."),
     FSlateIcon(),
     FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::CopyThis, Column))		//FIX
     );

	MenuBuilder.AddMenuEntry(
     LOCTEXT("MapManagerRowMenuActions_DeleteRoom", "Delete this room"),
     LOCTEXT("MapManagerRowMenuActions_DeleteRoomAssetTooltip", "Delete this room."),
     FSlateIcon(),
     FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::DeleteThis))		//FIX
    );

		
	if(Column != FMapManagerRowListViewData::ColumnName_RoomId)
	{
		FString LevelPath;
		if(Column == FMapManagerRowListViewData::ColumnName_Design)
		{
			LevelPath = RowDataPtr->DesignMap;
		}
		else if(Column == FMapManagerRowListViewData::ColumnName_Develop)
		{
			LevelPath = RowDataPtr->DevelopMap;
		}
		if(LevelPath != FMapManager::EMPTY_PLACE_HOLDER)
		{
			MenuBuilder.AddMenuSeparator();
			
			MenuBuilder.AddMenuEntry(
              LOCTEXT("MapManagerRowMenuActions_Browse", "Browse"),
              LOCTEXT("MapManagerRowMenuActions_BrowseTooltip", "Show in content browser."),
              FSlateIcon(),
              FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::Browse, Column))		//FIX
             );
			
			MenuBuilder.AddMenuEntry(
	          LOCTEXT("MapManagerRowMenuActions_OpenLevel", "Open this level"),
	          LOCTEXT("MapManagerRowMenuActions_OpenLevelTooltip", "Open the choosing level."),
	          FSlateIcon(),
	          FUIAction(FExecuteAction::CreateSP(this, &SMapManagerListViewRow::OpenChoosingLevel, LevelPath))		
         );
		}
		
	}
	
	FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();
	TSharedPtr<IMenu> Menu =
		FSlateApplication::Get().PushMenu(
		AsShared(),
		WidgetPath,
		MenuBuilder.MakeWidget(),
		MouseEvent.GetScreenSpacePosition(),
		FPopupTransitionEffect::ContextMenu
		);
	Menu->GetOnMenuDismissed().AddRaw(this, &SMapManagerListViewRow::Event_OnMenuDismissed);
}

void SMapManagerListViewRow::PopupMapSelector(FPointerEvent MouseEvent, FName Column)
{
	PopupAssetSelector->SetContent(
	            PropertyCustomizationHelpers::MakeAssetPickerWithMenu(
	                FAssetData(),	//TODO: Get asset assigned here
	                true,
	                {UWorld::StaticClass()},
	                {},
	                FOnShouldFilterAsset()/*::CreateRaw()*/,		//TODO: exclude world which has sublevel (maybe)
	                FOnAssetSelected::CreateRaw(this, &SMapManagerListViewRow::AssetSelected, Column),
	                FSimpleDelegate::CreateRaw(this, &SMapManagerListViewRow::ClosePopupSelector)
	                )
            )
    ;

	PopupAssetSelector->SetIsOpen(true);
	
	FWidgetPath WidgetPath = MouseEvent.GetEventPath() != nullptr ? *MouseEvent.GetEventPath() : FWidgetPath();		//FIX
	FSlateApplication::Get().PushMenu(
		AsShared(),
		WidgetPath,
		PopupAssetSelector.ToSharedRef(),
		MouseEvent.GetScreenSpacePosition(),
		FPopupTransitionEffect::ComboButton
		);
}

void SMapManagerListViewRow::ClosePopupSelector()
{
	PopupAssetSelector->SetContent(SNullWidget::NullWidget);
	PopupAssetSelector->SetIsOpen(false);
	ClearActive();
}

void SMapManagerListViewRow::AssetSelected(const FAssetData& AssetData, FName Column)
{
	const FString AssetDataFilePath = AssetData.PackageName.ToString();
	
	//GetColumnBlock(Column)->SetText(FText::FromString(AssetDataFilePath));

	TextCommitted(AssetDataFilePath, Column);
}

void SMapManagerListViewRow::TextCommitted(const FText& InText, ETextCommit::Type InCommitType, FName ColumnName)
{
	TextCommitted(InText.ToString(), ColumnName);
}

void SMapManagerListViewRow::TextCommitted(const FString& InString, FName ColumnName)
{
	FString& EditingString =
        (ColumnName == FMapManagerRowListViewData::ColumnName_RoomId) ? RowDataPtr->RoomId
    : 	(ColumnName == FMapManagerRowListViewData::ColumnName_Design) ? RowDataPtr->DesignMap
    : 	RowDataPtr->DevelopMap
    ;
	EditingString = InString;
	MapManagerWidget.Pin()->SaveDirtyMapTable();
	
	ClearActive();
}

void SMapManagerListViewRow::OpenChoosingLevel(FString LevelPath)
{
	// If there are any unsaved changes to the current level, see if the user wants to save those first.
	if (FEditorFileUtils::SaveDirtyPackages(true, true, true))
	{
		FEditorFileUtils::LoadMap(LevelPath, false, true);
	}
	
	ClearActive();
}

void SMapManagerListViewRow::CopyThis(FName Column)
{
	FString String = RowDataPtr->RoomId;
	if(Column == FMapManagerRowListViewData::ColumnName_Design)
	{
		String = RowDataPtr->DesignMap;
	}else if(Column == FMapManagerRowListViewData::ColumnName_Develop)
	{
		String = RowDataPtr->DevelopMap;
	}

	FPlatformApplicationMisc::ClipboardCopy( *String );
	
	ClearActive();
}


void SMapManagerListViewRow::DeleteThis()
{
	if(EAppReturnType::Yes == FMessageDialog::Open(
        EAppMsgType::YesNo,
        EAppReturnType::No,
        LOCTEXT("Prompt_DeleteRoom", "Are you sure DELETING this roooooom?")
        ))
	{
		RowDataPtr->RoomId = FMapManager::DELETED_ROOM_ID;
		MapManagerWidget.Pin()->SaveDirtyMapTable();
	}
	
	ClearActive();
		
}

void SMapManagerListViewRow::Browse(FName Column)
{
	FString String = RowDataPtr->RoomId;
	if(Column == FMapManagerRowListViewData::ColumnName_Design)
	{
		String = RowDataPtr->DesignMap;
	}else if(Column == FMapManagerRowListViewData::ColumnName_Develop)
	{
		String = RowDataPtr->DevelopMap;
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	FString ErrorMsg;
	FAssetData MapAssetData;
	if(MapManagerUtils::GetMapAssetFromPath(String, MapAssetData, ErrorMsg))
	{
		ContentBrowserModule.Get().SyncBrowserToAssets({MapAssetData});
	}else
	{
		FMessageDialog::Open(
	        EAppMsgType::Ok,
	        EAppReturnType::Ok,
	        LOCTEXT("Prompt_BrowseFail", "Sorry but this map does NOT exist.")
	        );
	}
	
	ClearActive();
}

void SMapManagerListViewRow::CreateForDevFromDesign()
{
	FString DesignMapPath = RowDataPtr->DesignMap;
	
	FString DefaultPath = GetMutableDefault<UMapManagerConfig>()->DevelopLevelPath;

	FString SaveObjectPath;
	
	UObject* NewMap = CreateMapWithDialog(DesignMapPath, DefaultPath, SaveObjectPath);

	if(!NewMap)
	{
		return;
	}
	
	AssetSelected(FAssetData(NewMap), FMapManagerRowListViewData::ColumnName_Develop);
	
	//TODO: Replace sublevel reference in persistent level package

	ClearActive();
}

void SMapManagerListViewRow::CreateForDesignFromDev()
{
	FString DevMapPath = RowDataPtr->DevelopMap;
	 
	FString DefaultPath = GetMutableDefault<UMapManagerConfig>()->DesignLevelPath;

	FString SaveObjectPath;
	
	UObject* NewMap = CreateMapWithDialog(DevMapPath, DefaultPath, SaveObjectPath);
	
	if(!NewMap)
	{
		return;
	}
	
	AssetSelected(FAssetData(NewMap), FMapManagerRowListViewData::ColumnName_Design);
	
	//TODO: Replace sublevel reference in persistent level package

	ClearActive();
}
UObject* SMapManagerListViewRow::CreateMapWithDialog(const FString& SourceMapPath, FString DefaultPath,
	FString& NewMapPath)
{
	//ask for saving
	if (!FEditorFileUtils::SaveDirtyPackages(true, true, true))
	{
		return nullptr;
	}
	
	FString NewNameSuggestion;
	SourceMapPath.Split(TEXT("/"), nullptr, &NewNameSuggestion, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	
	if (DefaultPath.IsEmpty())
	{
		DefaultPath = TEXT("/Game/Maps");
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssetsByPackageName(FName(*SourceMapPath), AssetDataArray);

	if(AssetDataArray.Num() != 1)
	{
		return nullptr;
	}
	
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetTools& AssetTool = AssetToolsModule.Get();
	
	UObject* NewMap = nullptr;
	NewMap = AssetTool.DuplicateAssetWithDialog(NewNameSuggestion, DefaultPath, AssetDataArray[0].GetAsset());

	//save without asking
	if (!FEditorFileUtils::SaveDirtyPackages(false, true, true))
	{
		return nullptr;
	}
	
	ClearActive();
	
	return NewMap;
}

FText SMapManagerListViewRow::GetRoomId() const
{
	return FText::FromString(RowDataPtr->RoomId);
}

FText SMapManagerListViewRow::GetDesignMap() const
{
	/*return FText::FromString(
		(RowDataPtr->DesignMap.IsEmpty() ? TEXT("<Empty>") : RowDataPtr->DesignMap)
		);*/

	return FText::FromString(RowDataPtr->DesignMap);
}

FText SMapManagerListViewRow::GetDevelopMap() const
{
	/*return FText::FromString(
        (RowDataPtr->DevelopMap.IsEmpty() ? TEXT("<Empty>") : RowDataPtr->DevelopMap)
        );*/
	return FText::FromString(RowDataPtr->DevelopMap);
}

FSlateColor SMapManagerListViewRow::GetCurrentTextColor(FName Column) const
{
	return FSlateColor(FLinearColor(
		IsSelected() ?
			(IsActiveColumn(Column) ? FColor::Red : FColor::Orange)
			: FColor::White
	));
}

void SMapManagerListViewRow::ClearActive()
{
	ActiveColumn = FName(NAME_None);
}

void SMapManagerListViewRow::Event_OnMenuDismissed(TSharedRef<IMenu> DismissedMenu)
{
	ClearActive();
}

bool SMapManagerListViewRow::IsActiveColumn(FName Column) const
{
	if(!ActiveColumn.IsNone())
	{
		return Column == ActiveColumn;
	}
	return GetHoveringColumn() == Column;
}

const FName SMapManagerListViewRow::GetHoveringColumn() const
{
	if(RoomBlock->IsHovered())
	{
		return FMapManagerRowListViewData::ColumnName_RoomId;
	}
	else if(DesignBlock->IsHovered())
	{
		return FMapManagerRowListViewData::ColumnName_Design;
	}
	else if(DevBlock->IsHovered())
	{
		return FMapManagerRowListViewData::ColumnName_Develop;
	}
	return NAME_None;
}

void SMapManagerListViewRow::SetActiveColumn(const FName HandlingColumn)
{
	ActiveColumn = HandlingColumn;
}


#undef LOCTEXT_NAMESPACE
