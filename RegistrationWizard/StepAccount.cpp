#include "StepAccount.h"

StepAccount::StepAccount(int stepIndex, PanelConfig cfg, RegistrationData& data)
    : WizardStep(stepIndex, std::move(cfg), data) {
    if (this->cfg.fields.empty()) {
        this->cfg = MakeDefault();
    }
}

PanelConfig StepAccount::MakeDefault() {
    PanelConfig cfg;
    cfg.type = L"ACCOUNT";
    cfg.title = L"Облікові дані та безпека";
    cfg.fields = {
        { L"email",        L"Електронна пошта (Email):", L"", false },
        { L"login",        L"Логін користувача:",        L"", false },
        { L"password",     L"Пароль (мін. 6 символів):", L"", true  },
        { L"pass_confirm", L"Підтвердження пароля:",     L"", true  }
    };
    return cfg;
}

bool StepAccount::Validate(std::wstring& err) {
    if (!WizardStep::Validate(err)) return false;

    std::wstring email = GetByKey(L"email");
    if (!email.empty()) {
        size_t a = email.find(L'@');
        if (a == std::wstring::npos || a == 0 || email.find(L'.', a) == std::wstring::npos) {
            err = L"Некоректний формат електронної пошти (user@domain).";
            FocusKey(L"email");
            return false;
        }
    }

    std::wstring login = GetByKey(L"login");
    if (!login.empty() && login.size() < 3) {
        err = L"Логін має містити не менше 3 символів.";
        FocusKey(L"login");
        return false;
    }

    std::wstring pass = GetByKey(L"password");
    if (!pass.empty() && pass.size() < 6) {
        err = L"Пароль має містити не менше 6 символів.";
        FocusKey(L"password");
        return false;
    }

    std::wstring str1 = GetByKey(L"pass_confirm");
    if (!str1.empty() && pass != str1) {
        err = L"Пароль та підтвердження пароля не збігаються.";
        FocusKey(L"pass_confirm");
        return false;
    }
    return true;
}
