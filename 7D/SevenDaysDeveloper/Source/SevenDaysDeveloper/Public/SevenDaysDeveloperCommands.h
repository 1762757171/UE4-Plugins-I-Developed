// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SevenDaysDeveloperStyle.h"

class FSevenDaysDeveloperCommands : public TCommands<FSevenDaysDeveloperCommands>
{
public:

	FSevenDaysDeveloperCommands()
		: TCommands<FSevenDaysDeveloperCommands>(TEXT("SevenDaysDeveloper"), NSLOCTEXT("Contexts", "SevenDaysDeveloper", "SevenDaysDeveloper Plugin"), NAME_None, FSevenDaysDeveloperStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	//TSharedPtr< FUICommandInfo > OpenPluginWindow;
};