// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MapManagerUtils.h"

class FMapManager
{
public:
	FMapManager();

public:	// for Map manager module
	
	void SetTableFilePath(FString TableFilePath);
	void RefreshTableFromFile();
	bool WriteTableIntoFile(const TArray<FMapManagerRowListViewData>& NewData);
	
public:	// public getter
	
	const TArray<FMapManagerRowListViewData>& GetMapRows() const;

public:	// Button method
	
	void CommitAllDesign();
	void CommitDesignSelected(const TArray<FMapManagerRowListViewDataPtr>& Selected);

	void RunDesignMap();

	void RegenerateFromDevelopFolders();
	
public:	// function use internal and may be used by other class

	static bool DoWorldHaveLevel(UWorld* World, const FString& LevelName);
	static class ULevelStreaming* AddLevelToWorld(UWorld* World, const FString& LevelName, bool bAlwaysLoaded);
	static TArray<FString> GetSublevelNames(const FString& PersistentLevelName, const FString& SuffixToIgnore);
	static bool IsSublevelInitiallyLoaded(const FString& PersistentLevelName, const FString& SublevelName);
	static bool RemoveLevelFromWorld(UWorld* World, const FString& LevelName);
	static bool GetFolderPath(UWorld* World, const FString& LevelName, FName& OutFolder);
	static void SetFolderPath(UWorld* World, const FString& LevelName, const FName& Folder);
protected:
	/*FAssetToolkit interface*/
	/*virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;*/
	/*FAssetToolkit interface*/
	void CommitDesignInternal(const TArray<FMapManagerRowListViewData>& Maps);
	void FixupRedirectory_Develop(const TArray<FMapManagerRowListViewData>& RoomData);

private:
	FMapManagerRowListViewData& FindOrAddRow(const FName& RoomId);
private:
	FString _TablePath;
	TArray<FMapManagerRowListViewData> MapRows;
public:
	static const FString DELETED_ROOM_ID;
	static const FString EMPTY_PLACE_HOLDER;
};
