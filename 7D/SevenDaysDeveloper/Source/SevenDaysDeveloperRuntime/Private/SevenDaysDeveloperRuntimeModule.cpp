// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SevenDaysDeveloperRuntime/Public/SevenDaysDeveloperRuntimeModule.h"

#include "RoomSchedulerSubsystem.h"
#include "Door.h"
#include "EngineUtils.h"
#include "ISettingsModule.h"
#include "MessageDialog.h"
#include "SDBlueprintFunctionLibrary.h"
#include "SevenDaysDeveloperConfig.h"
#include "SevenDaysDeveloperConsoleCommands.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SevenDaysDeveloperRuntimeUtils.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "FSevenDaysDeveloperRuntimeModule"

void FSevenDaysDeveloperRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	ConsoleCommands = MakeShareable(new FSevenDaysDeveloperConsoleCommands(*this));
	
	// Register custom config into project settings
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterSettings("Project", "SD", "7DaysDeveloper",
        LOCTEXT("RuntimeSettingsName", "7 Days Developer"),
        LOCTEXT("RuntimeSettingsDescription", "7 Days Developer Config"),
        GetMutableDefault<USevenDaysDeveloperConfig>()
    );
}

void FSevenDaysDeveloperRuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	ConsoleCommands.Reset();
}

void FSevenDaysDeveloperRuntimeModule::LoadLevel(const TArray<FName>& RoomId)
{
	UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
	if(!PlayWorld)
	{
		return;
	}
	URoomSchedulerSubsystem* Scheduler = PlayWorld->GetSubsystem<URoomSchedulerSubsystem>();
	if(Scheduler)
	{
		for(const FName& r : RoomId)
		{
			Scheduler->LoadRelatedRooms(r);
		}
	}
}

void FSevenDaysDeveloperRuntimeModule::TeleportLevel(const FName& RoomId)
{
	UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
    if(!PlayWorld)
    {
        return;
    }

	//LoadLevel({RoomId});
    
    FVector TransportLocation = FVector::ZeroVector;

    const auto& RoomLocationMap = GetMutableDefault<USevenDaysDeveloperConfig>()->RoomTransportLocation;
    const auto& LocationData = RoomLocationMap.Find(RoomId);
    if(LocationData)
    {
        //Use data in config first
        TransportLocation = *LocationData;
    }else
    {
        //if no config data set, iterate all doors

        ADoor* Door = nullptr;
        bool bFromThis = false;

        for(TActorIterator<ADoor> ActorIterator(PlayWorld); ActorIterator; ++ActorIterator)
        {
            ADoor* ThisDoor = *ActorIterator;
            FName DoorId = ThisDoor->GetPropsID();
            FString DoorIdStr = DoorId.ToString();
            if(DoorIdStr.Contains(RoomId.ToString()))
            {
                Door = ThisDoor;
                bFromThis = DoorIdStr.StartsWith(FString("5_") + RoomId.ToString());
                break;
            }
            
        }

        if(!Door)
        {
            FText Hint = FText::Format(LOCTEXT("Prompt_NoRoomInfo",
                    "Cannot find transport info of room [{0}], please write into config manually."),  FText::FromName( RoomId ) );
            
            FMessageDialog::Open(
                EAppMsgType::Ok,
                Hint
            );
            return;
        }

        UArrowComponent* ArrowComponent = Door->FindComponentByClass<UArrowComponent>();
        if(!ArrowComponent)
        {
            FMessageDialog::Open(
                EAppMsgType::Ok,
                LOCTEXT("Prompt_WhatHappened","Error! Contact LYX.")
            );
            return;
        }
        FVector ArrowDir = ArrowComponent->GetForwardVector() * ( bFromThis ? -200.f : 200.f);

		static const FVector ZOffset(0, 0, 100);
        TransportLocation = Door->GetActorLocation() + ZOffset + ArrowDir;
        
    }
    
    {
		//Save to Config
		USevenDaysDeveloperConfig* Config = GetMutableDefault<USevenDaysDeveloperConfig>();
		Config->RoomTransportLocation.Add(RoomId, TransportLocation);

		FPropertyChangedEvent PropChangeEvent(USevenDaysDeveloperConfig::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(USevenDaysDeveloperConfig, RoomTransportLocation)));
#if WITH_EDITOR
		Config->PostEditChangeProperty(PropChangeEvent);
#endif

		Config->SaveConfig();
     }

	Teleport_Internal(TransportLocation);
	
	// USDBlueprintFunctionLibrary::GetSDStatisticsSubsystem(PlayWorld)->UpdateCurrentRoomID(RoomId);

}

void FSevenDaysDeveloperRuntimeModule::ChangeSpeed(float NewSpeed)
{
	UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
	if(!PlayWorld) {return;}
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(PlayWorld, 0);
	if(!PlayerCharacter) {return;}
	UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement();
	
	if(SpeedWanted == -1.0f)
	{
		OriginSpeed = Movement->MaxWalkSpeed;
	}
	
	SpeedCache = NewSpeed == -1.0f ? SpeedCache : NewSpeed;
	SpeedWanted = NewSpeed;
	auto& TimerManager = PlayWorld->GetTimerManager();
	if(!SpeedTimerHandle.IsValid() || !TimerManager.TimerExists(SpeedTimerHandle))
	{
		auto TimerDelegate = FTimerDelegate::CreateRaw(this, &FSevenDaysDeveloperRuntimeModule::Tick_ApplySpeedModify);
		TimerManager.SetTimer(SpeedTimerHandle, TimerDelegate, 0.1f, true);
	}
}

float FSevenDaysDeveloperRuntimeModule::GetSpeed() const
{
	return SpeedWanted;
}

float FSevenDaysDeveloperRuntimeModule::GetLastCachedSpeed() const
{
	return SpeedCache;
}

void FSevenDaysDeveloperRuntimeModule::GameplayTest(bool bGoTest)
{
	FVector NewLocation;
	if(bGoTest)
	{
		NewLocation = GetMutableDefault<USevenDaysDeveloperConfig>()->GameplayMapTransportLocation;
	}
	else
	{
		NewLocation = GetMutableDefault<USevenDaysDeveloperConfig>()->GobackLocation;
	}
	
	Teleport_Internal(NewLocation);
}

void FSevenDaysDeveloperRuntimeModule::Tick_ApplySpeedModify()
{
	UWorld* PlayWorld = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
	if(!PlayWorld) {return;}
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(PlayWorld, 0);
	if(!PlayerCharacter) {return;}
	UCharacterMovementComponent* Movement = PlayerCharacter->GetCharacterMovement();

	if(SpeedWanted != -1.0f)
	{
		if(Movement->GetMaxSpeed() != SpeedWanted)
		{
			Movement->MaxWalkSpeed = SpeedWanted;
		}
	}else
	{
		if(OriginSpeed != -1.0f)
		{
			Movement->MaxWalkSpeed = OriginSpeed;
			OriginSpeed = -1.0f;
		}
	}
}

void FSevenDaysDeveloperRuntimeModule::Teleport_Internal(const FVector& TargetLocation)
{
	UWorld* World = SevenDaysDeveloperRuntimeUtils::GetRuntimePlayWorld();
	if(World)
	{
		ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(World, 0);
		if(PlayerCharacter)
		{
			PlayerCharacter->SetActorLocation(TargetLocation);
		}
	}
}

/*
void FSevenDaysDebuggerModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(SevenDaysDebuggerTabName);
}

void FSevenDaysDebuggerModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FSevenDaysDebuggerCommands::Get().OpenPluginWindow);
}

void FSevenDaysDebuggerModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FSevenDaysDebuggerCommands::Get().OpenPluginWindow);
}
*/

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSevenDaysDeveloperRuntimeModule, SevenDaysDeveloperRuntime)