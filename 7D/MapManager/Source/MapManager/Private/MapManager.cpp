// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MapManager.h"

#include "AssetEditorManager.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "EditorLevelUtils.h"
#include "FileHelper.h"
#include "FileHelpers.h"
#include "FileManager.h"
#include "GameMapsSettings.h"
#include "LevelEditor.h"
#include "MapManagerModule.h"
#include "MessageDialog.h"
#include "SMapManager.h"
#include "ObjectTools.h"
#include "SDGameInstance.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "Engine/LevelStreamingDynamic.h"
#include "Engine/MapBuildDataRegistry.h"
#include "Kismet/KismetStringLibrary.h"
#include "Private/LevelCollectionModel.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"

const FString FMapManager::DELETED_ROOM_ID(FName(NAME_None).ToString());
const FString FMapManager::EMPTY_PLACE_HOLDER(TEXT("<Empty>"));

FMapManager::FMapManager()
{
}

void FMapManager::SetTableFilePath(FString TableFilePath)
{
	FString AbsPath = TableFilePath / TEXT("Table.csv");
	_TablePath = IFileManager::Get().ConvertToRelativePath(*AbsPath);
}

void FMapManager::RefreshTableFromFile()
{
	FString Data;
	if( FFileHelper::LoadFileToString( Data, *_TablePath, FFileHelper::EHashOptions::None, FILEREAD_AllowWrite))
	{
		MapRows.Reset();
		//Now parse it
		TArray<FString> Lines;
		Data.ParseIntoArrayLines(Lines);
		for(const auto& ls : Lines)
		{
			TArray<FString> Pair;
			ls.ParseIntoArray(Pair, TEXT(","));
			
			
			while(Pair.Num() < 3)
			{
				Pair.Add(EMPTY_PLACE_HOLDER);
			}

			if(Pair[1] == FString())
			{
				Pair[1] = EMPTY_PLACE_HOLDER;
			}
			if(Pair[2] == FString())
			{
				Pair[2] = EMPTY_PLACE_HOLDER;
			}

			if(Pair[0] == DELETED_ROOM_ID)
			{
				continue;
			}
			FMapManagerRowListViewData CurrentData(Pair[0], Pair[1], Pair[2]);
			MapRows.Add(CurrentData);
		}
	}else
	{
		//if no table csv, create a new one
		FFileHelper::SaveStringToFile(FString(), *_TablePath);
	}
}

bool FMapManager::WriteTableIntoFile(const TArray<FMapManagerRowListViewData>& NewData)
{
	const FString Comma(TEXT(","));
	const FString NewLine = TEXT("\r\n");
	FString Data;
	for(const auto& d : NewData)
	{
		if(d.RoomId == DELETED_ROOM_ID)
		{
			continue;
		}

		FString DesignMap = d.DesignMap;
		FString DevMap = d.DevelopMap;
		
		if(d.DesignMap == FName(NAME_None).ToString())
		{
			DesignMap = EMPTY_PLACE_HOLDER;
		}
		if(d.DevelopMap == FName(NAME_None).ToString())
		{
			DevMap = EMPTY_PLACE_HOLDER;
		}
			
		FString CurrentLine;
		CurrentLine += d.RoomId;
		CurrentLine += Comma;
		CurrentLine += DesignMap;
		CurrentLine += Comma;
		CurrentLine += DevMap;
		CurrentLine += NewLine;
		Data += CurrentLine;
	}

	const bool bSuccess = FFileHelper::SaveStringToFile(Data, *_TablePath);

	if(bSuccess)
	{
		MapRows = NewData;
	}else
	{
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("Prompt_SaveFileFail", "Failed to save config to file!")
			);
		
		UE_LOG(LogTemp, Warning, TEXT("Save to table file failed."));
	}

	return bSuccess;
}

const TArray<FMapManagerRowListViewData>& FMapManager::GetMapRows() const
{
	return MapRows;
}

bool FMapManager::DoWorldHaveLevel(UWorld* World, const FString& LevelName)
{
	auto LevelStreaming = World->GetStreamingLevels();
	for(auto ls : LevelStreaming)
	{
		if(ls->GetWorldAssetPackageName() == LevelName)
		{
			return true;
		}
	}
	return false;
}

ULevelStreaming* FMapManager::AddLevelToWorld(UWorld* World, const FString& LevelName, bool bAlwaysLoaded)
{
	if(LevelName.IsEmpty()) { return nullptr; }
	auto LevelStreaming = World->GetStreamingLevels();
	for(auto ls : LevelStreaming)
	{
		if(ls->GetWorldAssetPackageName() == LevelName
			&&
			ls->GetCurrentState() != ULevelStreaming::ECurrentState::Removed)
		{
			return ls;
		}
	}
	//UEditorLevelUtils::AddLevelToWorld(World, *LevelName, ULevelStreamingDynamic::StaticClass());

	TSubclassOf<ULevelStreaming> StreamingClass = ULevelStreamingDynamic::StaticClass();
	//if(LevelName.Contains(TEXT("Persistent")))
	//{
	//	StreamingClass = ULevelStreamingAlwaysLoaded::StaticClass();
	//}
	if(bAlwaysLoaded)
	{
		StreamingClass = ULevelStreamingAlwaysLoaded::StaticClass();
	}
	auto StreamingLevel = NewObject<ULevelStreaming>(World, StreamingClass, NAME_None, RF_NoFlags, NULL);

	// Associate a package name.
	StreamingLevel->SetWorldAssetByPackageName(*LevelName);

	StreamingLevel->LevelTransform = FTransform::Identity;

	// Seed the level's draw color.
	StreamingLevel->LevelColor = FLinearColor::MakeRandomColor();

	// Add the new level to world.
	World->AddStreamingLevel(StreamingLevel);

	World->MarkPackageDirty();

	return StreamingLevel;
}

TArray<FString> FMapManager::GetSublevelNames(const FString& PersistentLevelName, const FString& SuffixToIgnore)
{
	FAssetData PersistentAsset;
	FString Temp;
	if(!MapManagerUtils::GetMapAssetFromPath(PersistentLevelName, PersistentAsset, Temp))
	{
		return {};
	}
	UPackage* Package = PersistentAsset.GetPackage();
	UWorld* World = UWorld::FindWorldInPackage(Package);
	if(!World)
	{
		return {};
	}
	auto Levels = World->GetStreamingLevels();
	TArray<FString> RetVal;
	for(const auto& l : Levels)
	{
		FString name = l->GetWorldAssetPackageName();
		if(name.EndsWith(SuffixToIgnore))
		{
			continue;
		}
		RetVal.Add(name);
	}
	return RetVal;
}

bool FMapManager::IsSublevelInitiallyLoaded(const FString& PersistentLevelName, const FString& SublevelName)
{
	FAssetData PersistentAsset;
	FString Temp;
	if(!MapManagerUtils::GetMapAssetFromPath(PersistentLevelName, PersistentAsset, Temp))
	{
		return false;
	}
	UPackage* Package = PersistentAsset.GetPackage();
	UWorld* World = UWorld::FindWorldInPackage(Package);
	if(!World)
	{
		return false;
	}
	ULevelStreamingDynamic* Level = Cast<ULevelStreamingDynamic>( World->GetLevelStreamingForPackageName(*SublevelName) );
	if(!Level)
	{
		return false;
	}
	return Level->bInitiallyLoaded;
}

bool FMapManager::RemoveLevelFromWorld(UWorld* World, const FString& LevelName)
{
	auto LevelStreaming = World->GetStreamingLevels();
	for(auto ls : LevelStreaming)
	{
		if(ls->GetWorldAssetPackageName() == LevelName)
		{
			if(ls->HasLoadedLevel())
			{
				EditorLevelUtils::RemoveLevelFromWorld(ls->GetLoadedLevel());
			}else
			{
				World->RemoveStreamingLevel(ls);
				ls->MarkPendingKill();
				World->MarkPackageDirty();
			}
			return true;
		}
	}
	return false;
}

bool FMapManager::GetFolderPath(UWorld* World, const FString& LevelName, FName& OutFolder)
{
	auto LevelStreaming = World->GetStreamingLevels();
	for(auto ls : LevelStreaming)
	{
		if(ls->GetWorldAssetPackageName() == LevelName)
		{
			OutFolder = ls->GetFolderPath();
			return true;
		}
	}
	return false;
}

void FMapManager::SetFolderPath(UWorld* World, const FString& LevelName, const FName& Folder)
{
	auto LevelStreaming = World->GetStreamingLevels();
	for(auto ls : LevelStreaming)
	{
		if(ls->GetWorldAssetPackageName() == LevelName)
		{
			ls->SetFolderPath(Folder);
			return;
		}
	}
	return;
}

void FMapManager::CommitDesignInternal(const TArray<FMapManagerRowListViewData>& Maps)
{
	if(Maps.Num() == 0)
	{
		FMessageDialog::Open(
            EAppMsgType::Ok,
            EAppReturnType::Ok,
            LOCTEXT("NoSelectedToCommit", "Please select at least one map to commit!")
            );
		return;
	}
	
	// If there are any unsaved changes to the current level, see if the user wants to save those first.
	if (!FEditorFileUtils::SaveDirtyPackages(true, true, true))
	{
		return;
	}

	//DEPRECATE API:
	//FAssetEditorManager::Get().OpenEditorForAsset(TEXT("/Engine/Maps/Entry"));
	//NEW API:
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if(!AssetEditorSubsystem)
	{
		FMessageDialog::Open(
            EAppMsgType::Ok,
            EAppReturnType::Ok,
            LOCTEXT("NoAssetEditorSubsystem", "Fatal error: No Asset Editor Subsystem found!")
            );
		return;
	}
	AssetEditorSubsystem->OpenEditorForAsset(TEXT("/Engine/Maps/Entry"));
	
	const FString NewLine(TEXT("\r\n"));
	const FString OldSuffix(TEXT("__OLD_OUTDATED"));
	FString OutputLog(TEXT("Commit result:"));
	OutputLog += NewLine;
	
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetTools& AssetTool = AssetToolsModule.Get();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FMapManagerRowListViewData> ChangedRoomData;

	
	for(const auto& RoomData : Maps)
	{
		double Time = FPlatformTime::Seconds();
		double Time_1 = 0;
		double Time_2 = 0;
		double Time_3 = 0;
		//==============================================================================================================
		//	Check exists
		//==============================================================================================================
		if(RoomData.DesignMap.IsEmpty())
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (TEXT("Design map not exist!"));
			OutputLog += NewLine;
			continue;
		}
		if(RoomData.DevelopMap.IsEmpty())
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (TEXT("Develop map not exist!"));
			OutputLog += NewLine;
			continue;
		}
		
		FAssetData OldAsset;
		if(!MapManagerUtils::GetMapAssetFromPath(RoomData.DevelopMap, OldAsset, OutputLog))
		{
			continue;
		}
		
		FAssetData NewAsset;
		
		if(!MapManagerUtils::GetMapAssetFromPath(RoomData.DesignMap, NewAsset, OutputLog))
		{
			continue;
		}
		
		UObject* DesginAsset = NewAsset.GetAsset();
		if(!DesginAsset)
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" asset not found!"));
			OutputLog += NewLine;
			continue;
		}

		//Start with reference fix
		FixupRedirectory_Develop({RoomData});

		//==============================================================================================================
		//	1. rename old
		//==============================================================================================================
		
		FAssetRenameData RenameData(OldAsset.GetAsset(), FPaths::GetPath(RoomData.DevelopMap), FPaths::GetBaseFilename(RoomData.DevelopMap) + OldSuffix);
		AssetTool.RenameAssetsWithDialog({RenameData});

		FAssetData OldAssetRE;
		
		MapManagerUtils::GetMapAssetFromPath(RoomData.DevelopMap + OldSuffix, OldAssetRE, OutputLog);
		
		UObject* OldMapRE = OldAssetRE.GetAsset();
		if(!OldMapRE)
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" rename section unexpected error!"));
			OutputLog += NewLine;
			
			continue;
		}
		FixupRedirectory_Develop({RoomData});

		Time_1 = FPlatformTime::Seconds();
			
		//==============================================================================================================
		//	2.duplicate new
		//==============================================================================================================
		
		UObject* NewDuplicatedMap = nullptr;

		NewDuplicatedMap = AssetTool.DuplicateAsset(FPaths::GetBaseFilename(RoomData.DevelopMap), FPaths::GetPath(RoomData.DevelopMap), DesginAsset);

		if(!NewDuplicatedMap)
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" duplication section unexpected error!"));
			OutputLog += NewLine;
			
			//if failed, restore
			FAssetRenameData RenameDataRE(OldMapRE, FPaths::GetPath(RoomData.DevelopMap), FPaths::GetBaseFilename(RoomData.DevelopMap) );
			AssetTool.RenameAssetsWithDialog({RenameDataRE});
			continue;
		}
		FixupRedirectory_Develop({RoomData});
		//Save all
		if (!FEditorFileUtils::SaveDirtyPackages(false, true, true))
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" cannot save!"));
			OutputLog += NewLine;
			continue;
		}
		FAssetData NewDuplicatedMapAssetData(NewDuplicatedMap);

		Time_2 = FPlatformTime::Seconds();
		
		//==============================================================================================================
		//	3.Consolidate
		//==============================================================================================================

		TArray<UObject*> OldMap_Array{OldMapRE};
		TSet<UObject*> OldMap_Set_Within{OldMapRE};
		TSet<UObject*> OldMap_Set_NotWithin;
		// Perform the object consolidation
		ObjectTools::FConsolidationResults ConsResults =
			ObjectTools::ConsolidateObjects(
				NewDuplicatedMap,
				OldMap_Array,
				OldMap_Set_Within,
				OldMap_Set_NotWithin,
				true );

		// Check if the user has specified if they'd like to save the dirtied packages post-consolidation
		// If the consolidation resulted in failed (partially consolidated) objects, do not save
		if ( ConsResults.FailedConsolidationObjs.Num() > 0 )
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" cannot consolidate!"));
			OutputLog += NewLine;

			//if failed, restore
			ObjectTools::DeleteAssets({NewDuplicatedMapAssetData}, false);
			FAssetRenameData RenameDataRE(OldMapRE, FPaths::GetPath(RoomData.DevelopMap), FPaths::GetBaseFilename(RoomData.DevelopMap) );
			AssetTool.RenameAssetsWithDialog({RenameDataRE});
			
			continue;
		}
		FixupRedirectory_Develop({RoomData});
		//Save all
		if (!FEditorFileUtils::SaveDirtyPackages(false, true, true))
		{
			OutputLog += TEXT("Failed: ");
			OutputLog += (RoomData.DevelopMap + TEXT(" cannot save!"));
			OutputLog += NewLine;
			
			//if failed, restore
			ObjectTools::DeleteAssets({NewDuplicatedMapAssetData}, false);
			FAssetRenameData RenameDataRE(OldMapRE, FPaths::GetPath(RoomData.DevelopMap), FPaths::GetBaseFilename(RoomData.DevelopMap) );
			AssetTool.RenameAssetsWithDialog({RenameDataRE});
			
			continue;
		}
		
		Time_3 = FPlatformTime::Seconds();
		
		//==============================================================================================================
		//	SUCCESS
		//==============================================================================================================
		
		OutputLog += TEXT("Success: ");
		OutputLog += (RoomData.DevelopMap);
		OutputLog += (FString::Printf(TEXT(". Use time: %0.4fs + %0.4fs + %0.4fs ."), Time_1 - Time, Time_2 - Time_1, Time_3 - Time_2));
		OutputLog += NewLine;
		
		ChangedRoomData.Add(RoomData);
		Time = FPlatformTime::Seconds();
	}

	//End with reference fix
	FixupRedirectory_Develop(ChangedRoomData);

	TArray<FAssetData> OldMapBuildAssetData;
	
	for(const auto& room : ChangedRoomData)
	{
		FString DevPath = FPaths::GetPath(room.DevelopMap);
		// Form a filter from the paths
		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Emplace(*DevPath);
		Filter.ClassNames.Emplace(UMapBuildDataRegistry::StaticClass()->GetFName());

		// Query for a list of assets in the selected paths
		TArray<FAssetData> AssetList;
		AssetRegistry.GetAssets(Filter, AssetList);

		for(const FAssetData& AssetData : AssetList)
		{
			if(AssetData.AssetName.ToString().Contains(OldSuffix))
			{
				OldMapBuildAssetData.Add(AssetData);
			}
		}

	}

	bool bShowConfirmation = false;
#ifdef MAPMANAGER_DEVELOPING
	bShowConfirmation = true;
#endif

	ObjectTools::DeleteAssets(OldMapBuildAssetData, bShowConfirmation);
	
	FMessageDialog::Open(
        EAppMsgType::Ok,
        EAppReturnType::Ok,
		FText::FromString(OutputLog)
        );
}

void FMapManager::FixupRedirectory_Develop(const TArray<FMapManagerRowListViewData>& RoomData)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetTools& AssetTool = AssetToolsModule.Get();
	
	TArray<UObjectRedirector*> Redirectors;
	for(const auto& room : RoomData)
	{
		FString FixPath = FPaths::GetPath(room.DevelopMap);
		// Form a filter from the paths
		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.PackagePaths.Emplace(*FixPath);
		Filter.ClassNames.Emplace(UObjectRedirector::StaticClass()->GetFName());

		// Query for a list of assets in the selected paths
		TArray<FAssetData> AssetList;
		AssetRegistry.GetAssets(Filter, AssetList);

		if (AssetList.Num() > 0)
		{
			{
				for(const FAssetData& AssetData : AssetList)
				{
					// Load up the object
					UObject* LoadedObject = LoadObject<UObject>(NULL, *room.DevelopMap, NULL, LOAD_None, NULL);
					if (!AssetData.IsRedirector()) {
						continue;
					}
					UObjectRedirector* Redirector = Cast<UObjectRedirector>(LoadedObject);
					Redirectors.Add(Redirector);
				}
			}
		}

	}
	AssetTool.FixupReferencers(Redirectors);

}

FMapManagerRowListViewData& FMapManager::FindOrAddRow(const FName& RoomId)
{
	for(auto& mr : MapRows)
	{
		if(mr.RoomId == RoomId.ToString())
		{
			return mr;
		}
	}
	//if not contained
	FMapManagerRowListViewData Data(RoomId.ToString());
	int32 NewIndex = MapRows.Add(Data);
	return MapRows[NewIndex];
}

void FMapManager::CommitAllDesign()
{
	CommitDesignInternal(MapRows);
}

void FMapManager::CommitDesignSelected(const TArray<FMapManagerRowListViewDataPtr>& Selected)
{
	TArray<FMapManagerRowListViewData> Rows;
	for(const auto& ptr : Selected)
	{
		Rows.Add(*ptr);
	}
	CommitDesignInternal(Rows);
}

void FMapManager::RunDesignMap()
{
	FMapManagerModule& MapManagerModule = FModuleManager::GetModuleChecked<FMapManagerModule>( TEXT("MapManager") );
	MapManagerModule.DesignButtonClicked();
}

void FMapManager::RegenerateFromDevelopFolders()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	IAssetTools& AssetTool = AssetToolsModule.Get();
	
	TAssetPtr<UWorld> AssetPtr_Design = GetMutableDefault<UMapManagerConfig>()->DesignMap;
	TAssetPtr<UWorld> AssetPtr_Develop = GetMutableDefault<UMapManagerConfig>()->DevelopMap;
	UWorld* PL_Design = AssetPtr_Design.LoadSynchronous();
	UWorld* PL_Develop = AssetPtr_Develop.LoadSynchronous();

	if(!PL_Design || !PL_Develop) { return; }

	TArray<FName> Folders = GetMutableDefault<UMapManagerConfig>()->VillaFoldersInFullName;
	
	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Append(Folders);
	Filter.ClassNames.Emplace(UWorld::StaticClass()->GetFName());

	// Query for a list of assets in the selected paths
	TArray<FAssetData> AssetList;
	AssetRegistry.GetAssets(Filter, AssetList);

	TMap<FString, FString> Assets_Develop_Design_Map;

	// Now we've got all the map asset which should be in villa map
	// There are 3 things to do:
	// 1. Generate map table from assets
	// 2. Modify data table from assets
	// 3. Generate villa (and villa design) from assets

	//	///////////////////////////////////////////////////////////////////
	//	1. Generate map table from assets
	//	///////////////////////////////////////////////////////////////////
	
	for(const auto& a : AssetList)
	{
		FName path = a.PackagePath;
		if(path.ToString().Contains(TEXT("Static")))
		{
			FName name = a.AssetName;
			FMapManagerRowListViewData& Data = FindOrAddRow(name);
			Data.DevelopMap = a.PackageName.ToString();
			if (Data.DesignMap.IsEmpty()) {
				Data.DesignMap = EMPTY_PLACE_HOLDER;
			}
			Assets_Develop_Design_Map.FindOrAdd(Data.DevelopMap, Data.DesignMap);
		};
	}
	WriteTableIntoFile(MapRows);
	//Refresh later, make sure file handler closed

	//	///////////////////////////////////////////////////////////////////
	//	2. Modify data table from assets
	//	///////////////////////////////////////////////////////////////////

	/*FSoftClassPath SDGI_ClassSoft = GetMutableDefault<UGameMapsSettings>()->GameInstanceClass;
	UClass* SDGI_Class = SDGI_ClassSoft.TryLoadClass<USDGameInstance>();
	USDGameInstance* SDGI = Cast<USDGameInstance>(SDGI_Class->ClassDefaultObject);
	UDataTable* Table = SDGI->GetDTSubLevelGraph();
	const bool bMarkDirty = Table->MarkPackageDirty();
	
	for(const auto& a : AssetList)
	{
		FName path = a.PackageName;
		if(path.ToString().Contains(TEXT("Static")))
		{
			FName name = a.AssetName;
			FDTSubLevelNode* TableRow = Table->FindRow<FDTSubLevelNode>(name, TEXT("FMapManager::RegenerateFromDevelopFolders"), false);
			if(TableRow)
			{
				TableRow->StaticLevelName = a.PackageName;
			}
			else
			{
				FDTSubLevelNode NewNode;
				NewNode.StaticLevelName = a.PackageName;
				Table->AddRow(name, NewNode);
			}
		}else if(path.ToString().Contains(TEXT("Dynamic")))
		{
			//TODO: Fix dynamic level field in data table
			//[FIX DELAY] Less priority
		}
	}

	//Refresh now, make sure file handler closed
	RefreshTableFromFile();*/


	UE_LOG(LogTemp, Warning, TEXT("FMapManager::RegenerateFromDevelopFolders STEP 2 no implementation !"));

	
	//	///////////////////////////////////////////////////////////////////
	//	3. Generate villa (and villa design) from assets
	//	///////////////////////////////////////////////////////////////////

	PL_Design->ClearStreamingLevels();
	PL_Develop->ClearStreamingLevels();

	for(const auto& a : AssetList)
	{
		FString DevelopPath = a.PackageName.ToString();
		FString Folder;
		//Get folder path here
		for(const FName& folder : Folders)
		{
			if(DevelopPath.StartsWith(folder.ToString()))
			{
				FString Leaf = FPaths::GetPathLeaf(folder.ToString());
				Folder = Leaf / DevelopPath.Replace(*folder.ToString(), TEXT(""));
				int32 SlashIndex;
				if (Folder.FindLastChar(TEXT('/'), SlashIndex)) {
					Folder = Folder.LeftChop(Folder.Len() - SlashIndex);
				}
				break;
			}
		}

		const bool bAlwaysLoaded = DevelopPath.Contains(TEXT("Persistent"));
		//const bool bAlwaysLoaded = false;
		
		{
			auto Level = AddLevelToWorld(PL_Develop, DevelopPath, bAlwaysLoaded);

			if (DevelopPath.Contains(TEXT("Static"))) {
				Level->bIsStatic = true;
			}

			SetFolderPath(PL_Develop, DevelopPath, *Folder);
		}
		{
			auto SubLevels = GetSublevelNames(DevelopPath, TEXT("_L"));
			for(const auto& str : SubLevels)
			{
				if(IsSublevelInitiallyLoaded(DevelopPath, str))
				{
					AddLevelToWorld(PL_Develop, str, bAlwaysLoaded);
					SetFolderPath(PL_Develop, str, *Folder);
				}
			}
		}
		
		FString DesignPath = DevelopPath;
		FString* PathPtr = Assets_Develop_Design_Map.Find(DevelopPath);
		if(PathPtr)
		{
			DesignPath = *PathPtr;
		}
		{
			AddLevelToWorld(PL_Design, DesignPath, bAlwaysLoaded);
			SetFolderPath(PL_Design, DesignPath, *Folder);
		}
		{
			auto SubLevels = GetSublevelNames(DesignPath, TEXT("_L"));
			for(const auto& str : SubLevels)
			{
				if(IsSublevelInitiallyLoaded(DesignPath, str))
				{
					AddLevelToWorld(PL_Design, str, bAlwaysLoaded);
					SetFolderPath(PL_Design, str, *Folder);
				}
			}
		}
	}

	//SaveMaps
	UEditorLoadingAndSavingUtils::SaveMap(PL_Design, AssetPtr_Design.GetLongPackageName());
	UEditorLoadingAndSavingUtils::SaveMap(PL_Develop, AssetPtr_Develop.GetLongPackageName());
	
}

#undef LOCTEXT_NAMESPACE
	