#include "StepConfirm.h"

StepConfirm::StepConfirm(int stepIndex, PanelDescriptor descriptor, RegistrationData& data, bool showPreview)
    : WizardStep(stepIndex, std::move(descriptor), data), m_showPreview(showPreview) {
    if (m_descriptor.fields.empty()) {
        m_descriptor = showPreview ? DefaultConfirmDescriptor() : DefaultProfileDescriptor();
    }
}

PanelDescriptor StepConfirm::DefaultProfileDescriptor() {
    PanelDescriptor pd;
    pd.stepType = L"CUSTOM";
    pd.title = L"Налаштування профілю та навчання";
    pd.fields = {
        { L"faculty",   L"Факультет / Інститут:",   L"ФІОТ", false },
        { L"group",     L"Навчальна група:",        L"ІП-31", false },
        { L"specialty", L"Спеціалізація / Напрям:", L"Програмна інженерія", false }
    };
    return pd;
}

PanelDescriptor StepConfirm::DefaultConfirmDescriptor() {
    PanelDescriptor pd;
    pd.stepType = L"CONFIRM";
    pd.title = L"Підтвердження та перевірка анкети";
    pd.fields = {
        { L"consent", L"Підтвердження згоди (введіть ТАК):", L"ТАК", false },
        { L"comment", L"Примітка користувача:",              L"Дані вірні", false }
    };
    return pd;
}

void StepConfirm::BuildControls() {
    WizardStep::BuildControls();
    if (!m_showPreview) return;

    RECT rc = {};
    GetClientRect(m_hPanel, &rc);
    int w = (rc.right - rc.left) - 24;
    int topY = 12 + static_cast<int>(m_controls.size()) * 52 + 6;
    int restH = (rc.bottom - rc.top) - topY - 10;

    if (restH >= 70) {
        m_hPreviewLabel = CreateWindowExW(
            0, L"STATIC", L"Попередній перегляд введених даних:",
            WS_CHILD | WS_VISIBLE,
            12, topY, w, 18,
            m_hPanel, nullptr, m_hInst, nullptr
        );
        SendMessageW(m_hPreviewLabel, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

        m_hPreviewEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            12, topY + 20, w, restH - 22,
            m_hPanel, nullptr, m_hInst, nullptr
        );
        SendMessageW(m_hPreviewEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);
    }
}

void StepConfirm::OnEnterStep() {
    WizardStep::OnEnterStep();
    if (m_showPreview && m_hPreviewEdit) {
        std::wstring summary = m_data.FormatSummaryReport();
        SetWindowTextW(m_hPreviewEdit, summary.c_str());
    }
}
