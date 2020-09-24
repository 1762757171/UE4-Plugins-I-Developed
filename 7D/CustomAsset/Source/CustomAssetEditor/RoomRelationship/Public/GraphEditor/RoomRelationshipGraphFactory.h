// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphNode.h"

struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphNodeFactory : public FGraphPanelNodeFactory
{
public:
    virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override;
};

struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphPinFactory : public FGraphPanelPinFactory
{
public:
    virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* Pin) const override;
};

struct CUSTOMASSETEDITOR_API FRoomRelationshipGraphPinConnectionFactory : public FGraphPanelPinConnectionFactory
{
public:
    virtual class FConnectionDrawingPolicy* CreateConnectionPolicy(const class UEdGraphSchema* Schema, int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const class FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
};