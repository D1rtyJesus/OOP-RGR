#include "StepConfirm.h"

StepConfirm::StepConfirm(int stepIndex, PanelConfig cfg, RegistrationData& data, bool isFinal)
    : WizardStep(stepIndex, std::move(cfg), data), isFinal(isFinal) {
    if (this->cfg.fields.empty()) {
        this->cfg = isFinal ? MakeConfirmDefault() : MakeProfileDefault();
    }
}

PanelConfig StepConfirm::MakeProfileDefault() {
    PanelConfig cfg;
    cfg.type = L"CUSTOM";
    cfg.title = L"Налаштування профілю та навчання";
    cfg.fields = {
        { L"faculty",   L"Факультет / Інститут:",   L"ФІОТ", false },
        { L"group",     L"Навчальна група:",        L"ІП-31", false },
        { L"specialty", L"Спеціалізація / Напрям:", L"Програмна інженерія", false }
    };
    return cfg;
}

PanelConfig StepConfirm::MakeConfirmDefault() {
    PanelConfig cfg;
    cfg.type = L"CONFIRM";
    cfg.title = L"Підтвердження та перевірка анкети";
    cfg.fields = {
        { L"consent", L"Підтвердження згоди (введіть ТАК):", L"ТАК", false },
        { L"comment", L"Примітка користувача:",              L"Дані вірні", false }
    };
    return cfg;
}

void StepConfirm::CreateControls() {
    WizardStep::CreateControls();
    if (!isFinal) return;

    RECT rc = {};
    GetClientRect(hWnd, &rc);
    int w = (rc.right - rc.left) - 24;
    int y = 12 + static_cast<int>(ctrls.size()) * 52 + 6;
    int h = (rc.bottom - rc.top) - y - 10;

    if (h >= 70) {
        hLbl = CreateWindowExW(
            0, L"STATIC", L"Попередній перегляд введених даних:",
            WS_CHILD | WS_VISIBLE,
            12, y, w, 18,
            hWnd, nullptr, hInst, nullptr
        );
        SendMessageW(hLbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

        hBox = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            12, y + 20, w, h - 22,
            hWnd, nullptr, hInst, nullptr
        );
        SendMessageW(hBox, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    }
}

void StepConfirm::OnShow() {
    WizardStep::OnShow();
    if (isFinal && hBox) {
        std::wstring buf = data.BuildReport();
        SetWindowTextW(hBox, buf.c_str());
    }
}
