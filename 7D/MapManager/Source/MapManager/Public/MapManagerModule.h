// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetEditorToolkit.h"
#include "MapManagerConfig.h"
#include "Modules/ModuleManager.h"


class FMapManagerModule : public IModuleInterface
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
	
private:
    TSharedRef<class SDockTab> OnSpawnManagerTab(const class FSpawnTabArgs& SpawnTabArgs);
    TSharedRef<class SDockTab> OnSpawnLoaderTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
    TSharedPtr<class FMapManager> Manager;
    TSharedPtr<class FMapLoader> Loader;
public:
    /** These function will be bound to Command. */
    void PlayLastCommand();
    bool LastCommandCanExecute() const;
    void DevelopButtonClicked();
    bool DevelopButtonCanExecute() const;
    void DesignButtonClicked();
    bool DesignButtonCanExecute() const;
    
private:
    
    void GoStartupMapAndPlay();
    void GoMapAndPlay(const FString& MapLongPkgName);
	
private:

    void AddToolbarExtension(FToolBarBuilder& Builder);
    void AddMenuExtension(FMenuBuilder& Builder);

    TSharedRef<FUICommandInfo> GetLastExecuteCommand() const;
    void SetLastExecutedPlayMode(EMapManagerLastPlayMode PlayMode);
    FText GetLastPlayToolTip();
    FSlateIcon GetLastPlayIcon();
    static TSharedRef< SWidget > GeneratePlayMenuContent( TSharedRef<FUICommandList> InCommandList );

    void Event_OnBeginDesignGame(const bool IsSimulating);

public:
    static UWorld* GetPlayWorld();
    static bool IsGamePlaying();
private:
    TSharedPtr<class FUICommandList> PluginCommands;
    FDelegateHandle PreEndPieDelegateHandler;
    FDelegateHandle EndPieTickDelegateHandler;
    FString EditingMapBeforePlay;
    FString PieMap;
    FVector EditingViewLocation;
    FRotator EditingViewRotation;
    FDelegateHandle BeginDesignGameDelegateHandle;
public:
    void StopPieWrapper(const bool);
    void StopPieExec(float);

private:
    FEditorViewportClient* GetEditorViewportClient() const;
};
