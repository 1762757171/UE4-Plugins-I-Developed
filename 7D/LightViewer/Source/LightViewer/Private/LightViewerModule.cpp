// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "LightViewerModule.h"
#include "EditorViewportClient.h"
#include "IAssetViewport.h"
//#include "KismetWidgets.h"

#include "LevelEditor.h"
#include "SDockTab.h"
#include "SLightViewer.h"
#include "TabManager.h"
#include "UnrealEdGlobals.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Components/CapsuleComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/LevelStreamingAlwaysLoaded.h"

#define LOCTEXT_NAMESPACE "FLightViewerModule"

static const FName LightViewerModuleTabName("FLightViewerModule");

FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors LevelViewportContextMenuBlueprintExtender;

void FLightViewerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	// Add Tool interface into Development Tools menu
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LightViewerModuleTabName, FOnSpawnTab::CreateRaw(this, &FLightViewerModule::OnSpawnPluginTab))
        .SetDisplayName(LOCTEXT("TabTitle", "Light Viewer"))
        .SetTooltipText(LOCTEXT("TooltipText", "Open the Light viewer tab."))
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
        .SetIcon(FSlateIcon());
	
	/*if (GIsEditor)
	{
		// Extend the level viewport context menu to handle blueprints
		LevelViewportContextMenuBlueprintExtender = FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateLambda(
			[](const TSharedRef<FUICommandList> CommandList, TArray<AActor*> SelectedActors)
	        {
	            TSharedPtr<FExtender> Extender = MakeShareable(new FExtender);

	            Extender->AddMenuExtension("LevelViewportEdit", EExtensionHook::Before, CommandList,
	                FMenuExtensionDelegate::CreateLambda(
						[&](FMenuBuilder& MenuBuilder)
						{
							
						}
	                ));

	            return Extender.ToSharedRef();
	        }
		);
		
		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
		MenuExtenders.Add(LevelViewportContextMenuBlueprintExtender);
		LevelViewportContextMenuBlueprintExtenderDelegateHandle = MenuExtenders.Last().GetHandle();

		FModuleManager::Get().LoadModuleChecked<FKismetWidgetsModule>("KismetWidgets");
	}*/
    
}

void FLightViewerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LightViewerModuleTabName);
	
	// Remove level viewport context menu extenders
	/*if ( FModuleManager::Get().IsModuleLoaded( "LevelEditor" ) )
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders().RemoveAll([&](const FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors& Delegate) {
            return Delegate.GetHandle() == LevelViewportContextMenuBlueprintExtenderDelegateHandle;
        });
	}*/
}

TSharedRef<SDockTab> FLightViewerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab);

	TSharedPtr<SWidget> TabContent =
        SNew(SLightViewer, Tab, SpawnTabArgs.GetOwnerWindow())
    ;

	Tab->SetContent(TabContent.ToSharedRef());

	return Tab;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLightViewerModule, LightViewer)