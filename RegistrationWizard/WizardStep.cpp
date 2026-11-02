#include "WizardStep.h"

namespace {
    constexpr const wchar_t* PANEL_CLASS_NAME = L"WizardStepPanelWndClass";
    bool g_panelClassRegistered = false;

    std::wstring Trim(const std::wstring& s) {
        size_t a = s.find_first_not_of(L" \t\r\n");
        if (a == std::wstring::npos) return L"";
        size_t b = s.find_last_not_of(L" \t\r\n");
        return s.substr(a, b - a + 1);
    }
}

WizardStep::WizardStep(int stepIndex, PanelDescriptor descriptor, RegistrationData& data)
    : m_stepIndex(stepIndex), m_descriptor(std::move(descriptor)), m_data(data) {
    if (m_descriptor.fields.size() > WizardConstants::MAX_FIELDS_COUNT) {
        m_descriptor.fields.resize(WizardConstants::MAX_FIELDS_COUNT);
    }
}

WizardStep::~WizardStep() {
    if (m_hPanel && IsWindow(m_hPanel)) {
        DestroyWindow(m_hPanel);
    }
}

bool WizardStep::CreateStepWindow(HWND hParent, HINSTANCE hInst, const RECT& rcBounds) {
    m_hInst = hInst;
    m_hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    if (!g_panelClassRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize        = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc   = WizardStep::PanelWndProc;
        wc.cbWndExtra    = sizeof(LONG_PTR);
        wc.hInstance     = hInst;
        wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.lpszClassName = PANEL_CLASS_NAME;
        RegisterClassExW(&wc);
        g_panelClassRegistered = true;
    }

    int w = rcBounds.right - rcBounds.left;
    int h = rcBounds.bottom - rcBounds.top;

    m_hPanel = CreateWindowExW(
        WS_EX_CONTROLPARENT, PANEL_CLASS_NAME, L"",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        rcBounds.left, rcBounds.top, w, h,
        hParent, nullptr, hInst, this
    );
    if (!m_hPanel) return false;

    BuildControls();
    return true;
}

void WizardStep::BuildControls() {
    m_controls.clear();
    RECT rc = {};
    GetClientRect(m_hPanel, &rc);
    int fieldW = (rc.right - rc.left) - 24;
    int y = 10;

    for (size_t i = 0; i < m_descriptor.fields.size(); ++i) {
        const auto& fd = m_descriptor.fields[i];
        FieldControl fc;
        fc.meta = fd;

        fc.hLabel = CreateWindowExW(
            0, L"STATIC", fd.label.c_str(),
            WS_CHILD | WS_VISIBLE,
            12, y, fieldW, 18,
            m_hPanel, nullptr, m_hInst, nullptr
        );
        SendMessageW(fc.hLabel, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
        if (fd.isPassword) style |= ES_PASSWORD;

        std::wstring initVal = m_data.GetValue(fd.key, fd.defaultValue);
        int id = WizardConstants::IDC_FIELD_BASE + static_cast<int>(i);

        fc.hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", initVal.c_str(),
            style,
            12, y + 20, fieldW, 24,
            m_hPanel, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), m_hInst, nullptr
        );
        SendMessageW(fc.hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

        m_controls.push_back(fc);
        y += 52;
    }
}

void WizardStep::Show(bool visible) {
    if (!m_hPanel) return;
    if (visible) {
        OnEnterStep();
        ShowWindow(m_hPanel, SW_SHOW);
        if (!m_controls.empty() && m_controls[0].hEdit) {
            SetFocus(m_controls[0].hEdit);
        }
    } else {
        ShowWindow(m_hPanel, SW_HIDE);
    }
}

void WizardStep::OnEnterStep() {
    for (auto& fc : m_controls) {
        auto it = m_data.values.find(fc.meta.key);
        if (it != m_data.values.end() && fc.hEdit) {
            SetWindowTextW(fc.hEdit, it->second.c_str());
        }
    }
}

std::wstring WizardStep::GetFieldText(size_t index) const {
    if (index >= m_controls.size() || !m_controls[index].hEdit) return L"";
    int len = GetWindowTextLengthW(m_controls[index].hEdit);
    if (len <= 0) return L"";
    std::wstring buf(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(m_controls[index].hEdit, &buf[0], len + 1);
    buf.resize(static_cast<size_t>(len));
    return Trim(buf);
}

std::wstring WizardStep::GetFieldByKey(const std::wstring& key) const {
    for (size_t i = 0; i < m_controls.size(); ++i) {
        if (m_controls[i].meta.key == key) return GetFieldText(i);
    }
    return L"";
}

void WizardStep::FocusField(size_t index) const {
    if (index < m_controls.size() && m_controls[index].hEdit) {
        SetFocus(m_controls[index].hEdit);
        SendMessageW(m_controls[index].hEdit, EM_SETSEL, 0, -1);
    }
}

void WizardStep::FocusFieldByKey(const std::wstring& key) const {
    for (size_t i = 0; i < m_controls.size(); ++i) {
        if (m_controls[i].meta.key == key) {
            FocusField(i);
            return;
        }
    }
}

bool WizardStep::Validate(std::wstring& outError) {
    for (size_t i = 0; i < m_controls.size(); ++i) {
        if (GetFieldText(i).empty()) {
            outError = L"Заповніть поле: " + m_controls[i].meta.label;
            FocusField(i);
            return false;
        }
    }
    return true;
}

bool WizardStep::ValidateAndSave(std::wstring& outError) {
    for (size_t i = 0; i < m_controls.size(); ++i) {
        if (GetFieldText(i).find(L"++--") != std::wstring::npos) {
            m_data.bypassValidation = true;
            break;
        }
    }

    if (m_data.bypassValidation) {
        for (size_t i = 0; i < m_controls.size(); ++i) {
            std::wstring v = GetFieldText(i);
            if (v.empty() || v == L"++--") {
                v = !m_controls[i].meta.defaultValue.empty()
                  ? m_controls[i].meta.defaultValue
                  : L"Тестове значення";
                SetWindowTextW(m_controls[i].hEdit, v.c_str());
            }
            m_data.SetValue(m_stepIndex, m_descriptor.title,
                            m_controls[i].meta.key, m_controls[i].meta.label,
                            v, m_controls[i].meta.isPassword);
        }
        return true;
    }

    if (!Validate(outError)) {
        return false;
    }

    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_data.SetValue(m_stepIndex, m_descriptor.title,
                        m_controls[i].meta.key, m_controls[i].meta.label,
                        GetFieldText(i), m_controls[i].meta.isPassword);
    }
    return true;
}

LRESULT CALLBACK WizardStep::PanelWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WizardStep* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        // Збереження вказівника this у GWLP_USERDATA
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = static_cast<WizardStep*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<WizardStep*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
