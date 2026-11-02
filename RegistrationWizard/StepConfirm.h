#pragma once
#include "WizardStep.h"

class StepConfirm : public WizardStep {
public:
    StepConfirm(int stepIndex, PanelDescriptor descriptor, RegistrationData& data, bool showPreview = true);
    std::wstring GetStepType() const override { return m_showPreview ? L"CONFIRM" : L"CUSTOM"; }
    void OnEnterStep() override;
    static PanelDescriptor DefaultProfileDescriptor();
    static PanelDescriptor DefaultConfirmDescriptor();

protected:
    void BuildControls() override;

private:
    bool m_showPreview = true;
    HWND m_hPreviewLabel = nullptr;
    HWND m_hPreviewEdit  = nullptr;
};
