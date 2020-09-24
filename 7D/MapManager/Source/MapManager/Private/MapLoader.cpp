
#include "MapLoader.h"

#include "AssetEditorManager.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "Editor.h"
#include "EditorLevelUtils.h"
#include "FileHelpers.h"
#include "GameMapsSettings.h"
#include "MapManagerConfig.h"
#include "MapManagerUtils.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/KismetSystemLibrary.h"


TArray<FString> FMapLevelLoaderData::GetAllChildLeaves() const
{
    if(!IsFolder)
    {
        return {LevelStreaming_or_HierarchyName};
    }
    
    TArray<FString> Leaves;
    for(const auto& c : Children)
    {
        Leaves.Append(c->GetAllChildLeaves());
    }
    return Leaves;
}

ECheckBoxState FMapLevelLoaderData::GetLoadedState() const
{
    if(!IsFolder)
    {
        return bLoaded ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
    }

    int32 Checks = 0;
    for(const auto& c : Children)
    {
        ECheckBoxState CurrState = c->GetLoadedState();
        switch (CurrState)
        {
            case ECheckBoxState::Undetermined:
                return ECheckBoxState::Undetermined;
            case ECheckBoxState::Checked:
                Checks++;
                break;
            case ECheckBoxState::Unchecked:
                break;
        }
    }

    if(Checks == 0)
    {
        return ECheckBoxState::Unchecked;
    }
    if(Checks == Children.Num())
    {
        return ECheckBoxState::Checked;
    }
    
    return ECheckBoxState::Undetermined;
}

const bool operator<(const FMapLevelLoaderDataPtr& lhs, const FMapLevelLoaderDataPtr& rhs)
{
    return lhs->LevelStreaming_or_HierarchyName < rhs->LevelStreaming_or_HierarchyName;
}

TSharedPtr<TreeHierarchy> TreeHierarchy::BuildTree(const FString& Path, const FString& Parent)
{
    int32 Pos;
    if(!Path.FindChar('/', Pos))
    {
        return SharedThis(this);
    }
	const FString ThisFolder = Parent / Path.Left(Pos);
	const FString ChildFolder = Path.Right(Path.Len() - Pos - 1);
    auto ChildTree = FindOrAdd(ThisFolder);
    return ChildTree->BuildTree(ChildFolder, ThisFolder);
}

TSharedPtr<TreeHierarchy> TreeHierarchy::FindOrAdd(const FString& Leaf)
{
    TSharedPtr<TreeHierarchy>& SubtreePtr = Hierarchy.FindOrAdd(Leaf);
    if(!SubtreePtr.IsValid())
    {
        SubtreePtr = MakeShareable(new TreeHierarchy);
    }
    return SubtreePtr;
}

void TreeHierarchy::AddChild(FMapLevelLoaderDataPtr Child)
{
    ChildrenAsset.Add(Child);
}

void TreeHierarchy::AddChildWithHierarchy(FMapLevelLoaderDataPtr Child, const FString& Path)
{
    BuildTree(Path)->AddChild(Child);
}

FMapLevelLoaderDataPtr TreeHierarchy::MakeSelf() const
{
    FMapLevelLoaderDataPtr ThisData = MakeShareable(new FMapLevelLoaderData);
    ThisData->bLoaded = false;
    ThisData->IsFolder = true;
    ThisData->LevelStreaming_or_HierarchyName = TEXT("Villa");
    ThisData->Children = ChildrenAsset;
    ThisData->Children.Sort();
    return ThisData;
}

FMapLevelLoaderDataPtr TreeHierarchy::GeneratedDataTree(FMapLevelLoaderDataPtr SelfMade) const
{
    FMapLevelLoaderDataPtr ThisData = SelfMade ? SelfMade : MakeSelf();

    TArray<TSharedPtr<TreeHierarchy>> ChildFolders;
    Hierarchy.GenerateValueArray(ChildFolders);

    TArray<FString> Keys;
    Hierarchy.GenerateKeyArray(Keys);
    Keys.Sort();

	//ThisData->Children.Empty();
    for(const auto& k : Keys)
    {
        const auto& v = Hierarchy[k];
        FMapLevelLoaderDataPtr VSelf = v->MakeSelf();
        VSelf->LevelStreaming_or_HierarchyName = k;
        VSelf = v->GeneratedDataTree(VSelf);
        ThisData->Children.AddUnique(VSelf);
    }

    return ThisData;
}

bool FMapLoader::Initialize()
{
    //FName MapName = FName(* GetMutableDefault<UMapManagerConfig>()->DevelopMap );
    //WorldPackage = LoadPackage(nullptr, *MapName.ToString(), LOAD_None);
    //PersistentWorld = UWorld::FindWorldInPackage(WorldPackage);

    TAssetPtr<UWorld> DevelopMap = GetMutableDefault<UMapManagerConfig>()->DevelopMap;

	if (DevelopMap.IsNull()) {
		return false;
	}

    LevelPathName = DevelopMap.ToString();
    PersistentWorld = DevelopMap.LoadSynchronous();

	if (!PersistentWorld) {
		return false;
	}

    AllLevels = PersistentWorld->GetStreamingLevels();

    
    if(GetMutableDefault<UMapManagerPlaySettings>()->LastLoadedMaps.Num() != 0){
        LoadedLevelNames = GetMutableDefault<UMapManagerPlaySettings>()->LastLoadedMaps;
    }else
    {
        LoadedLevelNames = GetMutableDefault<UMapManagerConfig>()->MapsAlwaysLoadInFullName;
        WriteIntoConfig();
    }

    /*for(auto sl : AllLevels)
    {
        FString LevelNameStr = sl->GetWorldAssetPackageName();
        if(MapsNameAlwaysLoad.Contains(LevelNameStr))
        {
            LoadedLevelNames.Add(LevelNameStr);
        }
    }*/
	return true;
}

void FMapLoader::OpenMap()
{
    FEditorFileUtils::SaveDirtyPackages(true, true, true);

    if(IsInTargetLevel())
    {
        return;
    }
    
    Flush();

    FEditorFileUtils::LoadMap(LevelPathName, false, true);

    Initialize();
}

void FMapLoader::LoadLevel(FString Level)
{
    LoadLevels({Level});
}

void FMapLoader::UnloadLevel(FString Level)
{
   UnloadLevels({Level});
}

void FMapLoader::LoadLevels(const TArray<FString>& Levels)
{
    if(!IsInTargetLevel())
    {
        return;
    }

    bool AtLeastOneChanged = false;

    for(const FString& l : Levels)
    {
        if(!LoadedLevelNames.Contains(l))
        {
            AtLeastOneChanged = true;
            LoadedLevelNames.Add(l);
        }
    }

    if(AtLeastOneChanged)
    {
        Flush();
    }
}

void FMapLoader::UnloadLevels(const TArray<FString>& Levels)
{
    if(!IsInTargetLevel())
    {
        return;
    }
    
    bool AtLeastOneChanged = false;

    for(const FString& l : Levels)
    {
        if(LoadedLevelNames.Contains(l))
        {
            AtLeastOneChanged = true;
            LoadedLevelNames.Remove(l);
        }
    }
    
    if(AtLeastOneChanged)
    {
        Flush();
    }
}

void FMapLoader::SetFilter(const FString& Filter)
{
    FilterString = Filter;
}

TArray<FMapLevelLoaderDataPtr> FMapLoader::GenerateData() const
{
    TSharedPtr<TreeHierarchy> Root(new TreeHierarchy);
    
    for(const auto& l : AllLevels)
    {
        const FString& AssetName = l->GetWorldAssetPackageName();
        const FString& Folder = l->GetFolderPath().ToString();
        const FString& LevelPathInWorld = Folder / FPaths::GetBaseFilename(AssetName);
        if(!FilterString.IsEmpty() && !LevelPathInWorld.Contains(FilterString))
        {
            continue;
        }
        FMapLevelLoaderDataPtr Data = MakeShareable(new FMapLevelLoaderData);
        Data->IsFolder = false;
        Data->LevelStreaming_or_HierarchyName = AssetName;
        Data->bLoaded = LoadedLevelNames.Contains(Data->LevelStreaming_or_HierarchyName);
        FString Hierarchy = Folder + TEXT("/");
        Root->AddChildWithHierarchy(Data, Hierarchy);
    }

    return {Root->GeneratedDataTree(nullptr)};
}

void FMapLoader::ShowUCX(const FString& Level)
{
    if(!Initialize()) { return; }
    ULevelStreaming* LevelStreaming = PersistentWorld->GetLevelStreamingForPackageName(*Level);
    ULevel* LevelLoaded = LevelStreaming->GetLoadedLevel();
    if(!LevelLoaded) { return; }
    const auto& Actors = LevelLoaded->Actors;
    for(const auto& a : Actors)
    {
        if(a->GetActorLabel().Contains(TEXT("UCX")))
        {
            a->SetIsTemporarilyHiddenInEditor( false );
        }
    }
}

void FMapLoader::HideUCX(const FString& Level)
{
    if(!Initialize()) { return; }
    ULevelStreaming* LevelStreaming = PersistentWorld->GetLevelStreamingForPackageName(*Level);
    ULevel* LevelLoaded = LevelStreaming->GetLoadedLevel();
    if(!LevelLoaded) { return; }
    const auto& Actors = LevelLoaded->Actors;
    for(const auto& a : Actors)
    {
        if(a->GetActorLabel().Contains(TEXT("UCX")))
        {
            a->SetIsTemporarilyHiddenInEditor( true );
        }
    }
}

void FMapLoader::Flush()
{
    WriteIntoConfig();

    //FAssetEditorManager::Get().OpenEditorForAsset(TEXT("/Engine/Maps/Entry"));

    if(!Initialize())
    {
        return;
    }
    
    /*PersistentWorld->SetShouldForceUnloadStreamingLevels(true);
    
    PersistentWorld->FlushLevelStreaming();
    
    PersistentWorld->SetShouldForceUnloadStreamingLevels(false);*/

    const auto& levels = PersistentWorld->GetStreamingLevels();
    
    /*for(const auto& l : levels)
    {
        if(LoadedLevelNames.Contains(l->GetWorldAssetPackageName()))
        {
            l->SetShouldBeVisibleInEditor(true);
            PersistentWorld->UpdateStreamingLevelShouldBeConsidered(l);
        }
    }
    
    FEditorFileUtils::LoadMap(LevelPathName, false, true);*/
    for(const auto& l : levels)
    {
        auto& CurrentState = access_private::CurrentState(*l);
		FString AssetName = l->GetWorldAssetPackageName();
        if(LoadedLevelNames.Contains(AssetName))
        {
            if(l->HasLoadedLevel())
            {
                UEditorLevelUtils::SetLevelVisibility(l->GetLoadedLevel(), true, true, ELevelVisibilityDirtyMode::DontModify);
            }
            else
            {
                CurrentState = ULevelStreaming::ECurrentState::Unloaded;
				l->SetShouldBeVisibleInEditor(true);
            }
        }else
        {
            if(l->HasLoadedLevel())
            {
                UEditorLevelUtils::SetLevelVisibility(l->GetLoadedLevel(), false, false, ELevelVisibilityDirtyMode::DontModify);
            }
            else
            {
                CurrentState = ULevelStreaming::ECurrentState::FailedToLoad;
            }
        }
    }
    if(IsInTargetLevel())
    {
        PersistentWorld->PopulateStreamingLevelsToConsider();
        PersistentWorld->FlushLevelStreaming();
    }
    
    OnLoaderFlushed.Broadcast();
}

void FMapLoader::WriteIntoConfig()
{
    UMapManagerPlaySettings* PlaySettings = GetMutableDefault<UMapManagerPlaySettings>();
    PlaySettings->LastLoadedMaps = LoadedLevelNames;
	
    FPropertyChangedEvent PropChangeEvent(UMapManagerPlaySettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMapManagerPlaySettings, LastLoadedMaps)));
    PlaySettings->PostEditChangeProperty(PropChangeEvent);
	
    PlaySettings->SaveConfig();
}


bool FMapLoader::IsInTargetLevel()
{
    return (GEditor->GetEditorWorldContext().World()->GetName() == GetMutableDefault<UMapManagerConfig>()->DevelopMap.GetAssetName());
}
