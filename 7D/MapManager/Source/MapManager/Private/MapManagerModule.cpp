// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MapManagerModule.h"
#include "MapManager.h"
#include "AssetEditorManager.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "DataTableEditorUtils.h"
#include "DebuggerCommands.h"
#include "EditorViewportClient.h"
#include "FileHelper.h"
#include "FileHelpers.h"
#include "GameMapsSettings.h"
#include "IPluginManager.h"
#include "ISettingsModule.h"
#include "MapManagerStyle.h"
#include "LevelEditor.h"
#include "MapLoader.h"
#include "MapManagerCommands.h"
#include "MessageDialog.h"
#include "SButton.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "SMapManager.h"
#include "MapManagerConfig.h"
#include "MultiBoxBuilder.h"
#include "SDGameInstance.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "MapManagerUtils.h"
#include "SMapLoader.h"
#include "LevelEditor/Private/LevelEditorContextMenu.h"

static const FName MapManagerTabName("MapManager");
static const FName MapLoaderTabName("MapLoader");

#define LOCTEXT_NAMESPACE "FMapManagerModule"

void FMapManagerModule::StartupModule()
{
	FMapManagerStyle::Initialize();
	FMapManagerStyle::ReloadTextures();

	FMapManagerCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
        FMapManagerCommands::Get().CommandAction_Last,
        FExecuteAction::CreateRaw(this, &FMapManagerModule::PlayLastCommand),
        FCanExecuteAction::CreateRaw(this, &FMapManagerModule::LastCommandCanExecute)
        );

	PluginCommands->MapAction(
        FMapManagerCommands::Get().CommandAction_Develop,
        FExecuteAction::CreateRaw(this, &FMapManagerModule::DevelopButtonClicked),
        FCanExecuteAction::CreateRaw(this, &FMapManagerModule::DevelopButtonCanExecute)
        );

	PluginCommands->MapAction(
        FMapManagerCommands::Get().CommandAction_Design,
        FExecuteAction::CreateRaw(this, &FMapManagerModule::DesignButtonClicked),
        FCanExecuteAction::CreateRaw(this, &FMapManagerModule::DesignButtonCanExecute)
        );
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FMapManagerModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		
	}

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	//

	Manager = MakeShareable<FMapManager>(new FMapManager);
	const FString TablePath = IPluginManager::Get().FindPlugin("MapManager")->GetBaseDir() / TEXT("Data");
	Manager->SetTableFilePath(TablePath);
	Manager->RefreshTableFromFile();

	Loader = MakeShareable<FMapLoader>(new FMapLoader);
	
	// Register custom config into project settings
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterSettings("Project", "SD", "MapManager",
        LOCTEXT("RuntimeSettingsName", "Map Manager"),
        LOCTEXT("RuntimeSettingsDescription", "Map manager for designer and developer"),
        GetMutableDefault<UMapManagerConfig>()
    );

	// Add Tool interface into Development Tools menu
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MapManagerTabName, FOnSpawnTab::CreateRaw(this, &FMapManagerModule::OnSpawnManagerTab))
        .SetDisplayName(NSLOCTEXT("FMapManagerModule", "TabTitle", "Map Manager"))
        .SetTooltipText(NSLOCTEXT("FMapManagerModule", "TooltipText", "Open the Map Manager tab."))
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
        .SetIcon(FSlateIcon(FMapManagerStyle::GetStyleSetName(), "MapManager.Manager"));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(MapLoaderTabName, FOnSpawnTab::CreateRaw(this, &FMapManagerModule::OnSpawnLoaderTab))
        .SetDisplayName(NSLOCTEXT("FMapManagerModule", "LoaderTabTitle", "Level Loader"))
        .SetTooltipText(NSLOCTEXT("FMapManagerModule", "LevelLoaderTooltipText", "Open the Level Loader tab."))
        //.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
        //.SetIcon(FSlateIcon(FMapManagerStyle::GetStyleSetName(), "MapManager.Manager"));
	;

}

void FMapManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	Manager.Reset();
	
	FMapManagerStyle::Shutdown();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MapManagerTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(MapLoaderTabName);
	/**/
}

TSharedRef<SDockTab> FMapManagerModule::OnSpawnManagerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab);

	TSharedPtr<SWidget> TabContent =
        SNew(SMapManager, Tab, SpawnTabArgs.GetOwnerWindow())
            .MapManager(Manager)
            .MapLoader(Loader)
    ;

	Tab->SetContent(TabContent.ToSharedRef());

	return Tab;
}

TSharedRef<SDockTab> FMapManagerModule::OnSpawnLoaderTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> Tab = SNew(SDockTab)
         .TabRole(ETabRole::NomadTab);
 
 	TSharedPtr<SWidget> TabContent =
         SNew(SMapLoader, Tab, SpawnTabArgs.GetOwnerWindow())
             .Loader(Loader)
     ;
 
 	Tab->SetContent(TabContent.ToSharedRef());
 
 	return Tab;
}

void FMapManagerModule::PlayLastCommand()
{
	// Let a game have a go at settings before we play
	UMapManagerPlaySettings* PlaySettings = GetMutableDefault<UMapManagerPlaySettings>();
	PlaySettings->PostEditChange();

	// Grab the play command and execute it
	TSharedRef<FUICommandInfo> LastCommand = GetLastExecuteCommand();
	UE_LOG(LogTemp, Log, TEXT("Repeating last play command: %s"), *LastCommand->GetLabel().ToString());

	PluginCommands->ExecuteAction(LastCommand);
}

bool FMapManagerModule::LastCommandCanExecute() const
{
	return PluginCommands->CanExecuteAction( GetLastExecuteCommand() );
}

void FMapManagerModule::DevelopButtonClicked()
{
	if(!DevelopButtonCanExecute())
	{
		return;
	}
	
	GoStartupMapAndPlay();
	
	SetLastExecutedPlayMode(EMapManagerLastPlayMode::Develop);
}

bool FMapManagerModule::DevelopButtonCanExecute() const
{
	return !IsGamePlaying();
}

void FMapManagerModule::DesignButtonClicked()
{
	if(!DesignButtonCanExecute())
	{
		return;
	}
	
	BeginDesignGameDelegateHandle = FEditorDelegates::PostPIEStarted.AddRaw(this, &FMapManagerModule::Event_OnBeginDesignGame);
    
	GoStartupMapAndPlay();
    
	SetLastExecutedPlayMode(EMapManagerLastPlayMode::Design);
}

bool FMapManagerModule::DesignButtonCanExecute() const
{
	return !IsGamePlaying();
}

void FMapManagerModule::GoStartupMapAndPlay()
{
	const FString EditorStartupMap = GetDefault<UGameMapsSettings>()->EditorStartupMap.GetLongPackageName();

	if(EditorStartupMap.IsEmpty())
	{
		FMessageDialog::Open(
        EAppMsgType::Ok,
        EAppReturnType::Ok,
        LOCTEXT("Prompt_ConfigError", "Invalid editor startup map in config!")
        );
		return;
	}
	
	GoMapAndPlay(EditorStartupMap);
}

void FMapManagerModule::GoMapAndPlay(const FString& MapLongPkgName)
{
	//if running, do nothing
	if(IsGamePlaying())
	{
		UE_LOG(LogTemp, Warning, TEXT("Starter run when running PIE."));
		return;
	}

	//Get current map state
	{
		UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	
		if(!EditorWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("<FMapManagerModule::CommandAction_Develop Clicked> : EditorWorld is null!"));
			return;
		}
		const FString LevelName = FEditorFileUtils::GetFilename(EditorWorld->GetCurrentLevel());
	
		EditingMapBeforePlay = LevelName;

		FEditorViewportClient* EditorViewClient = GetEditorViewportClient();
		
		if( EditorViewClient )
		{
			EditingViewLocation = EditorViewClient->GetViewLocation();
			EditingViewRotation = EditorViewClient->GetViewRotation();
		}
	}
	
	//Get wanted map, and load it if not editing
	{
		const FString GotoMap = MapLongPkgName;
		
		FString MapFileNameToLoad;
		if ( !FPackageName::IsValidLongPackageName(GotoMap, true) )
		{
			return;
		}
		MapFileNameToLoad = FPackageName::LongPackageNameToFilename( GotoMap );
		MapFileNameToLoad += FPackageName::GetMapPackageExtension();
		PieMap = MapFileNameToLoad;

		
		if(EditingMapBeforePlay != PieMap)
		{
			// If there are any unsaved changes to the current level, see if the user wants to save those first.
			if (!FEditorFileUtils::SaveDirtyPackages(true, true, true))
			{
				return;
			}
			FEditorFileUtils::LoadMap(PieMap, false, true);
		}
		
	}

	//when stop play, load last editing map
	{
		PreEndPieDelegateHandler = FEditorDelegates::PrePIEEnded.AddRaw(this, &FMapManagerModule::StopPieWrapper);
	}
	
	//Start Play in Editor
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );

		const TSharedPtr<IAssetViewport> ActiveLevelViewport = LevelEditorModule.GetFirstActiveViewport();

		// If there is an active level view port, play the game in it.
		GUnrealEd->RequestPlaySession(
            true,
            ActiveLevelViewport,
            false
            );
	}

	
}

void FMapManagerModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FMapManagerCommands::Get().CommandAction_Develop);
	Builder.AddMenuEntry(FMapManagerCommands::Get().CommandAction_Design);
}

TSharedRef<FUICommandInfo> FMapManagerModule::GetLastExecuteCommand() const
{
	const UMapManagerPlaySettings* PlaySettings = GetDefault<UMapManagerPlaySettings>();

	const FMapManagerCommands& Commands = FMapManagerCommands::Get();
	TSharedRef < FUICommandInfo > Command = Commands.CommandAction_Develop.ToSharedRef();

	switch( PlaySettings->LastPlayMode )
	{
	case EMapManagerLastPlayMode::Develop:		
		Command = Commands.CommandAction_Develop.ToSharedRef();				
		break;

	case EMapManagerLastPlayMode::Design:			
		Command = Commands.CommandAction_Design.ToSharedRef();			
		break;
	};

	return Command;
}

void FMapManagerModule::SetLastExecutedPlayMode(EMapManagerLastPlayMode PlayMode)
{
	UMapManagerPlaySettings* PlaySettings = GetMutableDefault<UMapManagerPlaySettings>();
	PlaySettings->LastPlayMode = PlayMode;
	
	FPropertyChangedEvent PropChangeEvent(UMapManagerPlaySettings::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UMapManagerPlaySettings, LastPlayMode)));
	PlaySettings->PostEditChangeProperty(PropChangeEvent);
	
	PlaySettings->SaveConfig();
}

FText FMapManagerModule::GetLastPlayToolTip()
{
	return GetLastExecuteCommand()->GetDescription();
}

struct FSlateIcon FMapManagerModule::GetLastPlayIcon()
{
	return GetLastExecuteCommand()->GetIcon();
}

TSharedRef<SWidget> FMapManagerModule::GeneratePlayMenuContent(TSharedRef<FUICommandList> InCommandList)
{
	// Get all menu extenders for this context menu from the level editor module
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	FMenuBuilder MenuBuilder( true, InCommandList );
	
	MenuBuilder.AddMenuEntry(FMapManagerCommands::Get().CommandAction_Design);
	MenuBuilder.AddMenuEntry(FMapManagerCommands::Get().CommandAction_Develop);

	return MenuBuilder.MakeWidget();
}

void FMapManagerModule::Event_OnBeginDesignGame(const bool IsSimulating)
{
	if (!BeginDesignGameDelegateHandle.IsValid()) {
		return;
	}
	FEditorDelegates::PostPIEStarted.Remove(BeginDesignGameDelegateHandle);
	BeginDesignGameDelegateHandle.Reset();
    
	UWorld* World = GetPlayWorld();

	if(!World) { return; }
    
	USDGameInstance* GI = Cast<USDGameInstance>(World->GetGameInstance());
	if(!GI) { return; }

	{
		//Create and inject
		UEditableDataTable* NewTable = NewObject<UEditableDataTable>();
		UDataTable* OldTable = GI->GetMainLevelLoadingData();
		NewTable->CreateTableFromOtherTable(OldTable);
		TMap<FName, uint8*>& NewEditableRowMaps = NewTable->GetNonConstRowMap();

		TAssetPtr<UWorld> DesignMap = GetMutableDefault<UMapManagerConfig>()->DesignMap;
		TAssetPtr<UWorld> DevelopMap = GetMutableDefault<UMapManagerConfig>()->DevelopMap;

		FName OldMapName = (* FPaths::GetBaseFilename(DevelopMap.GetLongPackageName()));
		FName NewMapName = (*FPaths::GetBaseFilename(DesignMap.GetLongPackageName()));

		for(auto& r : NewEditableRowMaps)
		{
			FDTLevelLoadingData* row = reinterpret_cast<FDTLevelLoadingData*>(r.Value);
			if(row->PersistentLevelName == OldMapName)
			{
				row->PersistentLevelName = NewMapName;
			}
		}
		
		GI->SetMainLevelLoadingData(NewTable);
	}
	/////////////////////////////////////////////////////////////////////////////////

	/*{
		//Create and inject
		TArray<FMapManagerRowListViewData> MapRows = Manager->GetMapRows();
		UEditableDataTable* NewTable = NewObject<UEditableDataTable>();
		UDataTable* OldTable = GI->GetDTSubLevelGraph();
		NewTable->CreateTableFromOtherTable(OldTable);
		TMap<FName, uint8*>& NewEditableRowMaps = NewTable->GetNonConstRowMap();
	
		for(const auto& room : MapRows)
		{
			if(room.DevelopMap.IsEmpty() || room.DesignMap.IsEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("MapManage: Room info missing. Room id: %s"), *room.RoomId);
				continue;
			}

			uint8** RawRowPtr = NewEditableRowMaps.Find(FName(*room.RoomId));
			if(!RawRowPtr)
			{
				UE_LOG(LogTemp, Warning, TEXT("MapManage: Room not exist. Room id: %s"), *room.RoomId);
				continue;
			}
			FDTSubLevelNode* Row = reinterpret_cast<FDTSubLevelNode*>(*RawRowPtr);
			if(Row->StaticLevelName == FName(*room.DevelopMap))
			{
				Row->StaticLevelName = FName(*room.DesignMap);
			}else
			{
				UE_LOG(LogTemp, Warning, TEXT("MapManage: Room info not match: %s develop level name should be %s but %s"), *room.RoomId, *Row->StaticLevelName.ToString(), *room.DevelopMap);
			}
		}

		GI->SetDTSubLevelGraph(NewTable);
	}*/


	UE_LOG(LogTemp, Warning, TEXT("FMapManagerModule::Event_OnBeginDesignGame no implementation !"));
}

UWorld* FMapManagerModule::GetPlayWorld()
{
	return GEditor->PlayWorld;
}

bool FMapManagerModule::IsGamePlaying()
{
	return GetPlayWorld() != nullptr;
}


void FMapManagerModule::StopPieWrapper(const bool)
{
	if (!PreEndPieDelegateHandler.IsValid()) {
		return;
	}
	//Pie Stopped
	FEditorDelegates::PrePIEEnded.Remove(PreEndPieDelegateHandler);
	PreEndPieDelegateHandler.Reset();

	//CANNOT change map here directly
	//will cause some GC error
	//because the session is not stop yet
	//my solution is delay the call of changing map
	//util the session is totally stopped, and the Play World is cleared
	//so I bind callback to the tick delegate
	EndPieTickDelegateHandler = GEditor->OnPostEditorTick().AddRaw(this, &FMapManagerModule::StopPieExec);
}

void FMapManagerModule::StopPieExec(float)
{
	if(IsGamePlaying())
	{
		return;
	}
	
	if(!EndPieTickDelegateHandler.IsValid()){
		return;
	}

	GEditor->OnPostEditorTick().Remove(EndPieTickDelegateHandler);
	EndPieTickDelegateHandler.Reset();

	
	if(FPaths::GetBaseFilename(EditingMapBeforePlay) == GetMutableDefault<UMapManagerConfig>()->DevelopMap.GetAssetName())
	{
		Loader->Initialize();
		Loader->OpenMap();
		return;
	}
	
	if(EditingMapBeforePlay != PieMap)
	{
		FEditorFileUtils::LoadMap(
         EditingMapBeforePlay,
         false,
         true
         );
	}
	
	FEditorViewportClient* EditorViewClient = GetEditorViewportClient();
		
	if( EditorViewClient )
	{
		EditorViewClient->SetViewLocation(EditingViewLocation);
		EditorViewClient->SetViewRotation(EditingViewRotation);
	}
}

FEditorViewportClient* FMapManagerModule::GetEditorViewportClient() const
{
	FEditorViewportClient* EditorViewClient = nullptr;
	
	FViewport* ActiveViewport = GEditor->GetActiveViewport();
	if(ActiveViewport)
	{
		EditorViewClient = static_cast<FEditorViewportClient*>(ActiveViewport->GetClient());
	}
	
	return EditorViewClient;
}

void FMapManagerModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	//Builder.AddToolBarButton(FMapManagerCommands::Get().CommandAction_Develop);
	//Builder.AddToolBarButton(FMapManagerCommands::Get().CommandAction_Design);

	// Play
	Builder.AddToolBarButton( 
        FMapManagerCommands::Get().CommandAction_Last, 
        NAME_None, 
        LOCTEXT("LastPlay", "Play"),
        TAttribute< FText >::Create( TAttribute< FText >::FGetter::CreateRaw(this, &FMapManagerModule::GetLastPlayToolTip ) ),
        TAttribute< FSlateIcon >::Create( TAttribute< FSlateIcon >::FGetter::CreateRaw( this, &FMapManagerModule::GetLastPlayIcon ) ),
        FName(TEXT("MapManagerLastPlay"))
    );

	// Play combo box
	FUIAction SpecialPIEOptionsMenuAction;
	SpecialPIEOptionsMenuAction.IsActionVisibleDelegate = FIsActionButtonVisible::CreateLambda([](){return true;} );

	Builder.AddComboButton(
        SpecialPIEOptionsMenuAction,
        FOnGetContent::CreateStatic( &GeneratePlayMenuContent, PluginCommands.ToSharedRef() ),
        LOCTEXT( "PlayCombo_Label", "Active Play Mode" ),
        LOCTEXT( "PIEComboToolTip", "Change Play Mode and Play Settings" ),
        FSlateIcon(FMapManagerStyle::GetStyleSetName(), "MapManager.CommandAction_Develop"),
        true
    );

}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMapManagerModule, MapManager)