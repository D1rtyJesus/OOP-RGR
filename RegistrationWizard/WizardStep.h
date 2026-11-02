#pragma once
#include "Common.h"

class WizardStep {
public:
    WizardStep(int stepIndex, PanelDescriptor descriptor, RegistrationData& data);
    virtual ~WizardStep();

    WizardStep(const WizardStep&) = delete;
    WizardStep& operator=(const WizardStep&) = delete;

    bool CreateStepWindow(HWND hParent, HINSTANCE hInst, const RECT& rcBounds);
    void Show(bool visible);

    virtual void OnEnterStep();
    virtual bool Validate(std::wstring& outError);
    bool ValidateAndSave(std::wstring& outError);

    virtual std::wstring GetStepType() const = 0;

    HWND GetHwnd() const { return m_hPanel; }
    const std::wstring& GetTitle() const { return m_descriptor.title; }

protected:
    struct FieldControl {
        FieldDescriptor meta;
        HWND hLabel = nullptr;
        HWND hEdit  = nullptr;
    };

    virtual void BuildControls();
    std::wstring GetFieldText(size_t index) const;
    std::wstring GetFieldByKey(const std::wstring& key) const;
    void FocusFieldByKey(const std::wstring& key) const;
    void FocusField(size_t index) const;

    int m_stepIndex;
    PanelDescriptor m_descriptor;
    RegistrationData& m_data;
    HWND m_hPanel = nullptr;
    HINSTANCE m_hInst = nullptr;
    HFONT m_hFont = nullptr;
    std::vector<FieldControl> m_controls;

private:
    static LRESULT CALLBACK PanelWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

using Panel = WizardStep;
