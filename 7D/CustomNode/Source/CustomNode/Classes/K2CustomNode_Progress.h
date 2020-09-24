
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Textures/SlateIcon.h"
#include "K2Node.h"
#include "K2Node_AddPinInterface.h"
#include "EdGraph/EdGraphPin.h"

#include "K2CustomNode_Progress.generated.h"

UCLASS(MinimalAPI, meta = (Keywords = "progress"))
class UK2CustomNode_Progress : public UK2Node, public IK2Node_AddPinInterface
{
	GENERATED_BODY()

public:
	UK2CustomNode_Progress(const FObjectInitializer& ObjectInitializer = FObjectInitializer());

	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual void ReallocatePinsDuringReconstruction(::TArray<UEdGraphPin*>& OldPins) override;
	//virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	//~ Begin UK2Node Interface
	//virtual ERedirectType DoPinsMatchForReconstruction(const UEdGraphPin* NewPin, int32 NewPinIndex, const UEdGraphPin* OldPin, int32 OldPinIndex) const override;
	//virtual class FNodeHandlingFunctor* CreateNodeHandler(class FKismetCompilerContext& CompilerContext) const override;
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetMenuCategory() const override;
	virtual bool CanEverInsertExecutionPin() const override { return true; }
	virtual bool CanEverRemoveExecutionPin() const override { return true; }
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	//~ End UK2Node Interface

	// IK2Node_AddPinInterface interface
	virtual void AddInputPin() override;
	virtual bool CanAddPin() const override;
	// End of IK2Node_AddPinInterface interface

	//~ Begin K2Node_ExecutionSequence Interface

	/** Gets a unique pin name, the next in the sequence */
	FName GetUniquePinName();

	/**
	 * Inserts a new execution pin, before the specified execution pin, into an execution node
	 *
	 * @param	PinToInsertBefore	The pin to insert a new pin before on the node
	 */
	//void InsertPinIntoExecutionNode(UEdGraphPin* PinToInsertBefore, EPinInsertPosition Position);

	void RemovePinFromProgressNode(UEdGraphPin* TargetPin);
	bool CanRemovePin(const UEdGraphPin* Pin) const;

	/** Whether an execution pin can be removed from the node or not */
	//bool CanRemoveExecutionPin() const;

	UFUNCTION()
	static int32 LeftShiftPinMask(int32 PinIndex);

private:
	// Returns the exec output pin name for a given 0-based index
	FName GetPinNameGivenIndex(int32 Index) const;
};

