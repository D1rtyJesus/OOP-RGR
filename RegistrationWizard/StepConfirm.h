#pragma once
#include "WizardStep.h"

class StepConfirm : public WizardStep {
public:
    StepConfirm(int stepIndex, PanelConfig cfg, RegistrationData& data, bool isFinal = true);
    std::wstring GetType() const override { return isFinal ? L"CONFIRM" : L"CUSTOM"; }
    void OnShow() override;
    static PanelConfig MakeProfileDefault();
    static PanelConfig MakeConfirmDefault();

protected:
    void CreateControls() override;

private:
    bool isFinal = true;
    HWND hLbl = nullptr;
    HWND hBox = nullptr;
};
