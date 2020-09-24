// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

struct FGraphFactories
{
	TSharedPtr<struct FGraphPanelNodeFactory> GraphNodeFactory;
	TSharedPtr<struct FGraphPanelPinFactory> GraphPinFactory;
	TSharedPtr<struct FGraphPanelPinConnectionFactory> GraphConnectionFactory;

};


class FCustomAssetEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	template<typename NodeFactory, typename PinFactory, typename PinConnectionFactory>
	void AddFactory()
	{
		FGraphFactories Factory;
		// Create factories
		Factory.GraphNodeFactory = MakeShared<NodeFactory>();
		Factory.GraphPinFactory = MakeShared<PinFactory>();
		Factory.GraphConnectionFactory = MakeShared<PinConnectionFactory>();
		GraphFactorieses.Add(Factory);
	}

	void RegisterGraphFactories();
	void UnregisterGraphFactories();
private:
	TArray<FGraphFactories> GraphFactorieses;
};
