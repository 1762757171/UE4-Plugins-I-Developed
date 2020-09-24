#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

namespace SevenDaysDeveloperRuntimeUtils
{
    static UWorld* GetRuntimePlayWorld()
    {
        const auto& WorldContexts = GEngine->GetWorldContexts();
        for(const auto& wc : WorldContexts)
        {
            UWorld* World = wc.World();
            if(World->bBegunPlay)
            {
                return World;
            }
        }
        return nullptr;
    }
    static bool IsAtRuntime()
    {
        return GetRuntimePlayWorld() != nullptr;
    }
}
