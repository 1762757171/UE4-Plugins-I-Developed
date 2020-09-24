#pragma once

#include "CoreMinimal.h"
#include "SCheckBox.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "access_private.hpp"

ACCESS_PRIVATE_FIELD(ULevelStreaming, ULevelStreaming::ECurrentState, CurrentState);

using FMapLevelLoaderDataPtr = TSharedPtr<struct FMapLevelLoaderData>;

struct FMapLevelLoaderData
{
    bool bLoaded;
    FString LevelStreaming_or_HierarchyName;
    bool IsFolder;

    TArray<FMapLevelLoaderDataPtr> Children;

public:
    TArray<FString> GetAllChildLeaves() const;
    ECheckBoxState GetLoadedState() const;
};

const bool operator<(const FMapLevelLoaderDataPtr& lhs, const FMapLevelLoaderDataPtr& rhs);

struct TreeHierarchy : public TSharedFromThis<struct TreeHierarchy>
{
    TMap<FString, TSharedPtr<struct TreeHierarchy>> Hierarchy;
    TArray<FMapLevelLoaderDataPtr> ChildrenAsset;
public:
    TSharedPtr<struct TreeHierarchy> BuildTree(const FString& Path, const FString& Parent = TEXT(""));
    TSharedPtr<struct TreeHierarchy> FindOrAdd(const FString& Leaf);
    void AddChild(FMapLevelLoaderDataPtr Child);
    void AddChildWithHierarchy(FMapLevelLoaderDataPtr Child, const FString& Path);
    FMapLevelLoaderDataPtr MakeSelf() const;
    FMapLevelLoaderDataPtr GeneratedDataTree(FMapLevelLoaderDataPtr SelfMade = nullptr) const;
};


DECLARE_MULTICAST_DELEGATE(FOnLoaderFlushed);

class FMapLoader
{
public:
    bool Initialize();

    void OpenMap();

    void LoadLevel(FString Level);
    void UnloadLevel(FString Level);
    void LoadLevels(const TArray<FString>& Levels);
    void UnloadLevels(const TArray<FString>& Levels);

    void SetFilter(const FString& Filter);
    TArray<FMapLevelLoaderDataPtr> GenerateData() const;

    void ShowUCX(const FString& Level);
    void HideUCX(const FString& Level);

public:
    FOnLoaderFlushed OnLoaderFlushed;

    static bool IsInTargetLevel();
private:
    void Flush();
    void WriteIntoConfig();
private:
    TArray<class ULevelStreaming*> AllLevels;
    TArray<FString> LoadedLevelNames;
    FString LevelPathName;
    class UWorld* PersistentWorld = nullptr;
    TArray<FString> ExpandedFolder;
    FString FilterString;
};
