// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CustomAssetEditor/Module/Public/CustomAssetEditorModule.h"
#include "AssetToolsModule.h"
#include "EdGraphUtilities.h"
#include "IAssetTools.h"
#include "CustomAssetEditor/RoomRelationship/Public/RoomRelationshipAssetTypeAction.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphFactory.h"

#define LOCTEXT_NAMESPACE "CustomAssetEditorModule"

void FCustomAssetEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	IAssetTools& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedPtr<FRoomRelationshipAssetTypeAction> RoomRelationshipAssetTypeAction = MakeShareable(new FRoomRelationshipAssetTypeAction);
	AssetToolsModule.RegisterAssetTypeActions(RoomRelationshipAssetTypeAction.ToSharedRef());

	AddFactory<FRoomRelationshipGraphNodeFactory, FRoomRelationshipGraphPinFactory, FRoomRelationshipGraphPinConnectionFactory>();


	RegisterGraphFactories();
}

void FCustomAssetEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UnregisterGraphFactories();
}

void FCustomAssetEditorModule::RegisterGraphFactories()
{
	for(const auto& f : GraphFactorieses)
	{
		FEdGraphUtilities::RegisterVisualNodeFactory(f.GraphNodeFactory);
		FEdGraphUtilities::RegisterVisualPinFactory(f.GraphPinFactory);
		FEdGraphUtilities::RegisterVisualPinConnectionFactory(f.GraphConnectionFactory);
	}
}

void FCustomAssetEditorModule::UnregisterGraphFactories()
{
	for(const auto& f : GraphFactorieses)
	{
		FEdGraphUtilities::UnregisterVisualNodeFactory(f.GraphNodeFactory);
		FEdGraphUtilities::UnregisterVisualPinFactory(f.GraphPinFactory);
		FEdGraphUtilities::UnregisterVisualPinConnectionFactory(f.GraphConnectionFactory);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCustomAssetEditorModule, CustomAssetEditor)