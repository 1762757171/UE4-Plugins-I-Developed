

#include "LightViewer.h"


#include "Editor.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"
#include "OpenGLDrv/Private/Android/AndroidOpenGL.h"


const FName FLightViewerMap::COLUMN_USING(TEXT("Column_Using"));
const FName FLightViewerMap::COLUMN_LEVEL(TEXT("Column_Level"));

FLightViewer::FLightViewer()
{
}

FLightViewer::~FLightViewer()
{
    UWorld* PlayWorld = GEditor->PlayWorld;
    if(!PlayWorld) { return; }

    for(const auto& l : Levels)
    {
        PlayWorld->RemoveStreamingLevels(Levels);
    }
}

bool FLightViewer::ChangeLevelVisibility(const FLightViewerMap& Info)
{
    UWorld* PlayWorld = GEditor->PlayWorld;
    if(!PlayWorld) { return false; }

    ULevelStreaming* LevelStreamingAlwaysLoaded = nullptr;

    //Level already added
    for(const auto& l : Levels)
    {
        if(l->GetWorldAsset() == Info.Level)
        {
            LevelStreamingAlwaysLoaded = l;
            break;
        }
    }

    //Level not added but world contain this level
    //this means world already have this streaming level
    //it will not take any effect
    if(!LevelStreamingAlwaysLoaded)
    {
        TArray< ULevelStreaming* > LevelStreaming = PlayWorld->GetStreamingLevels();
        for(const auto& l : LevelStreaming)
        {
            if(l->GetWorldAsset() == Info.Level)
            {
                return false;
            }
        }
    }
    
    if(!LevelStreamingAlwaysLoaded)
    {
        LevelStreamingAlwaysLoaded =
            NewObject<ULevelStreamingAlwaysLoaded>(
                PlayWorld,
                ULevelStreamingAlwaysLoaded::StaticClass(),
                NAME_None,
                RF_Transient,
                NULL
                );

        LevelStreamingAlwaysLoaded->SetWorldAsset(Info.Level);

        LevelStreamingAlwaysLoaded->LevelTransform = FTransform::Identity;

        // Seed the level's draw color.
        LevelStreamingAlwaysLoaded->LevelColor = FLinearColor::MakeRandomColor();

        // Add the new level to world.
        PlayWorld->AddStreamingLevel(LevelStreamingAlwaysLoaded);

        Levels.Add(LevelStreamingAlwaysLoaded);
    }

    LevelStreamingAlwaysLoaded->SetShouldBeVisible(Info.bUsing);

    return true;
}
