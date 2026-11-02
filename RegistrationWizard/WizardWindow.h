#pragma once
#include "Common.h"
#include "WizardStep.h"
#include "StepPersonal.h"
#include "StepAccount.h"
#include "StepConfirm.h"

class WizardWindow {
public:
    explicit WizardWindow(HINSTANCE hInst);
    ~WizardWindow() = default;

    WizardWindow(const WizardWindow&) = delete;
    WizardWindow& operator=(const WizardWindow&) = delete;

    bool Initialize(int nCmdShow);
    int RunMessageLoop();

private:
    bool LoadConfig(const std::wstring& path);
    void WriteDefaultConfig(const std::wstring& path) const;
    void CreateStepObjects();

    void SetActiveStep(int index);
    void UpdateButtons();
    void OnNextClicked();
    void OnBackClicked();
    void OnCancelClicked();

    bool SaveResultFile(const std::wstring& path) const;
    void DisplayResultFile(const std::wstring& path);

    static LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE m_hInst = nullptr;
    HWND m_hWnd = nullptr;
    HWND m_hHeaderLabel = nullptr;
    HWND m_hBtnBack = nullptr;
    HWND m_hBtnNext = nullptr;
    HWND m_hBtnCancel = nullptr;
    HWND m_hResultEdit = nullptr;
    HFONT m_hFont = nullptr;

    RegistrationData m_data;
    std::vector<PanelDescriptor> m_configs;
    std::vector<std::unique_ptr<WizardStep>> m_steps;
    int m_currentIndex = 0;
    bool m_finished = false;
};
