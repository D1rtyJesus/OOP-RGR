#pragma once
#include "WizardStep.h"

class StepAccount : public WizardStep {
public:
    StepAccount(int stepIndex, PanelDescriptor descriptor, RegistrationData& data);
    std::wstring GetStepType() const override { return L"ACCOUNT"; }
    bool Validate(std::wstring& outError) override;
    static PanelDescriptor DefaultDescriptor();
};
