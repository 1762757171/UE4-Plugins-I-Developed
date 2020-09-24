// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SevenDaysDeveloperModule.h"

#include "ISettingsModule.h"
#include "SevenDaysDeveloperStyle.h"
#include "SevenDaysDeveloperCommands.h"
#include "SSevenDaysDeveloper.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName SevenDaysDeveloperTabName("SevenDaysDeveloper");

#define LOCTEXT_NAMESPACE "FSevenDaysDeveloperModule"

void FSevenDaysDeveloperModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSevenDaysDeveloperStyle::Initialize();
	FSevenDaysDeveloperStyle::ReloadTextures();

	FSevenDaysDeveloperCommands::Register();
	
	// Add Tool interface into Development Tools menu
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SevenDaysDeveloperTabName, FOnSpawnTab::CreateRaw(this, &FSevenDaysDeveloperModule::OnSpawnPluginTab))
        .SetDisplayName(NSLOCTEXT("FSevenDaysDeveloperModule", "TabTitle", "SevenDays Developer"))
        .SetTooltipText(NSLOCTEXT("FSevenDaysDeveloperModule", "TooltipText", "Open the SevenDays Developer tab."))
        .SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory())
        .SetIcon(FSlateIcon(FSevenDaysDeveloperStyle::GetStyleSetName(), "SevenDaysDeveloper.DeveloperModule"));
	
}

void FSevenDaysDeveloperModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FSevenDaysDeveloperStyle::Shutdown();

	FSevenDaysDeveloperCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SevenDaysDeveloperTabName);
}

TSharedRef<SDockTab> FSevenDaysDeveloperModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> Tab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab);

	TSharedPtr<SWidget> TabContent =
        SNew(SSevenDaysDeveloper, Tab, SpawnTabArgs.GetOwnerWindow())
    ;

	Tab->SetContent(TabContent.ToSharedRef());

	return Tab;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSevenDaysDeveloperModule, SevenDaysDeveloper)