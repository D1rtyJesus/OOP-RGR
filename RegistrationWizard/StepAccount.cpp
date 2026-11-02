#include "StepAccount.h"

StepAccount::StepAccount(int stepIndex, PanelDescriptor descriptor, RegistrationData& data)
    : WizardStep(stepIndex, std::move(descriptor), data) {
    if (m_descriptor.fields.empty()) {
        m_descriptor = DefaultDescriptor();
    }
}

PanelDescriptor StepAccount::DefaultDescriptor() {
    PanelDescriptor pd;
    pd.stepType = L"ACCOUNT";
    pd.title = L"Облікові дані та безпека";
    pd.fields = {
        { L"email",        L"Електронна пошта (Email):", L"", false },
        { L"login",        L"Логін користувача:",        L"", false },
        { L"password",     L"Пароль (мін. 6 символів):", L"", true  },
        { L"pass_confirm", L"Підтвердження пароля:",     L"", true  }
    };
    return pd;
}

bool StepAccount::Validate(std::wstring& outError) {
    if (!WizardStep::Validate(outError)) return false;

    std::wstring email = GetFieldByKey(L"email");
    if (!email.empty()) {
        size_t at = email.find(L'@');
        if (at == std::wstring::npos || at == 0 || email.find(L'.', at) == std::wstring::npos) {
            outError = L"Некоректний формат електронної пошти (очікується user@domain).";
            FocusFieldByKey(L"email");
            return false;
        }
    }

    std::wstring pass = GetFieldByKey(L"password");
    if (!pass.empty() && pass.size() < 6) {
        outError = L"Пароль має містити щонайменше 6 символів.";
        FocusFieldByKey(L"password");
        return false;
    }

    std::wstring confirm = GetFieldByKey(L"pass_confirm");
    if (!confirm.empty() && pass != confirm) {
        outError = L"Пароль та підтвердження пароля не збігаються.";
        FocusFieldByKey(L"pass_confirm");
        return false;
    }
    return true;
}
