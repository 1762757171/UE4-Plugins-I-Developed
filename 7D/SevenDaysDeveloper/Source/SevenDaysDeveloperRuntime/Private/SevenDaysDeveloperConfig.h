#pragma once

#include "CoreMinimal.h"
#include "SevenDaysDeveloperConfig.generated.h"


UCLASS(config = SevenDaysDeveloper, defaultconfig)
class SEVENDAYSDEVELOPERRUNTIME_API USevenDaysDeveloperConfig : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(Config, EditAnywhere, Category = Gameplay)
    FVector GameplayMapTransportLocation;
	UPROPERTY(Config, EditAnywhere, Category = Gameplay)
    FVector GobackLocation;
	UPROPERTY(Config, EditAnywhere, Category = Gameplay)
    TMap<FName, FVector> RoomTransportLocation;
};