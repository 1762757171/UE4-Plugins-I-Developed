
#pragma once

#include "CoreTypes.h"
#include "EdGraph/EdGraphSchema.h"
#include "RoomRelationshipGraphSchema.generated.h"

UCLASS()
class URoomRelationshipGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()
public:
	/**
	 * Get all actions that can be performed when right clicking on a graph or drag-releasing on a graph from a pin
	 *
	 * @param [in,out]	ContextMenuBuilder	The context (graph, dragged pin, etc...) and output menu builder.
	 */
	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;

	/**
	/**
	* Gets actions that should be added to the right-click context menu for a node or pin
	* 
	* @param	Menu				The menu to append actions to.
	* @param	Context				The menu's context.
	*/
	virtual void GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;

	/** Returns schema action to create comment from implemention */
	virtual TSharedPtr<FEdGraphSchemaAction> GetCreateCommentAction() const override;
	
	/**
	 * Populate new graph with any default nodes
	 *
	 * @param	Graph			Graph to add the default nodes to
	 */
	//virtual void CreateDefaultNodesForGraph(UEdGraph& Graph) const override;

	/** Copies pin links from one pin to another without breaking the original links */
	//virtual FPinConnectionResponse CopyPinLinks(UEdGraphPin& CopyFromPin, UEdGraphPin& CopyToPin, bool bIsIntermediateCopy = false) const override;

	/**
	 * Determine if a connection can be created between two pins.
	 *
	 * @param	PinA	The first pin.
	 * @param	PinB	The second pin.
	 *
	 * @return	An empty string if the connection is legal, otherwise a message describing why the connection would fail.
	 */
	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const override;

	/**
	 * Try to make a connection between two pins.
	 *
	 * @param	PinA	The first pin.
	 * @param	PinB	The second pin.
	 *
	 * @return	True if a connection was made/broken (graph was modified); false if the connection failed and had no side effects.
	 */
	virtual bool TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

	/**
	 * Breaks all links from/to a single node
	 *
	 * @param	TargetNode	The node to break links on
	 */
	//virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;

	/**
	 * Breaks all links from/to a single pin
	 *
	 * @param	TargetPin	The pin to break links on
	 * @param	bSendsNodeNotifcation	whether to send a notification to the node post pin connection change
	 */
	//virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;

	/**
	 * Breaks the link between two nodes.
	 *
	 * @param	SourcePin	The pin where the link begins.
	 * @param	TargetPin	The pin where the link ends.
	 */
	//virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const override;

	//virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;

};
