#include "WizardWindow.h"

namespace {
    constexpr const wchar_t* MAIN_WND_CLASS = L"RegistrationWizardMainWnd";

    std::vector<std::wstring> SplitLine(const std::wstring& s, wchar_t delim = L'|') {
        std::vector<std::wstring> parts;
        std::wstringstream ss(s);
        std::wstring item;
        while (std::getline(ss, item, delim)) {
            parts.push_back(item);
        }
        return parts;
    }

    std::wstring FromUtf8(const std::string& bytes) {
        if (bytes.empty()) return L"";
        int n = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
        if (n <= 0) return L"";
        std::wstring ws(static_cast<size_t>(n), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), &ws[0], n);
        return ws;
    }

    std::string ToUtf8(const std::wstring& ws) {
        if (ws.empty()) return "";
        int n = WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), nullptr, 0, nullptr, nullptr);
        if (n <= 0) return "";
        std::string s(static_cast<size_t>(n), '\0');
        WideCharToMultiByte(CP_UTF8, 0, ws.data(), static_cast<int>(ws.size()), &s[0], n, nullptr, nullptr);
        return s;
    }
}

WizardWindow::WizardWindow(HINSTANCE hInst) : m_hInst(hInst) {}

void WizardWindow::WriteDefaultConfig(const std::wstring& path) const {
    std::wostringstream oss;
    oss << L"# Конфігурація панелей реєстрації (від 3 до 10 панелей, від 1 до 5 полів)\n";
    oss << L"PANEL|PERSONAL|Особисті дані користувача\n";
    oss << L"FIELD|full_name|Прізвище, Ім'я та По батькові:||0\n";
    oss << L"FIELD|phone|Номер телефону (+380...):|+380|0\n";
    oss << L"FIELD|city|Місто проживання:|Київ|0\n";
    oss << L"FIELD|birth_date|Дата народження (ДД.ММ.РРРР):||0\n\n";

    oss << L"PANEL|ACCOUNT|Облікові дані та безпека\n";
    oss << L"FIELD|email|Електронна пошта (Email):||0\n";
    oss << L"FIELD|login|Логін користувача:||0\n";
    oss << L"FIELD|password|Пароль (мін. 6 символів):||1\n";
    oss << L"FIELD|pass_confirm|Підтвердження пароля:||1\n\n";

    oss << L"PANEL|CUSTOM|Налаштування профілю та навчання\n";
    oss << L"FIELD|faculty|Факультет / Інститут:|ФІОТ|0\n";
    oss << L"FIELD|group|Навчальна група:|ІП-31|0\n";
    oss << L"FIELD|specialty|Спеціалізація / Напрям:|Програмна інженерія|0\n\n";

    oss << L"PANEL|CONFIRM|Підтвердження та перевірка анкети\n";
    oss << L"FIELD|consent|Підтвердження згоди (введіть ТАК):|ТАК|0\n";
    oss << L"FIELD|comment|Примітка користувача:|Дані вірні|0\n";

    std::ofstream out(path.c_str(), std::ios::binary);
    if (out.is_open()) {
        std::string utf8 = ToUtf8(oss.str());
        out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    }
}

bool WizardWindow::LoadConfig(const std::wstring& path) {
    m_configs.clear();
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in.is_open()) {
        WriteDefaultConfig(path);
        in.open(path.c_str(), std::ios::binary);
    }

    std::string raw;
    PanelDescriptor* curr = nullptr;
    while (std::getline(in, raw)) {
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();
        std::wstring line = FromUtf8(raw);
        if (line.empty() || line[0] == L'#') continue;

        auto parts = SplitLine(line);
        if (parts.empty()) continue;

        if (parts[0] == L"PANEL" && parts.size() >= 3) {
            if (m_configs.size() < WizardConstants::MAX_PANELS_COUNT) {
                m_configs.push_back({ parts[1], parts[2], {} });
                curr = &m_configs.back();
            }
        } else if (parts[0] == L"FIELD" && parts.size() >= 5 && curr) {
            if (curr->fields.size() < WizardConstants::MAX_FIELDS_COUNT) {
                curr->fields.push_back({ parts[1], parts[2], parts[3], parts[4] == L"1" });
            }
        }
    }

    if (m_configs.size() < WizardConstants::MIN_PANELS_COUNT) {
        m_configs = {
            StepPersonal::DefaultDescriptor(),
            StepAccount::DefaultDescriptor(),
            StepConfirm::DefaultProfileDescriptor(),
            StepConfirm::DefaultConfirmDescriptor()
        };
    }
    return true;
}

void WizardWindow::CreateStepObjects() {
    m_steps.clear();
    for (size_t i = 0; i < m_configs.size(); ++i) {
        const auto& cfg = m_configs[i];
        int idx = static_cast<int>(i);
        bool isLast = (i + 1 == m_configs.size());

        if (cfg.stepType == L"PERSONAL") {
            m_steps.push_back(std::make_unique<StepPersonal>(idx, cfg, m_data));
        } else if (cfg.stepType == L"ACCOUNT") {
            m_steps.push_back(std::make_unique<StepAccount>(idx, cfg, m_data));
        } else if (cfg.stepType == L"CONFIRM" || isLast) {
            m_steps.push_back(std::make_unique<StepConfirm>(idx, cfg, m_data, true));
        } else {
            m_steps.push_back(std::make_unique<StepConfirm>(idx, cfg, m_data, false));
        }
    }
}

bool WizardWindow::Initialize(int nCmdShow) {
    LoadConfig(WizardConstants::CONFIG_FILE_NAME);
    CreateStepObjects();

    m_hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WizardWindow::MainWndProc;
    wc.cbWndExtra    = sizeof(LONG_PTR);
    wc.hInstance     = m_hInst;
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = MAIN_WND_CLASS;
    RegisterClassExW(&wc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - WizardConstants::WINDOW_WIDTH) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - WizardConstants::WINDOW_HEIGHT) / 2;

    m_hWnd = CreateWindowExW(
        0, MAIN_WND_CLASS,
        L"Майстер реєстрації користувача (Варіант 8)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        x, y, WizardConstants::WINDOW_WIDTH, WizardConstants::WINDOW_HEIGHT,
        nullptr, nullptr, m_hInst, this
    );
    if (!m_hWnd) return false;

    RECT rc = {};
    GetClientRect(m_hWnd, &rc);
    int cw = rc.right - rc.left;
    int ch = rc.bottom - rc.top;

    m_hHeaderLabel = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        WizardConstants::PANEL_MARGIN, 14, cw - WizardConstants::PANEL_MARGIN * 2, 24,
        m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(WizardConstants::IDC_HEADER_TEXT)), m_hInst, nullptr
    );
    SendMessageW(m_hHeaderLabel, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

    RECT rcPanel = {
        WizardConstants::PANEL_MARGIN,
        WizardConstants::HEADER_HEIGHT,
        cw - WizardConstants::PANEL_MARGIN,
        ch - WizardConstants::FOOTER_HEIGHT
    };

    for (auto& step : m_steps) {
        step->CreateStepWindow(m_hWnd, m_hInst, rcPanel);
        step->Show(false);
    }

    m_hResultEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        rcPanel.left, rcPanel.top,
        rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top,
        m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(WizardConstants::IDC_RESULT_EDIT)), m_hInst, nullptr
    );
    SendMessageW(m_hResultEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

    int btnY = ch - WizardConstants::FOOTER_HEIGHT + 12;
    m_hBtnBack = CreateWindowExW(
        0, L"BUTTON", L"<< Назад",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        cw - 360, btnY, 105, 28,
        m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(WizardConstants::IDC_BTN_BACK)), m_hInst, nullptr
    );
    m_hBtnNext = CreateWindowExW(
        0, L"BUTTON", L"Далі >>",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        cw - 245, btnY, 105, 28,
        m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(WizardConstants::IDC_BTN_NEXT)), m_hInst, nullptr
    );
    m_hBtnCancel = CreateWindowExW(
        0, L"BUTTON", L"Відміна",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        cw - 130, btnY, 105, 28,
        m_hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(WizardConstants::IDC_BTN_CANCEL)), m_hInst, nullptr
    );

    SendMessageW(m_hBtnBack,   WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);
    SendMessageW(m_hBtnNext,   WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);
    SendMessageW(m_hBtnCancel, WM_SETFONT, reinterpret_cast<WPARAM>(m_hFont), TRUE);

    SetActiveStep(0);
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
    return true;
}

void WizardWindow::SetActiveStep(int index) {
    if (index < 0 || index >= static_cast<int>(m_steps.size())) return;

    // Приховуємо неактивні панелі та показуємо поточну
    for (size_t i = 0; i < m_steps.size(); ++i) {
        if (static_cast<int>(i) != index) m_steps[i]->Show(false);
    }
    m_currentIndex = index;
    m_steps[static_cast<size_t>(m_currentIndex)]->Show(true);

    std::wstring hdr = L"Крок " + std::to_wstring(m_currentIndex + 1) + L" з "
                     + std::to_wstring(m_steps.size()) + L": "
                     + m_steps[static_cast<size_t>(m_currentIndex)]->GetTitle();
    SetWindowTextW(m_hHeaderLabel, hdr.c_str());
    UpdateButtons();
}

void WizardWindow::UpdateButtons() {
    if (m_finished) {
        ShowWindow(m_hBtnBack, SW_HIDE);
        ShowWindow(m_hBtnNext, SW_HIDE);
        SetWindowTextW(m_hBtnCancel, L"Закрити");
        return;
    }

    bool isFirst = (m_currentIndex == 0);
    bool isLast  = (m_currentIndex + 1 == static_cast<int>(m_steps.size()));

    ShowWindow(m_hBtnBack, isFirst ? SW_HIDE : SW_SHOW);
    SetWindowTextW(m_hBtnNext, isLast ? L"Завершити" : L"Далі >>");
}

void WizardWindow::OnNextClicked() {
    if (m_finished || m_steps.empty()) return;

    std::wstring err;
    if (!m_steps[static_cast<size_t>(m_currentIndex)]->ValidateAndSave(err)) {
        MessageBoxW(m_hWnd, err.c_str(), L"Помилка введення", MB_OK | MB_ICONWARNING);
        return;
    }

    if (m_currentIndex + 1 < static_cast<int>(m_steps.size())) {
        SetActiveStep(m_currentIndex + 1);
    } else {
        if (SaveResultFile(WizardConstants::RESULT_FILE_NAME)) {
            DisplayResultFile(WizardConstants::RESULT_FILE_NAME);
        }
    }
}

void WizardWindow::OnBackClicked() {
    if (!m_finished && m_currentIndex > 0) {
        SetActiveStep(m_currentIndex - 1);
    }
}

void WizardWindow::OnCancelClicked() {
    DestroyWindow(m_hWnd);
}

bool WizardWindow::SaveResultFile(const std::wstring& path) const {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out.is_open()) return false;
    std::string utf8 = ToUtf8(m_data.FormatSummaryReport());
    out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    return true;
}

void WizardWindow::DisplayResultFile(const std::wstring& path) {
    for (auto& step : m_steps) step->Show(false);
    m_finished = true;
    UpdateButtons();

    // Читання файлу результату та виведення у головному вікні
    std::ifstream in(path.c_str(), std::ios::binary);
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::wstring content = FromUtf8(bytes);

    SetWindowTextW(m_hHeaderLabel, L"Реєстрацію завершено. Вміст файлу registration_result.txt:");
    SetWindowTextW(m_hResultEdit, content.c_str());
    ShowWindow(m_hResultEdit, SW_SHOW);
}

int WizardWindow::RunMessageLoop() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(m_hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WizardWindow::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WizardWindow* pThis = nullptr;
    if (msg == WM_NCCREATE) {
        // Збереження вказівника this у GWLP_USERDATA
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = static_cast<WizardWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        if (pThis) pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<WizardWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    return pThis ? pThis->HandleMessage(msg, wParam, lParam) : DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT WizardWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                int id = LOWORD(wParam);
                if (id == WizardConstants::IDC_BTN_NEXT)   { OnNextClicked();   return 0; }
                if (id == WizardConstants::IDC_BTN_BACK)   { OnBackClicked();   return 0; }
                if (id == WizardConstants::IDC_BTN_CANCEL) { OnCancelClicked(); return 0; }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(m_hWnd, msg, wParam, lParam);
}
