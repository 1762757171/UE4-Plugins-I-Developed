#pragma once

#include "MapManagerUtils.h"

#include "AssetRegistryModule.h"
#include "CoreMinimal.h"

FName FMapManagerRowListViewData::ColumnName_RoomId(TEXT("RoomId"));
FName FMapManagerRowListViewData::ColumnName_Design(TEXT("Design"));
FName FMapManagerRowListViewData::ColumnName_Develop(TEXT("Develop"));

bool MapManagerUtils::GetMapAssetFromPath(const FString& Str, FAssetData& OutData, FString& ErrorMsg_Append)
{
    if(Str.IsEmpty())
    {
        ErrorMsg_Append += TEXT("Failed: Empty map path!");
        return false;
    }
    
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
    
    TArray<FAssetData> AssetDataArray;
    
    AssetRegistry.GetAssetsByPackageName(FName(*Str), AssetDataArray);
    if(AssetDataArray.Num() == 0)
    {
        ErrorMsg_Append += TEXT("Failed: Map<");
        ErrorMsg_Append += (Str + TEXT("> not exist!"));
        return false;
    }
    else if(AssetDataArray.Num() != 1)
    {
        ErrorMsg_Append += TEXT("Failed: Map<");
        ErrorMsg_Append += (Str + TEXT("> not unique!"));
        return false;
    }		
    OutData = AssetDataArray[0];
    return true;
}
