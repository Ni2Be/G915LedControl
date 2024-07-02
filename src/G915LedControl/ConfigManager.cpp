#include "stdafx.h"
#include "ConfigManager.h"
#include <ShlObj.h>
#include <fstream>

ConfigManager::ConfigManager() {
    Load();
}

ConfigManager::~ConfigManager() {
    Save();
}

std::wstring ConfigManager::GetConfigFilePath() {
    WCHAR path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::wstring configPath = path;
        configPath += L"\\LEDControl\\config.ini";
        return configPath;
    }
    return L"";
}

void ConfigManager::CreateConfigDirectory() {
    std::wstring configPath = GetConfigFilePath();
    size_t pos = configPath.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        std::wstring dirPath = configPath.substr(0, pos);
        CreateDirectoryW(dirPath.c_str(), NULL);
    }
}

void ConfigManager::Load() {
    std::wstring configPath = GetConfigFilePath();
    WCHAR buffer[256];

    GetPrivateProfileIntW(L"SolidColor", L"Red", solidRed, configPath.c_str());
    GetPrivateProfileIntW(L"SolidColor", L"Green", solidGreen, configPath.c_str());
    GetPrivateProfileIntW(L"SolidColor", L"Blue", solidBlue, configPath.c_str());

    GetPrivateProfileIntW(L"PulseColor", L"Red", pulseRed, configPath.c_str());
    GetPrivateProfileIntW(L"PulseColor", L"Green", pulseGreen, configPath.c_str());
    GetPrivateProfileIntW(L"PulseColor", L"Blue", pulseBlue, configPath.c_str());

    if (GetPrivateProfileStringW(L"PulseEffect", L"Duration", L"2000.0", buffer, 256, configPath.c_str())) {
        pulseDuration = static_cast<float>(_wtof(buffer));
    }

    currentEffect = static_cast<Effect>(GetPrivateProfileIntW(L"ActiveEffect", L"Type", (int)currentEffect, configPath.c_str()));
}

void ConfigManager::Save() {
    CreateConfigDirectory();
    std::wstring configPath = GetConfigFilePath();

    WCHAR buffer[256];

    /*
    
    int solidRed = 100, solidGreen = 0, solidBlue = 0;
    int pulseRed = 0, pulseGreen = 100, pulseBlue = 0;
    float pulseDuration = 2000.0f;
    Effect currentEffect = Effect::Solid;
    */

    WritePrivateProfileStringW(L"SolidColor", L"Red", std::to_wstring(solidRed).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"SolidColor", L"Green", std::to_wstring(solidGreen).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"SolidColor", L"Blue", std::to_wstring(solidBlue).c_str(), configPath.c_str());

    WritePrivateProfileStringW(L"PulseColor", L"Red", std::to_wstring(pulseRed).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"PulseColor", L"Green", std::to_wstring(pulseGreen).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"PulseColor", L"Blue", std::to_wstring(pulseBlue).c_str(), configPath.c_str());

    swprintf_s(buffer, L"%.1f", pulseDuration);
    WritePrivateProfileStringW(L"PulseEffect", L"Duration", buffer, configPath.c_str());

    WritePrivateProfileStringW(L"ActiveEffect", L"Type", std::to_wstring(static_cast<int>(currentEffect)).c_str(), configPath.c_str());
}

void ConfigManager::SetSolidColor(int red, int green, int blue) {
    solidRed = red;
    solidGreen = green;
    solidBlue = blue;
}

void ConfigManager::SetPulseColor(int red, int green, int blue) {
    pulseRed = red;
    pulseGreen = green;
    pulseBlue = blue;
}

void ConfigManager::SetPulseDuration(float duration) {
    pulseDuration = duration;
}

