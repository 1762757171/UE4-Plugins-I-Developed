
#pragma once
#include "Engine/DeveloperSettings.h"
#include "LightViewerConfig.generated.h"

#define LOCTEXT_NAMESPACE "FLightViewerModule"

UCLASS(config = SD, defaultconfig, notplaceable, meta = (DisplayName = "Light Viewer"))
class ULightViewerConfig : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    /** Gets the settings container name for the settings, either Project or Editor */
    virtual FName GetCategoryName() const override { return TEXT("SD"); }
    
#if WITH_EDITOR
    virtual FText GetSectionText() const override { return LOCTEXT("ConfigSection", "Light Viewer"); }
    virtual FText GetSectionDescription() const override { return LOCTEXT("ConfigSectionDesc", "Inject level when playing."); }
#endif
public:
    UPROPERTY(config, EditAnywhere, Category = "Light Viewer")
    TArray<TAssetPtr<class UWorld>> AdditionalWorlds;
};

#undef LOCTEXT_NAMESPACE
