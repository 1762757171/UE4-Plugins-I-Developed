
#include "RoomRelationship/Public/GraphEditor/RoomRelationshipGraphConnectionDrawingPolicy.h"

FRoomRelationshipGraphConnectionDrawingPolicy::
FRoomRelationshipGraphConnectionDrawingPolicy(int32 InBackLayerID,
    int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements,
    UEdGraph* InGraphObj)
    : FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
{
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
    FConnectionParams& Params)
{
    Params.AssociatedPin1 = OutputPin;
    Params.AssociatedPin2 = InputPin;
    Params.WireThickness = 1.5f;

    Params.WireColor = GetTransitionColor(HoveredPins.Contains(InputPin));

    const bool bDeemphasizeUnhoveredPins = HoveredPins.Num() > 0;
    if (bDeemphasizeUnhoveredPins)
    {
        ApplyHoverDeemphasis(OutputPin, InputPin, Params.WireThickness, Params.WireColor);
    }
}

void FRoomRelationshipGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries,
    FArrangedChildren& ArrangedNodes)
{
    // Now draw
    FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DetermineLinkGeometry(FArrangedChildren& ArrangedNodes,
    TSharedRef<SWidget>& OutputPinWidget, UEdGraphPin* OutputPin, UEdGraphPin* InputPin,
    FArrangedWidget*& StartWidgetGeometry, FArrangedWidget*& EndWidgetGeometry)
{
    StartWidgetGeometry = PinGeometries->Find(OutputPinWidget);
    
    TSharedPtr<SGraphPin>* pTargetWidget = PinToPinWidgetMap.Find(InputPin);
    if (pTargetWidget)
    {
        const TSharedPtr<SGraphPin> InputWidget = *pTargetWidget;
        EndWidgetGeometry = PinGeometries->Find(InputWidget.ToSharedRef());
    }
    
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom,
    const FGeometry& EndGeom, const FConnectionParams& Params)
{
    // Get a reasonable seed point (halfway between the boxes)
    const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
    const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
    const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

    // Find the (approximate) closest points between the two boxes
    const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
    const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

    DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
    //DrawSplineWithArrow(EndAnchorPoint, StartAnchorPoint, Params);
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartPoint,
    const FVector2D& EndPoint, const FConnectionParams& Params)
{
	DrawLineWithArrowImpl(StartPoint, EndPoint, Params);
	//DrawLineWithArrowImpl(EndPoint, StartPoint, Params);
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry,
    const FVector2D& StartPoint, const FVector2D& EndPoint, UEdGraphPin* Pin)
{
    FConnectionParams Params;
    DetermineWiringStyle(Pin, Pin, /*inout*/ Params);

    const FVector2D SeedPoint = EndPoint;
    const FVector2D AdjustedStartPoint = FGeometryHelper::FindClosestPointOnGeom(PinGeometry, SeedPoint);

    DrawSplineWithArrow(AdjustedStartPoint, EndPoint, Params);
}

FVector2D FRoomRelationshipGraphConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start,
    const FVector2D& End) const
{
    const FVector2D Delta = End - Start;
    const FVector2D NormDelta = Delta.GetSafeNormal();

    return NormDelta;
}

void FRoomRelationshipGraphConnectionDrawingPolicy::DrawLineWithArrowImpl(const FVector2D& StartAnchorPoint,
                                                                          const FVector2D& EndAnchorPoint, const FConnectionParams& Params)
{
    //@TODO: Should this be scaled by zoom factor?
    const float LineSeparationAmount = 0.0f;//4.5f;

    const FVector2D DeltaPos = EndAnchorPoint - StartAnchorPoint;
    const FVector2D UnitDelta = DeltaPos.GetSafeNormal();
    const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

    // Come up with the final start/end points
    const FVector2D DirectionBias = Normal * LineSeparationAmount;
    const FVector2D LengthBias = ArrowRadius.X * UnitDelta;
    const FVector2D StartPoint = StartAnchorPoint + DirectionBias + LengthBias;
    const FVector2D EndPoint = EndAnchorPoint + DirectionBias - LengthBias;

    // Draw a line/spline
    DrawConnection(WireLayerID, StartPoint, EndPoint, Params);

    // Draw the arrow
    const FVector2D StartArrowDrawPos = StartPoint - ArrowRadius;
    const FVector2D EndArrowDrawPos = EndPoint - ArrowRadius;
    const float StartAngleInRadians = FMath::Atan2(-DeltaPos.Y, -DeltaPos.X);
    const float EndAngleInRadians = FMath::Atan2(DeltaPos.Y, DeltaPos.X);

    FSlateDrawElement::MakeRotatedBox(
        DrawElementsList,
        ArrowLayerID,
        FPaintGeometry(StartArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
        ArrowImage,
        ESlateDrawEffect::None,
        StartAngleInRadians,
        TOptional<FVector2D>(),
        FSlateDrawElement::RelativeToElement,
        Params.WireColor
    );
    FSlateDrawElement::MakeRotatedBox(
        DrawElementsList,
        ArrowLayerID,
        FPaintGeometry(EndArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
        ArrowImage,
        ESlateDrawEffect::None,
        EndAngleInRadians,
        TOptional<FVector2D>(),
        FSlateDrawElement::RelativeToElement,
        Params.WireColor
        );
}

FLinearColor FRoomRelationshipGraphConnectionDrawingPolicy::GetTransitionColor(bool bIsHovered)
{
    //constexpr FLinearColor HoverColor(0.724f, 0.256f, 0.0f, 1.0f);
    //constexpr FLinearColor BaseColor(0.9f, 0.5f, 0.6f, 1.0f);
    return bIsHovered ? FLinearColor::Red : FLinearColor::Green;
}
