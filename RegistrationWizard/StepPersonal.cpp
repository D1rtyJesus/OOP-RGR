#include "StepPersonal.h"
#include <cwctype>

StepPersonal::StepPersonal(int stepIndex, PanelConfig cfg, RegistrationData& data)
    : WizardStep(stepIndex, std::move(cfg), data) {
    if (this->cfg.fields.empty()) {
        this->cfg = MakeDefault();
    }
}

PanelConfig StepPersonal::MakeDefault() {
    PanelConfig cfg;
    cfg.type = L"PERSONAL";
    cfg.title = L"Особисті дані користувача";
    cfg.fields = {
        { L"full_name",  L"Прізвище, Ім'я та По батькові:", L"",     false },
        { L"phone",      L"Номер телефону (+380...):",      L"+380", false },
        { L"city",       L"Місто проживання:",              L"Київ", false },
        { L"birth_date", L"Дата народження (ДД.ММ.РРРР):",  L"",     false }
    };
    return cfg;
}

bool StepPersonal::Validate(std::wstring& err) {
    if (!WizardStep::Validate(err)) return false;

    std::wstring str1 = GetByKey(L"full_name");
    if (!str1.empty() && str1.find(L' ') == std::wstring::npos) {
        err = L"Введіть Прізвище та Ім'я через пробіл.";
        FocusKey(L"full_name");
        return false;
    }

    std::wstring buf = GetByKey(L"phone");
    if (!buf.empty()) {
        int n = 0;
        for (wchar_t ch : buf) {
            if (std::iswdigit(ch)) ++n;
        }
        if (n < 10 || n > 13) {
            err = L"Номер телефону має містити від 10 до 13 цифр.";
            FocusKey(L"phone");
            return false;
        }
    }
    return true;
}
