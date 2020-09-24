// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class SEVENDAYSDEVELOPERRUNTIME_API FSevenDaysDeveloperRuntimeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
public:
	void LoadLevel(const TArray<FName>& RoomId);
	void TeleportLevel(const FName& RoomId);
	/* -1 to disable */
	void ChangeSpeed(float NewSpeed);
	float GetSpeed() const;
	float GetLastCachedSpeed() const;

	void GameplayTest(bool bGoTest);
private:
	void Tick_ApplySpeedModify();
	void Teleport_Internal(const FVector& TargetLocation);
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class FSevenDaysDeveloperConsoleCommands> ConsoleCommands;


	FTimerHandle SpeedTimerHandle;
	float SpeedWanted = -1.f;
	float OriginSpeed = -1.f;
	float SpeedCache = 500.0f;
};
