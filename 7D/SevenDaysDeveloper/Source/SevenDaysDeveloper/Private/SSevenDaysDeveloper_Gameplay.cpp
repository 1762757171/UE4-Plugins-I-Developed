
#include "SSevenDaysDeveloper_Gameplay.h"
#include "SEditableTextBox.h"
#include "SSlider.h"
#include "Engine.h"
#include "SevenDaysDeveloperRuntime/Public/SevenDaysDeveloperRuntimeModule.h"

#define LOCTEXT_NAMESPACE "FSevenDaysDeveloperModule"

void SSevenDaysDeveloper_Gameplay::Construct(const FArguments& InArgs)
{
    InitSpeed();
    
    this->ChildSlot
    [
        SNew(SVerticalBox)
            + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBorder)
                        .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
                        .BorderBackgroundColor( FLinearColor( .6, .6, .6, 1.0f ) )
                        .Padding( 3.0f )
                        .HAlign(HAlign_Center)
                        [
                            SNew( STextBlock )
                                .Text( LOCTEXT("RuntimeGameplay","Runtime Gameplay") )
                                .Font( FEditorStyle::GetFontStyle( "BoldFont" ) )
                                .ShadowOffset( FVector2D( 1.0f, 1.0f ) )
                        ]
                ]
            + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SNew(SVerticalBox)
                        +SVerticalBox::Slot()
                            .AutoHeight()
                                [
                                    Construct_Gameplay_SpeedModifier()
                                ]
                        +SVerticalBox::Slot()
                            .AutoHeight()
                                [
                                    Construct_Gameplay_TransportTestMap()
                                ]
                        
                ]
    ]
    ;
            
}

bool SSevenDaysDeveloper_Gameplay::IsGameplaySpeedModificationEnabled() const
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    return Module.GetSpeed() != -1.0f;
}

FText SSevenDaysDeveloper_Gameplay::GetSpeedText() const
{
    return FText::FromString(FString::SanitizeFloat(GetSliderShown()));
}

ECheckBoxState SSevenDaysDeveloper_Gameplay::GetCheckboxState() const
{
    return IsGameplaySpeedModificationEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSevenDaysDeveloper_Gameplay::OnSpeedTextCommitted(const FText& TextCommitted, ETextCommit::Type CommitType)
{
    float SpeedCommitted = FCString::Atof(*TextCommitted.ToString());
    SpeedCommitted = FMath::Clamp(SpeedCommitted, SpeedSlider->GetMinValue(), SpeedSlider->GetMaxValue());
    SpeedSlider->SetValue(SpeedCommitted);
    OnSpeedValueChanged(SpeedCommitted);
}

void SSevenDaysDeveloper_Gameplay::OnSpeedValueChanged(float NewValue)
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.ChangeSpeed(NewValue);
}

void SSevenDaysDeveloper_Gameplay::OnSpeedCheckBoxClicked(ECheckBoxState NewState)
{
    float NewSpeed = -1.0f;
    switch (NewState)
    {
        case ECheckBoxState::Checked:
            NewSpeed = SpeedSlider->GetValue();
            break;
        case ECheckBoxState::Unchecked:
            NewSpeed = -1.0f;
            break;
        default:
            break;
    }
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.ChangeSpeed(NewSpeed);
}

FReply SSevenDaysDeveloper_Gameplay::TransportToGameplayMap()
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.GameplayTest(true);
    return FReply::Handled();
}

FReply SSevenDaysDeveloper_Gameplay::Goback()
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.GameplayTest(false);
    return FReply::Handled();
}

void SSevenDaysDeveloper_Gameplay::InitSpeed()
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    Module.ChangeSpeed(500.0f);
}

TSharedRef<SWidget> SSevenDaysDeveloper_Gameplay::Construct_Gameplay_SpeedModifier()
{
    return
    SNew(SBorder)
        .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
        .BorderBackgroundColor( FLinearColor( .4, .4, .4, 1.0f ) )
        .Padding( 3.0f )
        .HAlign(HAlign_Fill)
        [
            SNew(SVerticalBox)
                +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                            +SHorizontalBox::Slot()
                                .AutoWidth()
                                [
                                    SAssignNew(SpeedCheckbox, SCheckBox)
                                        .IsChecked(this, &SSevenDaysDeveloper_Gameplay::GetCheckboxState)
                                        .OnCheckStateChanged(this, &SSevenDaysDeveloper_Gameplay::OnSpeedCheckBoxClicked)
                                ]
                            +SHorizontalBox::Slot()
                                .AutoWidth()
                                [
                                    SNew(STextBlock)
                                        .Text(LOCTEXT("ChangeCharacterSpeedLabel", "Change character move speed: "))
                                        .IsEnabled(this, &SSevenDaysDeveloper_Gameplay::IsGameplaySpeedModificationEnabled)
                                ]
                            +SHorizontalBox::Slot()
                                .FillWidth(1.0f)
                                [
                                    SNew(SEditableTextBox)
                                        .Text(this, &SSevenDaysDeveloper_Gameplay::GetSpeedText)
                                        .OnTextCommitted(this, &SSevenDaysDeveloper_Gameplay::OnSpeedTextCommitted)
                                        .IsEnabled(this, &SSevenDaysDeveloper_Gameplay::IsGameplaySpeedModificationEnabled)
                                ]
                    ]
                +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SAssignNew(SpeedSlider, SSlider)
                            .IsEnabled(this, &SSevenDaysDeveloper_Gameplay::IsGameplaySpeedModificationEnabled)
                            .Orientation(Orient_Horizontal)
                            .Value(this, &SSevenDaysDeveloper_Gameplay::GetSliderShown)
                            .MinValue(100)
                            .MaxValue(1500)
                            .StepSize(1)
                            .OnValueChanged(this, &SSevenDaysDeveloper_Gameplay::OnSpeedValueChanged)
                    ]
        ]
    ;
}

TSharedRef<SWidget> SSevenDaysDeveloper_Gameplay::Construct_Gameplay_TransportTestMap()
{
    return
    SNew(SBorder)
        .BorderImage( FEditorStyle::GetBrush( "DetailsView.CategoryTop" ) )
        .BorderBackgroundColor( FLinearColor( .4, .4, .4, 1.0f ) )
        .Padding( 15.0f )
        .HAlign(HAlign_Fill)
        [
            SNew(SVerticalBox)
                +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SButton)
                            .ButtonColorAndOpacity(FSlateColor(FLinearColor::Yellow))
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .ContentPadding(15.0f)
                            .OnClicked(this, &SSevenDaysDeveloper_Gameplay::TransportToGameplayMap)
                            [
                                SNew(STextBlock)
                                    .Text(LOCTEXT("TransportGameplay", "Transport to Gameplay map"))
                                    .HighlightText(LOCTEXT("TransportHignlight", "Gameplay"))
                            ]
                    ]
                +SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SButton)
                            .ButtonColorAndOpacity(FSlateColor(FLinearColor::Yellow))
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .ContentPadding(10.0f)
                            .OnClicked(this, &SSevenDaysDeveloper_Gameplay::Goback)
                            [
                                SNew(STextBlock)
                                    .Text(LOCTEXT("GobackGameplay", "Getback to Start Location"))
                                    .HighlightText(LOCTEXT("GobackHighlight", "Start Location"))
                            ]
                    ]
                    
        ]
    ;
}

float SSevenDaysDeveloper_Gameplay::GetSliderShown() const
{
    FSevenDaysDeveloperRuntimeModule& Module = SevenDaysDeveloperUtils::GetRuntimeModule();
    return Module.GetSpeed() != -1.0f ? Module.GetSpeed() : Module.GetLastCachedSpeed();
}

#undef LOCTEXT_NAMESPACE
