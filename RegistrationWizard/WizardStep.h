#pragma once
#include "Common.h"

class WizardStep {
public:
    WizardStep(int stepIndex, PanelConfig cfg, RegistrationData& data);
    virtual ~WizardStep();

    bool Create(HWND hParent, HINSTANCE hInst, int x, int y, int w, int h);
    void Show(bool vis);

    virtual void OnShow();
    virtual bool Validate(std::wstring& err);
    bool SaveStep(std::wstring& err);

    virtual std::wstring GetType() const = 0;

    HWND GetHwnd() const { return hWnd; }
    const std::wstring& GetTitle() const { return cfg.title; }

protected:
    struct Ctrl {
        FieldInfo info;
        HWND hLbl = nullptr;
        HWND hEdit = nullptr;
    };

    virtual void CreateControls();
    std::wstring GetText(size_t i) const;
    std::wstring GetByKey(const std::wstring& key) const;
    void FocusKey(const std::wstring& key) const;
    void FocusIdx(size_t i) const;

    int stepIndex;
    PanelConfig cfg;
    RegistrationData& data;
    HWND hWnd = nullptr;
    HINSTANCE hInst = nullptr;
    HFONT hFont = nullptr;
    std::vector<Ctrl> ctrls;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
};

using Panel = WizardStep;
