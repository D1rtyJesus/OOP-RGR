#pragma once
#include "WizardStep.h"

class StepPersonal : public WizardStep {
public:
    StepPersonal(int stepIndex, PanelDescriptor descriptor, RegistrationData& data);
    std::wstring GetStepType() const override { return L"PERSONAL"; }
    bool Validate(std::wstring& outError) override;
    static PanelDescriptor DefaultDescriptor();
};
