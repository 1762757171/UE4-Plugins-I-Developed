
#include "RoomRelationship/Public/RoomRelationshipAssetCommand.h"

#define LOCTEXT_NAMESPACE "RoomRelationshipAsset"

FRoomRelationsAssetCommands::FRoomRelationsAssetCommands():
	TCommands<FRoomRelationsAssetCommands>("RoomRelationsAssetEditor.Command",
		LOCTEXT("Command", "Command"),
		NAME_None,
		FEditorStyle::GetStyleSetName())
{}

void FRoomRelationsAssetCommands::RegisterCommands()
{
	//UI_COMMAND(SetAsEntry, "SetAsEntry", "Set this room as entry.", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
