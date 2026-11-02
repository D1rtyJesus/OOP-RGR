#include "StepPersonal.h"
#include <cwctype>

StepPersonal::StepPersonal(int stepIndex, PanelDescriptor descriptor, RegistrationData& data)
    : WizardStep(stepIndex, std::move(descriptor), data) {
    if (m_descriptor.fields.empty()) {
        m_descriptor = DefaultDescriptor();
    }
}

PanelDescriptor StepPersonal::DefaultDescriptor() {
    PanelDescriptor pd;
    pd.stepType = L"PERSONAL";
    pd.title = L"Особисті дані користувача";
    pd.fields = {
        { L"full_name",  L"Прізвище, Ім'я та По батькові:", L"",     false },
        { L"phone",      L"Номер телефону (+380...):",      L"+380", false },
        { L"city",       L"Місто проживання:",              L"Київ", false },
        { L"birth_date", L"Дата народження (ДД.ММ.РРРР):",  L"",     false }
    };
    return pd;
}

bool StepPersonal::Validate(std::wstring& outError) {
    if (!WizardStep::Validate(outError)) return false;

    std::wstring name = GetFieldByKey(L"full_name");
    if (!name.empty() && name.find(L' ') == std::wstring::npos) {
        outError = L"Введіть щонайменше Прізвище та Ім'я через пробіл.";
        FocusFieldByKey(L"full_name");
        return false;
    }

    std::wstring phone = GetFieldByKey(L"phone");
    if (!phone.empty()) {
        size_t digits = 0;
        for (wchar_t ch : phone) {
            if (std::iswdigit(ch)) ++digits;
        }
        if (digits < 10 || digits > 13) {
            outError = L"Номер телефону має містити від 10 до 13 цифр.";
            FocusFieldByKey(L"phone");
            return false;
        }
    }
    return true;
}
