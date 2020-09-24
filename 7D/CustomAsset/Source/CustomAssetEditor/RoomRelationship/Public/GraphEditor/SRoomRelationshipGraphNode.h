#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SGraphPin.h"

class SRoomRelationshipGraphPin : public SGraphPin
{
    public:
    SLATE_BEGIN_ARGS(SRoomRelationshipGraphPin) {}
    SLATE_END_ARGS()

public:
    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

    protected:
    //~ Begin SGraphPin Interface
    virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;
    //~ End SGraphPin Interface

};

class SRoomRelationshipGraphNode : public SGraphNode
{
public:
    SLATE_BEGIN_ARGS(SRoomRelationshipGraphNode) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, class URoomRelationshipGraphNode* InNode);

    // SGraphNode interface
    virtual void UpdateGraphNode() override;
    virtual void CreatePinWidgets() override;
    virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
    //virtual TSharedPtr<SToolTip> GetComplexTooltip() override;
    // End of SGraphNode interface

private:

    FSlateColor GetBackgroundColor() const;
    const FSlateBrush* GetPinBrush() const;
    const FSlateBrush* GetNodeBrush() const;
    FMargin GetMargin() const;
    FText GetDescText() const;
    FText GetEntryDay() const;
private:
    class URoomRelationshipGraphNode* RoomRelationshipGraphNode = nullptr;
	TSharedPtr<SNodeTitle> NodeTitle;
};
