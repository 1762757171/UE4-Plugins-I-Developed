#pragma once

#include "AssetData.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapManagerUtils.generated.h"

//#define MAPMANAGER_DEVELOPING 1

struct FMapManagerRowListViewData
{
    FString RoomId;
    
    FString DesignMap;

    FString DevelopMap;

public:
    FMapManagerRowListViewData(FString _RoomId, FString _Design = FString(), FString _Dev = FString())
        :RoomId(_RoomId), DesignMap(_Design), DevelopMap(_Dev)
    {}

public:
    static FName ColumnName_RoomId;
    static FName ColumnName_Design;
    static FName ColumnName_Develop;
};

using FMapManagerRowListViewDataPtr = TSharedPtr<struct FMapManagerRowListViewData>;

UCLASS()
class UEditableDataTable : public UDataTable
{
    GENERATED_BODY()
public:
    // TODO: remove this, it is temporarily here to allow DataTableEditorUtils to compile until I get around to updating functions like RemoveRow and RenameRow
    virtual TMap<FName, uint8*>& GetNonConstRowMap() override { return RowMap; }
};

namespace MapManagerUtils
{

    bool GetMapAssetFromPath(const FString& Str, FAssetData& OutData, FString& ErrorMsg_Append);
}