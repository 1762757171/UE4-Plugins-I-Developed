#pragma once

#include "RoomRelationship/Public/GraphEditor/SRoomRelationshipGraphNode.h"
#include "CoreMinimal.h"
#include "SInlineEditableTextBlock.h"
#include "VerticalBox.h"
#include "RoomRelationship/Classes/RoomRelationshipNode.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraph.h"
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphNode.h"

void SRoomRelationshipGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	CachedImg_Pin_Background = FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.Background"));
	CachedImg_Pin_BackgroundHovered = FEditorStyle::GetBrush(TEXT("Graph.StateNode.Pin.BackgroundHovered"));

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(SBorder::FArguments()
        .BorderImage(this, &SRoomRelationshipGraphPin::GetPinBorder)
        .BorderBackgroundColor(this, &SRoomRelationshipGraphPin::GetPinColor)
        .OnMouseButtonDown(this, &SRoomRelationshipGraphPin::OnPinMouseDown)
        .Cursor(this, &SRoomRelationshipGraphPin::GetPinCursor)
    );
}

TSharedRef<SWidget> SRoomRelationshipGraphPin::GetDefaultValueWidget()
{
	return SNew(STextBlock);
}

void SRoomRelationshipGraphNode::Construct(const FArguments& InArgs, URoomRelationshipGraphNode* InNode)
{
    GraphNode = InNode;
	RoomRelationshipGraphNode = InNode;
	
	UpdateGraphNode();
}

void SRoomRelationshipGraphNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	//const FLinearColor TitleShadowColor(0.6f, 0.6f, 0.6f);
	//TSharedPtr<SErrorText> ErrorText;
	NodeTitle = SNew(SNodeTitle, GraphNode);

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
				.BorderImage(this, &SRoomRelationshipGraphNode::GetPinBrush)
				.Padding(0)
				//.BorderBackgroundColor(this, &SGraphNodeAnimState::GetBorderBackgroundColor)
				.BorderBackgroundColor(this, &SRoomRelationshipGraphNode::GetBackgroundColor)
				[
					SNew(SOverlay)
					// PIN AREA
						+ SOverlay::Slot()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SAssignNew(RightNodeBox, SVerticalBox)
							]

						// STATE NAME AREA
						+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(10.0f)
							[
								SNew(SBorder)
									.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.ColorSpill"))
									.Padding(this, &SRoomRelationshipGraphNode::GetMargin)
									//.BorderBackgroundColor(TitleShadowColor)
									.BorderBackgroundColor(this, &SRoomRelationshipGraphNode::GetBackgroundColor)
									.HAlign(HAlign_Center)
									.VAlign(VAlign_Center)
									.Visibility(EVisibility::SelfHitTestInvisible)
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
												.AutoWidth()
												[
													// POPUP ERROR MESSAGE
													SAssignNew(ErrorReporting, SErrorText)
													.BackgroundColor(this, &SRoomRelationshipGraphNode::GetErrorColor)
													.ToolTipText(this, &SRoomRelationshipGraphNode::GetErrorMsgToolTip)
												]
											+ SHorizontalBox::Slot()
												.Padding(FMargin(4.0f, 0.0f, 4.0f, 0.0f))
												[
													SNew(SVerticalBox)
														+ SVerticalBox::Slot()
															.AutoHeight()
															[
																SAssignNew(InlineEditableText, SInlineEditableTextBlock)
																	.Style(FEditorStyle::Get(), "Graph.StateNode.NodeTitleInlineEditableText")
																	.Text(NodeTitle.Get(), &SNodeTitle::GetHeadTitle)
																	.OnVerifyTextChanged(this, &SRoomRelationshipGraphNode::OnVerifyNameTextChanged)
																	.OnTextCommitted(this, &SRoomRelationshipGraphNode::OnNameTextCommited)
																	.IsReadOnly(this, &SRoomRelationshipGraphNode::IsNameReadOnly)
																	.IsSelected(this, &SRoomRelationshipGraphNode::IsSelectedExclusively)
															]
														+ SVerticalBox::Slot()
															.AutoHeight()
															[
																SNew(STextBlock)
																	.Text(this, &SRoomRelationshipGraphNode::GetDescText)
																	.HighlightText(this, &SRoomRelationshipGraphNode::GetEntryDay)
															]
														+ SVerticalBox::Slot()
															.AutoHeight()
															[
																NodeTitle.ToSharedRef()
															]
												]
									]
							]
				]
		];

	//ErrorReporting = ErrorText;
	ErrorReporting->SetError(ErrorMsg);
	
	CreatePinWidgets();
}

void SRoomRelationshipGraphNode::CreatePinWidgets()
{
	UEdGraphPin* CurPin = RoomRelationshipGraphNode->GetPin();
	if (!CurPin->bHidden)
	{
		const TSharedPtr<SGraphPin> NewPin = SNew(SRoomRelationshipGraphPin, CurPin);
		this->AddPin(NewPin.ToSharedRef());
	}
}

void SRoomRelationshipGraphNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));
	RightNodeBox->AddSlot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .FillHeight(1.0f)
        [
            PinToAdd
        ];
	OutputPins.Add(PinToAdd);
}

FSlateColor SRoomRelationshipGraphNode::GetBackgroundColor() const
{
	return
    RoomRelationshipGraphNode->IsRoom() ?
		FLinearColor::White	:
		FLinearColor::Blue	;
}

const FSlateBrush* SRoomRelationshipGraphNode::GetPinBrush() const
{
	return
	RoomRelationshipGraphNode->IsRoom() ?
		FEditorStyle::GetBrush(TEXT("Graph.StateNode.Body")):
		FEditorStyle::GetBrush("Graph.StateNode.ColorSpill") 
	;
}

const FSlateBrush* SRoomRelationshipGraphNode::GetNodeBrush() const
{
	return
    RoomRelationshipGraphNode->IsRoom() ?
        FEditorStyle::GetBrush("Graph.StateNode.Body") :
        FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Body"))
    ;
}

FMargin SRoomRelationshipGraphNode::GetMargin() const
{
	return
    RoomRelationshipGraphNode->IsRoom() ?
        FMargin(0.0f, 15.0f, 0.0f, 15.0f) :
		FMargin(0.0f, 5.0f, 0.0f, 5.0f)
    ;
}

FText SRoomRelationshipGraphNode::GetDescText() const
{
	if(!RoomRelationshipGraphNode->IsRoom())
	{
		return FText::FromString(TEXT("Door"));
	}
	return FText::FromString( RoomRelationshipGraphNode->GetNode()->GetNodeInfo().GetDescStr() );
}

FText SRoomRelationshipGraphNode::GetEntryDay() const
{
	if(!RoomRelationshipGraphNode->IsRoom())
	{
		return FText();
	}
	uint8 Entry = Cast<URoomRelationshipGraph>( RoomRelationshipGraphNode->GetGraph() )->GetRoomRelationshipAssetEdit()->GetIsEntryRoom(RoomRelationshipGraphNode->GetNodeRoomName());
	if(Entry != 0)
	{
		return FText::FromString(FString::FromInt((int32) Entry));
	}
	return FText();
}
