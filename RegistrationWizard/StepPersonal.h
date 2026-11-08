#pragma once
#include "WizardStep.h"

class StepPersonal : public WizardStep {
public:
    StepPersonal(int stepIndex, PanelConfig cfg, RegistrationData& data);
    std::wstring GetType() const override { return L"PERSONAL"; }
    bool Validate(std::wstring& err) override;
    static PanelConfig MakeDefault();
};
