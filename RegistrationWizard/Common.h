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

constexpr int MIN_PANELS = 3;
constexpr int MAX_PANELS = 10;
constexpr int MIN_FIELDS = 1;
constexpr int MAX_FIELDS = 5;

constexpr int WIN_W = 640;
constexpr int WIN_H = 480;

constexpr int ID_BACK   = 101;
constexpr int ID_NEXT   = 102;
constexpr int ID_CANCEL = 103;
constexpr int ID_RESULT = 104;
constexpr int ID_TITLE  = 105;
constexpr int ID_FIELD  = 200;

struct FieldInfo {
    std::wstring key;
    std::wstring label;
    std::wstring defVal;
    bool isPass = false;
};

struct PanelConfig {
    std::wstring type;
    std::wstring title;
    std::vector<FieldInfo> fields;
};

struct RegistrationData {
    struct Item {
        int stepIndex;
        std::wstring title;
        std::wstring key;
        std::wstring label;
        std::wstring val;
        bool isPass;
    };

    std::vector<Item> items;
    std::map<std::wstring, std::wstring> values;

    void Set(int stepIndex, const std::wstring& title,
             const std::wstring& key, const std::wstring& label,
             const std::wstring& val, bool isPass = false) {
        values[key] = val;
        for (auto& item : items) {
            if (item.stepIndex == stepIndex && item.key == key) {
                item.val = val;
                item.title = title;
                item.label = label;
                item.isPass = isPass;
                return;
            }
        }
        items.push_back({ stepIndex, title, key, label, val, isPass });
    }

    std::wstring Get(const std::wstring& key, const std::wstring& defVal = L"") const {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : defVal;
    }

    std::wstring BuildReport() const {
        std::wostringstream ss;
        ss << L"=== ПІДСУМКОВІ ДАНІ РЕЄСТРАЦІЇ ===\r\n\r\n";
        int prev = -1;
        for (const auto& item : items) {
            if (item.stepIndex != prev) {
                prev = item.stepIndex;
                ss << L"[Крок " << (prev + 1) << L": " << item.title << L"]\r\n";
            }
            std::wstring str1 = item.isPass ? std::wstring(item.val.size(), L'*') : item.val;
            ss << L"  " << item.label << L": " << str1 << L"\r\n";
        }
        return ss.str();
    }
};
