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

    bool Init(int cmdShow);
    int Run();

private:
    bool LoadConfig(const std::wstring& path);
    void WriteDefaultConfig(const std::wstring& path) const;
    void BuildSteps();

    void SetStep(int idx);
    void UpdateButtons();
    void OnNext();
    void OnBack();

    bool SaveResult(const std::wstring& path) const;
    void ShowResult(const std::wstring& path);

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT OnMsg(UINT msg, WPARAM wp, LPARAM lp);

    HINSTANCE hInst = nullptr;
    HWND hWnd = nullptr;
    HWND hTitle = nullptr;
    HWND hBack = nullptr;
    HWND hNext = nullptr;
    HWND hCancel = nullptr;
    HWND hResult = nullptr;
    HFONT hFont = nullptr;

    RegistrationData data;
    std::vector<PanelConfig> configs;
    std::vector<std::unique_ptr<WizardStep>> steps;
    int stepIndex = 0;
    bool done = false;
};
