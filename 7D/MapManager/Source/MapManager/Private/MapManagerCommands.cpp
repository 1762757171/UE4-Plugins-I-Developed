// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MapManagerCommands.h"

#define LOCTEXT_NAMESPACE "FMapManagerModule"

void FMapManagerCommands::RegisterCommands()
{
    UI_COMMAND(CommandAction_Last, "[MapManager] Play", "Play last played mode", EUserInterfaceActionType::Button, FInputGesture());
    UI_COMMAND(CommandAction_Develop, "Player - Develop", "Start game for developer.", EUserInterfaceActionType::Button, FInputGesture());
    UI_COMMAND(CommandAction_Design, "Player - Design", "Start game for design.", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
