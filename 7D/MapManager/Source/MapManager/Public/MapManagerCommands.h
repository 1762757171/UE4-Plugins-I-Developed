// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MapManagerStyle.h"
#include "UICommandList.h"
#include "Settings/LevelEditorPlaySettings.h"

class FMapManagerCommands : public TCommands<FMapManagerCommands>
{
public:

    FMapManagerCommands()
        : TCommands<FMapManagerCommands>(TEXT("MapManager"), NSLOCTEXT("Contexts", "MapManager", "MapManager Plugin"), NAME_None, FMapManagerStyle::GetStyleSetName())
    {
    }

    // TCommands<> interface
    virtual void RegisterCommands() override;

public:
    TSharedPtr< FUICommandInfo > CommandAction_Last;
    TSharedPtr< FUICommandInfo > CommandAction_Develop;
    TSharedPtr< FUICommandInfo > CommandAction_Design;
};
