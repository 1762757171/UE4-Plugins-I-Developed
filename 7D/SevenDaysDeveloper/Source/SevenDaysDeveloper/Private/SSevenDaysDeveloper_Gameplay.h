#pragma once

#include "CoreMinimal.h"
#include "SCheckBox.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SevenDaysDeveloperUtils.h"
#include "SSlider.h"

class SSevenDaysDeveloper_Gameplay : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SSevenDaysDeveloper_Gameplay) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

public:
    bool IsGameplaySpeedModificationEnabled() const;
    FText GetSpeedText() const;
    ECheckBoxState GetCheckboxState() const;
    void OnSpeedTextCommitted(const FText& TextCommitted, ETextCommit::Type CommitType);
    void OnSpeedValueChanged(float NewValue);
    void OnSpeedCheckBoxClicked(ECheckBoxState NewState);

    FReply TransportToGameplayMap();
    FReply Goback();

    void InitSpeed();
private:
    TSharedRef<SWidget> Construct_Gameplay_SpeedModifier();
    TSharedRef<SWidget> Construct_Gameplay_TransportTestMap();

    float GetSliderShown() const;
private:
    TSharedPtr<SCheckBox> SpeedCheckbox;
	TSharedPtr<SSlider> SpeedSlider;
};


