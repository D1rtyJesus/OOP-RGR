#pragma once
#include "WizardStep.h"

class StepAccount : public WizardStep {
public:
    StepAccount(int stepIndex, PanelConfig cfg, RegistrationData& data);
    std::wstring GetType() const override { return L"ACCOUNT"; }
    bool Validate(std::wstring& err) override;
    static PanelConfig MakeDefault();
};
