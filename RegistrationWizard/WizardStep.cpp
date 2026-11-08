#include "WizardStep.h"

namespace {
    std::wstring Trim(const std::wstring& str1) {
        size_t a = str1.find_first_not_of(L" \t\r\n");
        if (a == std::wstring::npos) return L"";
        size_t b = str1.find_last_not_of(L" \t\r\n");
        return str1.substr(a, b - a + 1);
    }
}

WizardStep::WizardStep(int stepIndex, PanelConfig cfg, RegistrationData& data)
    : stepIndex(stepIndex), cfg(std::move(cfg)), data(data) {
    if (this->cfg.fields.size() > MAX_FIELDS) {
        this->cfg.fields.resize(MAX_FIELDS);
    }
}

WizardStep::~WizardStep() {
    if (hWnd && IsWindow(hWnd)) {
        DestroyWindow(hWnd);
    }
}

bool WizardStep::Create(HWND hParent, HINSTANCE hInst, int x, int y, int w, int h) {
    this->hInst = hInst;
    hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = WizardStep::WndProc;
    wc.cbWndExtra    = sizeof(LONG_PTR);
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"WizardStepPanel";
    RegisterClassExW(&wc);

    hWnd = CreateWindowExW(
        WS_EX_CONTROLPARENT, L"WizardStepPanel", L"",
        WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        x, y, w, h,
        hParent, nullptr, hInst, this
    );
    if (!hWnd) return false;

    CreateControls();
    return true;
}

void WizardStep::CreateControls() {
    ctrls.clear();
    RECT rc = {};
    GetClientRect(hWnd, &rc);
    int w = (rc.right - rc.left) - 24;
    int y = 10;

    for (size_t i = 0; i < cfg.fields.size(); ++i) {
        const auto& item = cfg.fields[i];
        Ctrl c;
        c.info = item;

        c.hLbl = CreateWindowExW(
            0, L"STATIC", item.label.c_str(),
            WS_CHILD | WS_VISIBLE,
            12, y, w, 18,
            hWnd, nullptr, hInst, nullptr
        );
        SendMessageW(c.hLbl, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

        DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL;
        if (item.isPass) style |= ES_PASSWORD;

        std::wstring str1 = data.Get(item.key, item.defVal);
        int id = ID_FIELD + static_cast<int>(i);

        c.hEdit = CreateWindowExW(
            WS_EX_CLIENTEDGE, L"EDIT", str1.c_str(),
            style,
            12, y + 20, w, 24,
            hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), hInst, nullptr
        );
        SendMessageW(c.hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

        ctrls.push_back(c);
        y += 52;
    }
}

void WizardStep::Show(bool vis) {
    if (!hWnd) return;
    if (vis) {
        OnShow();
        ShowWindow(hWnd, SW_SHOW);
        if (!ctrls.empty() && ctrls[0].hEdit) {
            SetFocus(ctrls[0].hEdit);
        }
    } else {
        ShowWindow(hWnd, SW_HIDE);
    }
}

void WizardStep::OnShow() {
    for (auto& c : ctrls) {
        auto it = data.values.find(c.info.key);
        if (it != data.values.end() && c.hEdit) {
            SetWindowTextW(c.hEdit, it->second.c_str());
        }
    }
}

std::wstring WizardStep::GetText(size_t i) const {
    if (i >= ctrls.size() || !ctrls[i].hEdit) return L"";
    int n = GetWindowTextLengthW(ctrls[i].hEdit);
    if (n <= 0) return L"";
    std::wstring buf(static_cast<size_t>(n) + 1, L'\0');
    GetWindowTextW(ctrls[i].hEdit, &buf[0], n + 1);
    buf.resize(static_cast<size_t>(n));
    return Trim(buf);
}

std::wstring WizardStep::GetByKey(const std::wstring& key) const {
    for (size_t i = 0; i < ctrls.size(); ++i) {
        if (ctrls[i].info.key == key) return GetText(i);
    }
    return L"";
}

void WizardStep::FocusIdx(size_t i) const {
    if (i < ctrls.size() && ctrls[i].hEdit) {
        SetFocus(ctrls[i].hEdit);
        SendMessageW(ctrls[i].hEdit, EM_SETSEL, 0, -1);
    }
}

void WizardStep::FocusKey(const std::wstring& key) const {
    for (size_t i = 0; i < ctrls.size(); ++i) {
        if (ctrls[i].info.key == key) {
            FocusIdx(i);
            return;
        }
    }
}

bool WizardStep::Validate(std::wstring& err) {
    for (size_t i = 0; i < ctrls.size(); ++i) {
        if (GetText(i).empty()) {
            err = L"Заповніть поле: " + ctrls[i].info.label;
            FocusIdx(i);
            return false;
        }
    }
    return true;
}

bool WizardStep::SaveStep(std::wstring& err) {
    if (!Validate(err)) return false;

    for (size_t i = 0; i < ctrls.size(); ++i) {
        data.Set(stepIndex, cfg.title,
                 ctrls[i].info.key, ctrls[i].info.label,
                 GetText(i), ctrls[i].info.isPass);
    }
    return true;
}

LRESULT CALLBACK WizardStep::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_NCCREATE) {
        // збереження this у GWLP_USERDATA
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }
    return DefWindowProcW(hWnd, msg, wp, lp);
}
