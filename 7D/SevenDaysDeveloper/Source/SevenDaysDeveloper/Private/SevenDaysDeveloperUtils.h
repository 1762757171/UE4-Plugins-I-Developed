#pragma once

#include "CoreMinimal.h"
#include "SevenDaysDeveloperRuntime/Public/SevenDaysDeveloperRuntimeModule.h"
#ifdef WITH_EDITOR
#include "Editor.h"
#endif

struct FSevenDaysDeveloperRowData
{
public:
    FName RoomName;
    bool IsLoaded;
    bool IsVisible;

public:
    FSevenDaysDeveloperRowData(const FName& _RoomName = FName(), bool bLoaded = false, bool bVisible = false)
        : RoomName(_RoomName), IsLoaded(bLoaded), IsVisible(bVisible)
    {}
};

using FSevenDaysDeveloperRowDataPtr = TSharedPtr<FSevenDaysDeveloperRowData>;

namespace SevenDaysDeveloperUtils
{
    static FSevenDaysDeveloperRuntimeModule& GetRuntimeModule()
    {
        return FModuleManager::LoadModuleChecked<FSevenDaysDeveloperRuntimeModule>("SevenDaysDeveloperRuntime");
    }
}