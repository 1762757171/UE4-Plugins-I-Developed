#pragma once
#include "AssetEditorToolkit.h"
#include "Commands.h"

class FRoomRelationsAssetCommands : public TCommands<FRoomRelationsAssetCommands>
{
public:
    FRoomRelationsAssetCommands();

    //TSharedPtr<FUICommandInfo> SetAsEntry;

    /** Initialize commands */
    virtual void RegisterCommands() override;
};
