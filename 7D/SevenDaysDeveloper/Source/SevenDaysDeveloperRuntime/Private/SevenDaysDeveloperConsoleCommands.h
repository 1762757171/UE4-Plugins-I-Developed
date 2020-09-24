
#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"
#include "SevenDaysDeveloperRuntime/Public/SevenDaysDeveloperRuntimeModule.h"

#define LOCTEXT_NAMESPACE "FSevenDaysDeveloperModule"

class FSevenDaysDeveloperConsoleCommands
{
public:
	FSevenDaysDeveloperRuntimeModule& Module;
	
	FText LoadHelp;
	FText TransportHelp;
	FText SpeedHelp;
	FText ResetSpeedHelp;
	FText GameplayTestHelp;
	
	FAutoConsoleCommand LoadCommand;
	FAutoConsoleCommand TransportCommand;
	FAutoConsoleCommand SpeedCommand;
	FAutoConsoleCommand ResetSpeedCommand;
	FAutoConsoleCommand GameplayTestCommand;

	
	FSevenDaysDeveloperConsoleCommands(FSevenDaysDeveloperRuntimeModule& InModule)
        : Module(InModule)
	,	LoadHelp(LOCTEXT("CommandText_Load", "Load specific room. USAGE: \'SD.Load 2008 2009 1017\'"))
	,	TransportHelp(LOCTEXT("CommandText_Transport", "Transport to specific room. USAGE: \'SD.Tp 2008\'"))
	,	SpeedHelp(LOCTEXT("CommandText_Speed", "Modify speed of player. USAGE: \'SD.Speed 500\' |or| \'SD.Speed -1\' to disable modification (another way: \'SD.ResetSpeed\')."))
	,	ResetSpeedHelp(LOCTEXT("CommandText_ResetSpeed", "Cancel speed modification of player. USAGE: \'SD.ResetSpeed\', same as \'SD.Speed -1\'"))
	,	GameplayTestHelp(LOCTEXT("CommandText_GameplayTest", "Teleport to gameplay test map. USAGE: \'SD.GmpTst 1\' to test |or| \'SD.Gmptst\' 0 to get back to main game map."))
    ,	LoadCommand(
        TEXT( "SD.Load" ),
        *LoadHelp.ToString(),
        FConsoleCommandWithArgsDelegate::CreateRaw( this, &FSevenDaysDeveloperConsoleCommands::Load ) )
    ,	TransportCommand(
        TEXT( "SD.Tp" ),
        *TransportHelp.ToString(),
        FConsoleCommandWithArgsDelegate::CreateRaw( this, &FSevenDaysDeveloperConsoleCommands::Transport ) )
    ,	SpeedCommand(
        TEXT( "SD.Speed" ),
        *SpeedHelp.ToString(),
        FConsoleCommandWithArgsDelegate::CreateRaw( this, &FSevenDaysDeveloperConsoleCommands::Speed ) )
	,	ResetSpeedCommand(
		TEXT( "SD.ResetSpeed" ),
		*ResetSpeedHelp.ToString(),
        FConsoleCommandWithArgsDelegate::CreateRaw( this, &FSevenDaysDeveloperConsoleCommands::ResetSpeed ) )
    ,	GameplayTestCommand(
        TEXT( "SD.Gameplay" ),
        *GameplayTestHelp.ToString(),
        FConsoleCommandWithArgsDelegate::CreateRaw( this, &FSevenDaysDeveloperConsoleCommands::GameplayTest ) )
	{}

	void Load(const TArray<FString>& Args)
	{
		if ( Args.Num() == 0 )
		{
			TCHAR Help[256];
			FCString::Strcpy(Help, *LoadHelp.ToString());
			UE_LOG(LogTemp, Warning, Help);
			return;
		}
		TArray<FName> RoomNames;
		for(const auto& a : Args)
		{
			RoomNames.Add(FName(*a));
		}
		Module.LoadLevel(RoomNames);
	}

	void Transport(const TArray<FString>& Args)
	{
		if (Args.Num() != 1)
		{
			TCHAR Help[256];
			FCString::Strcpy(Help, *TransportHelp.ToString());
			UE_LOG(LogTemp, Warning, Help);
			return;
		}
		FName RoomName(*Args[0]);
		
		Module.TeleportLevel(RoomName);
	}
	
	void Speed(const TArray<FString>& Args)
	{
		if (Args.Num() != 1)
		{
			TCHAR Help[256];
			FCString::Strcpy(Help, *TransportHelp.ToString());
			UE_LOG(LogTemp, Warning, Help);
			return;
		}
		float Speed = FCString::Atof(*Args[0]);
		if(Speed == 0.0f)
		{
			TCHAR Help[256];
			FCString::Strcpy(Help, *TransportHelp.ToString());
			UE_LOG(LogTemp, Warning, Help);
			return;
		}
		
		Module.ChangeSpeed(Speed);
	}

	void ResetSpeed(const TArray<FString>& Args)
	{
		Module.ChangeSpeed(-1);
	}
	
	void GameplayTest(const TArray<FString>& Args)
	{
		if (Args.Num() != 1)
		{
			TCHAR Help[256];
			FCString::Strcpy(Help, *TransportHelp.ToString());
			UE_LOG(LogTemp, Warning, Help);
			return;
		}
		int32 Flag = FCString::Atoi(*Args[0]);
		bool bGoTest = (Flag != 0);
		
		Module.GameplayTest(bGoTest);
	}
	
};

#undef  LOCTEXT_NAMESPACE