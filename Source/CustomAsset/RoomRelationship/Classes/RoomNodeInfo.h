#pragma once
#include "CoreMinimal.h"
#include "RoomNodeInfo.generated.h"

USTRUCT(BlueprintType)
struct FRoomNodeInfo
{
    GENERATED_BODY()

public:
    // I prefer this better than bit-mask enum
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day1 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day2 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day3 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day4 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day5 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day6 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day7_1 : 1;
    UPROPERTY(EditAnywhere, Category = "Available Days")
    uint8 Day7_2 : 1;
    
    UPROPERTY()
    uint8 bIsDoor : 1;

public:

    /////////////////////////////////////////////////////////
    ///            IS THERE ANY BETTER DESIGN?            ///
    /////////////////////////////////////////////////////////
    
    FString GetDescStr() const
    {
        return
        FString::Printf(
            TEXT("%d%d%d%d%d%d%d%d"),
            Day1 ? 1 : 0,
            Day2 ? 2 : 0,
            Day3 ? 3 : 0,
            Day4 ? 4 : 0,
            Day5 ? 5 : 0,
            Day6 ? 6 : 0,
            Day7_1 ? 7 : 0,
            Day7_2 ? 8 : 0
        )
        ;
    }

    bool IsAvailableWhen(uint8 Day) const
    {
        switch (Day)
        {
        case 0:
            return true;
        case 1:
            return Day1;
        case 2:
            return Day2;
        case 3:
            return Day3;
        case 4:
            return Day4;
        case 5:
            return Day5;
        case 6:
            return Day6;
        case 7:
            return Day7_1;
        case 8:
            return Day7_2;
        default:
            return false;
        }
    }
};

