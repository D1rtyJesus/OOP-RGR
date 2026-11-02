#pragma once

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <sstream>

namespace WizardConstants {
    constexpr int MIN_PANELS_COUNT = 3;
    constexpr int MAX_PANELS_COUNT = 10;
    constexpr int MIN_FIELDS_COUNT = 1;
    constexpr int MAX_FIELDS_COUNT = 5;

    constexpr int WINDOW_WIDTH  = 640;
    constexpr int WINDOW_HEIGHT = 480;
    constexpr int HEADER_HEIGHT = 50;
    constexpr int FOOTER_HEIGHT = 55;
    constexpr int PANEL_MARGIN  = 16;

    constexpr int IDC_BTN_BACK    = 1001;
    constexpr int IDC_BTN_NEXT    = 1002;
    constexpr int IDC_BTN_CANCEL  = 1003;
    constexpr int IDC_RESULT_EDIT = 1004;
    constexpr int IDC_HEADER_TEXT = 1005;
    constexpr int IDC_FIELD_BASE  = 2000;

    constexpr const wchar_t* CONFIG_FILE_NAME = L"wizard_config.txt";
    constexpr const wchar_t* RESULT_FILE_NAME = L"registration_result.txt";
}

struct FieldDescriptor {
    std::wstring key;
    std::wstring label;
    std::wstring defaultValue;
    bool isPassword = false;
};

struct PanelDescriptor {
    std::wstring stepType;
    std::wstring title;
    std::vector<FieldDescriptor> fields;
};

struct RegistrationData {
    struct Entry {
        int panelIndex;
        std::wstring panelTitle;
        std::wstring fieldKey;
        std::wstring fieldLabel;
        std::wstring value;
        bool isPassword;
    };

    std::vector<Entry> entries;
    std::map<std::wstring, std::wstring> values;
    bool bypassValidation = false;

    void SetValue(int panelIdx, const std::wstring& panelTitle,
                  const std::wstring& key, const std::wstring& label,
                  const std::wstring& val, bool isPass = false) {
        values[key] = val;
        for (auto& e : entries) {
            if (e.panelIndex == panelIdx && e.fieldKey == key) {
                e.value = val;
                e.panelTitle = panelTitle;
                e.fieldLabel = label;
                e.isPassword = isPass;
                return;
            }
        }
        entries.push_back({ panelIdx, panelTitle, key, label, val, isPass });
    }

    std::wstring GetValue(const std::wstring& key, const std::wstring& fallback = L"") const {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : fallback;
    }

    std::wstring FormatSummaryReport() const {
        std::wostringstream oss;
        oss << L"=== ПІДСУМКОВІ ДАНІ РЕЄСТРАЦІЇ ===\r\n\r\n";
        int lastPanel = -1;
        for (const auto& e : entries) {
            if (e.panelIndex != lastPanel) {
                lastPanel = e.panelIndex;
                oss << L"[Крок " << (lastPanel + 1) << L": " << e.panelTitle << L"]\r\n";
            }
            std::wstring shown = e.isPassword ? std::wstring(e.value.size(), L'*') : e.value;
            oss << L"  " << e.fieldLabel << L": " << shown << L"\r\n";
        }
        return oss.str();
    }
};
