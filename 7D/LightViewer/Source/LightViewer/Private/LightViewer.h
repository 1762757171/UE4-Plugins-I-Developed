
#pragma once
#include "AssetPtr.h"

struct FLightViewerMap
{
    bool bUsing;
    TAssetPtr<class UWorld> Level;

    static const FName COLUMN_USING;
    static const FName COLUMN_LEVEL;
};

using FLightViewerMapPtr = TSharedPtr<FLightViewerMap>;

class FLightViewer
{
public:
    FLightViewer();
    ~FLightViewer();

public:
    bool ChangeLevelVisibility(const FLightViewerMap& Info);

private:
    TArray<class ULevelStreaming *> Levels;
    
};