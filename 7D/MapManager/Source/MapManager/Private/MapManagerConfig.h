#pragma once

#include "CoreMinimal.h"
#include "MapManagerConfig.generated.h"


UCLASS(config = MapManager, defaultconfig)
class MAPMANAGER_API UMapManagerConfig : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category = Villa)
    TAssetPtr<UWorld> DesignMap;
    UPROPERTY(Config, EditAnywhere, Category = Villa)
    TAssetPtr<UWorld> DevelopMap;
    /*UPROPERTY(Config, EditAnywhere, Category = Settings)
    TAssetPtr<class UDataTable> DesignLevelLoadingTable;*/
    UPROPERTY(Config, EditAnywhere, Category = Settings)
    FString DevelopLevelPath;
    UPROPERTY(Config, EditAnywhere, Category = Settings)
    FString DesignLevelPath;
    UPROPERTY(Config, EditAnywhere, Category = Villa)
    TArray<FString> MapsAlwaysLoadInFullName;
    UPROPERTY(Config, EditAnywhere, Category = Villa)
    TArray<FName> VillaFoldersInFullName;
};

UENUM()
enum class EMapManagerLastPlayMode : uint8
{
    Develop,
    Design
};

/**
* Implements the Editor's play settings.
*/
UCLASS(config=EditorPerProjectUserSettings)
class MAPMANAGER_API UMapManagerPlaySettings : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(config)
    EMapManagerLastPlayMode LastPlayMode;
	UPROPERTY(config)
		TArray<FString> LastLoadedMaps;
};