#include "WizardWindow.h"

namespace {
    std::vector<std::wstring> Split(const std::wstring& str1, wchar_t ch = L'|') {
        std::vector<std::wstring> res;
        std::wstringstream ss(str1);
        std::wstring item;
        while (std::getline(ss, item, ch)) {
            res.push_back(item);
        }
        return res;
    }

    std::wstring FromUtf8(const std::string& str1) {
        if (str1.empty()) return L"";
        int n = MultiByteToWideChar(CP_UTF8, 0, str1.data(), static_cast<int>(str1.size()), nullptr, 0);
        if (n <= 0) return L"";
        std::wstring buf(static_cast<size_t>(n), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str1.data(), static_cast<int>(str1.size()), &buf[0], n);
        return buf;
    }

    std::string ToUtf8(const std::wstring& str1) {
        if (str1.empty()) return "";
        int n = WideCharToMultiByte(CP_UTF8, 0, str1.data(), static_cast<int>(str1.size()), nullptr, 0, nullptr, nullptr);
        if (n <= 0) return "";
        std::string buf(static_cast<size_t>(n), '\0');
        WideCharToMultiByte(CP_UTF8, 0, str1.data(), static_cast<int>(str1.size()), &buf[0], n, nullptr, nullptr);
        return buf;
    }
}

WizardWindow::WizardWindow(HINSTANCE hInst) : hInst(hInst) {}

void WizardWindow::WriteDefaultConfig(const std::wstring& path) const {
    std::wostringstream ss;
    ss << L"# Конфігурація панелей реєстрації (3..10 панелей, 1..5 полів)\n";
    ss << L"PANEL|PERSONAL|Особисті дані користувача\n";
    ss << L"FIELD|full_name|Прізвище, Ім'я та По батькові:||0\n";
    ss << L"FIELD|phone|Номер телефону (+380...):|+380|0\n";
    ss << L"FIELD|city|Місто проживання:|Київ|0\n";
    ss << L"FIELD|birth_date|Дата народження (ДД.ММ.РРРР):||0\n\n";

    ss << L"PANEL|ACCOUNT|Облікові дані та безпека\n";
    ss << L"FIELD|email|Електронна пошта (Email):||0\n";
    ss << L"FIELD|login|Логін користувача:||0\n";
    ss << L"FIELD|password|Пароль (мін. 6 символів):||1\n";
    ss << L"FIELD|pass_confirm|Підтвердження пароля:||1\n\n";

    ss << L"PANEL|CUSTOM|Налаштування профілю та навчання\n";
    ss << L"FIELD|faculty|Факультет / Інститут:|ФІОТ|0\n";
    ss << L"FIELD|group|Навчальна група:|ІП-31|0\n";
    ss << L"FIELD|specialty|Спеціалізація / Напрям:|Програмна інженерія|0\n\n";

    ss << L"PANEL|CONFIRM|Підтвердження та перевірка анкети\n";
    ss << L"FIELD|consent|Підтвердження згоди (введіть ТАК):|ТАК|0\n";
    ss << L"FIELD|comment|Примітка користувача:|Дані вірні|0\n";

    std::ofstream out(path.c_str(), std::ios::binary);
    if (out.is_open()) {
        std::string buf = ToUtf8(ss.str());
        out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    }
}

bool WizardWindow::LoadConfig(const std::wstring& path) {
    configs.clear();
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in.is_open()) {
        WriteDefaultConfig(path);
        in.open(path.c_str(), std::ios::binary);
    }

    std::string buf;
    PanelConfig* curr = nullptr;
    while (std::getline(in, buf)) {
        if (!buf.empty() && buf.back() == '\r') buf.pop_back();
        std::wstring str1 = FromUtf8(buf);
        if (str1.empty() || str1[0] == L'#') continue;

        auto a = Split(str1);
        if (a.empty()) continue;

        if (a[0] == L"PANEL" && a.size() >= 3) {
            if (configs.size() < MAX_PANELS) {
                configs.push_back({ a[1], a[2], {} });
                curr = &configs.back();
            }
        } else if (a[0] == L"FIELD" && a.size() >= 5 && curr) {
            if (curr->fields.size() < MAX_FIELDS) {
                curr->fields.push_back({ a[1], a[2], a[3], a[4] == L"1" });
            }
        }
    }

    if (configs.size() < MIN_PANELS) {
        configs = {
            StepPersonal::MakeDefault(),
            StepAccount::MakeDefault(),
            StepConfirm::MakeProfileDefault(),
            StepConfirm::MakeConfirmDefault()
        };
    }
    return true;
}

void WizardWindow::BuildSteps() {
    steps.clear();
    for (size_t i = 0; i < configs.size(); ++i) {
        const auto& item = configs[i];
        int idx = static_cast<int>(i);
        bool isLast = (i + 1 == configs.size());

        if (item.type == L"PERSONAL") {
            steps.push_back(std::make_unique<StepPersonal>(idx, item, data));
        } else if (item.type == L"ACCOUNT") {
            steps.push_back(std::make_unique<StepAccount>(idx, item, data));
        } else if (item.type == L"CONFIRM" || isLast) {
            steps.push_back(std::make_unique<StepConfirm>(idx, item, data, true));
        } else {
            steps.push_back(std::make_unique<StepConfirm>(idx, item, data, false));
        }
    }
}

bool WizardWindow::Init(int cmdShow) {
    LoadConfig(L"wizard_config.txt");
    BuildSteps();

    hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WizardWindow::WndProc;
    wc.cbWndExtra    = sizeof(LONG_PTR);
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"WizardMainWnd";
    RegisterClassExW(&wc);

    int x = (GetSystemMetrics(SM_CXSCREEN) - WIN_W) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - WIN_H) / 2;

    hWnd = CreateWindowExW(
        0, L"WizardMainWnd",
        L"Майстер реєстрації користувача (Варіант 8)",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        x, y, WIN_W, WIN_H,
        nullptr, nullptr, hInst, this
    );
    if (!hWnd) return false;

    RECT rc = {};
    GetClientRect(hWnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    hTitle = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        16, 14, w - 32, 24,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_TITLE)), hInst, nullptr
    );
    SendMessageW(hTitle, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    int px = 16, py = 48, pw = w - 32, ph = h - 103;
    for (auto& s : steps) {
        s->Create(hWnd, hInst, px, py, pw, ph);
        s->Show(false);
    }

    hResult = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        px, py, pw, ph,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_RESULT)), hInst, nullptr
    );
    SendMessageW(hResult, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    int by = h - 43;
    hBack = CreateWindowExW(
        0, L"BUTTON", L"<< Назад",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        w - 360, by, 105, 28,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_BACK)), hInst, nullptr
    );
    hNext = CreateWindowExW(
        0, L"BUTTON", L"Далі >>",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        w - 245, by, 105, 28,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_NEXT)), hInst, nullptr
    );
    hCancel = CreateWindowExW(
        0, L"BUTTON", L"Відміна",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        w - 130, by, 105, 28,
        hWnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_CANCEL)), hInst, nullptr
    );

    SendMessageW(hBack,   WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessageW(hNext,   WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SendMessageW(hCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    SetStep(0);
    ShowWindow(hWnd, cmdShow);
    UpdateWindow(hWnd);
    return true;
}

void WizardWindow::SetStep(int idx) {
    if (idx < 0 || idx >= static_cast<int>(steps.size())) return;

    for (size_t i = 0; i < steps.size(); ++i) {
        if (static_cast<int>(i) != idx) steps[i]->Show(false);
    }
    stepIndex = idx;
    steps[static_cast<size_t>(stepIndex)]->Show(true);

    std::wstring title = steps[static_cast<size_t>(stepIndex)]->GetTitle();
    std::wstring str1 = L"Крок " + std::to_wstring(stepIndex + 1) + L" з "
                      + std::to_wstring(steps.size()) + L": " + title;
    SetWindowTextW(hTitle, str1.c_str());
    UpdateButtons();
}

void WizardWindow::UpdateButtons() {
    if (done) {
        ShowWindow(hBack, SW_HIDE);
        ShowWindow(hNext, SW_HIDE);
        SetWindowTextW(hCancel, L"Закрити");
        return;
    }

    bool a = (stepIndex == 0);
    bool b = (stepIndex + 1 == static_cast<int>(steps.size()));

    ShowWindow(hBack, a ? SW_HIDE : SW_SHOW);
    SetWindowTextW(hNext, b ? L"Завершити" : L"Далі >>");
}

void WizardWindow::OnNext() {
    if (done || steps.empty()) return;

    std::wstring err;
    if (!steps[static_cast<size_t>(stepIndex)]->SaveStep(err)) {
        MessageBoxW(hWnd, err.c_str(), L"Помилка введення", MB_OK | MB_ICONWARNING);
        return;
    }

    if (stepIndex + 1 < static_cast<int>(steps.size())) {
        SetStep(stepIndex + 1);
    } else {
        if (SaveResult(L"registration_result.txt")) {
            ShowResult(L"registration_result.txt");
        }
    }
}

void WizardWindow::OnBack() {
    if (!done && stepIndex > 0) {
        SetStep(stepIndex - 1);
    }
}

bool WizardWindow::SaveResult(const std::wstring& path) const {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out.is_open()) return false;
    std::string buf = ToUtf8(data.BuildReport());
    out.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    return true;
}

void WizardWindow::ShowResult(const std::wstring& path) {
    for (auto& s : steps) s->Show(false);
    done = true;
    UpdateButtons();

    // читання файлу результату
    std::ifstream in(path.c_str(), std::ios::binary);
    std::string buf((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::wstring str1 = FromUtf8(buf);

    SetWindowTextW(hTitle, L"Реєстрацію завершено. Вміст файлу registration_result.txt:");
    SetWindowTextW(hResult, str1.c_str());
    ShowWindow(hResult, SW_SHOW);
}

int WizardWindow::Run() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WizardWindow::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    WizardWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        // збереження this у GWLP_USERDATA
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = static_cast<WizardWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        if (self) self->hWnd = hWnd;
    } else {
        self = reinterpret_cast<WizardWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }
    return self ? self->OnMsg(msg, wp, lp) : DefWindowProcW(hWnd, msg, wp, lp);
}

LRESULT WizardWindow::OnMsg(UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_COMMAND:
            if (HIWORD(wp) == BN_CLICKED) {
                int id = LOWORD(wp);
                if (id == ID_NEXT)   { OnNext(); return 0; }
                if (id == ID_BACK)   { OnBack(); return 0; }
                if (id == ID_CANCEL) { DestroyWindow(hWnd); return 0; }
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hWnd, msg, wp, lp);
}
