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

    // Solid Color
    solidRed = GetPrivateProfileIntW(L"SolidColor", L"Red", solidRed, configPath.c_str());
    solidGreen = GetPrivateProfileIntW(L"SolidColor", L"Green", solidGreen, configPath.c_str());
    solidBlue = GetPrivateProfileIntW(L"SolidColor", L"Blue", solidBlue, configPath.c_str());

    // Pulse Effect
    pulseRed = GetPrivateProfileIntW(L"PulseColor", L"Red", pulseRed, configPath.c_str());
    pulseGreen = GetPrivateProfileIntW(L"PulseColor", L"Green", pulseGreen, configPath.c_str());
    pulseBlue = GetPrivateProfileIntW(L"PulseColor", L"Blue", pulseBlue, configPath.c_str());
    if (GetPrivateProfileStringW(L"PulseEffect", L"Duration", L"2000.0", buffer, 256, configPath.c_str())) {
        pulseDuration = static_cast<float>(_wtof(buffer));
    }
    if (GetPrivateProfileStringW(L"PulseEffect", L"MinLight", L"20.0", buffer, 256, configPath.c_str())) {
        pulseMinLight = static_cast<float>(_wtof(buffer));
    }
    if (GetPrivateProfileStringW(L"PulseEffect", L"MaxLight", L"100.0", buffer, 256, configPath.c_str())) {
        pulseMaxLight = static_cast<float>(_wtof(buffer));
    }

    // HueWave Effect
    hueWaveRed = GetPrivateProfileIntW(L"HueWaveColor", L"Red", hueWaveRed, configPath.c_str());
    hueWaveGreen = GetPrivateProfileIntW(L"HueWaveColor", L"Green", hueWaveGreen, configPath.c_str());
    hueWaveBlue = GetPrivateProfileIntW(L"HueWaveColor", L"Blue", hueWaveBlue, configPath.c_str());
    if (GetPrivateProfileStringW(L"HueWaveEffect", L"Duration", L"3000.0", buffer, 256, configPath.c_str())) {
        hueWaveDuration = static_cast<float>(_wtof(buffer));
    }
    if (GetPrivateProfileStringW(L"HueWaveEffect", L"MinLight", L"30.0", buffer, 256, configPath.c_str())) {
        hueWaveMinLight = static_cast<float>(_wtof(buffer));
    }
    if (GetPrivateProfileStringW(L"HueWaveEffect", L"MaxLight", L"100.0", buffer, 256, configPath.c_str())) {
        hueWaveMaxLight = static_cast<float>(_wtof(buffer));
    }

    currentEffect = static_cast<Effect>(GetPrivateProfileIntW(L"ActiveEffect", L"Type", static_cast<int>(currentEffect), configPath.c_str()));
}

void ConfigManager::Save() {
    CreateConfigDirectory();
    std::wstring configPath = GetConfigFilePath();
    WCHAR buffer[256];

    // Solid Color
    WritePrivateProfileStringW(L"SolidColor", L"Red", std::to_wstring(solidRed).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"SolidColor", L"Green", std::to_wstring(solidGreen).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"SolidColor", L"Blue", std::to_wstring(solidBlue).c_str(), configPath.c_str());

    // Pulse Effect
    WritePrivateProfileStringW(L"PulseColor", L"Red", std::to_wstring(pulseRed).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"PulseColor", L"Green", std::to_wstring(pulseGreen).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"PulseColor", L"Blue", std::to_wstring(pulseBlue).c_str(), configPath.c_str());
    swprintf_s(buffer, L"%.1f", pulseDuration);
    WritePrivateProfileStringW(L"PulseEffect", L"Duration", buffer, configPath.c_str());
    swprintf_s(buffer, L"%.1f", pulseMinLight);
    WritePrivateProfileStringW(L"PulseEffect", L"MinLight", buffer, configPath.c_str());
    swprintf_s(buffer, L"%.1f", pulseMaxLight);
    WritePrivateProfileStringW(L"PulseEffect", L"MaxLight", buffer, configPath.c_str());

    // HueWave Effect
    WritePrivateProfileStringW(L"HueWaveColor", L"Red", std::to_wstring(hueWaveRed).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"HueWaveColor", L"Green", std::to_wstring(hueWaveGreen).c_str(), configPath.c_str());
    WritePrivateProfileStringW(L"HueWaveColor", L"Blue", std::to_wstring(hueWaveBlue).c_str(), configPath.c_str());
    swprintf_s(buffer, L"%.1f", hueWaveDuration);
    WritePrivateProfileStringW(L"HueWaveEffect", L"Duration", buffer, configPath.c_str());
    swprintf_s(buffer, L"%.1f", hueWaveMinLight);
    WritePrivateProfileStringW(L"HueWaveEffect", L"MinLight", buffer, configPath.c_str());
    swprintf_s(buffer, L"%.1f", hueWaveMaxLight);
    WritePrivateProfileStringW(L"HueWaveEffect", L"MaxLight", buffer, configPath.c_str());

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

void ConfigManager::SetHueWaveColor(int red, int green, int blue) {
    hueWaveRed = red;
    hueWaveGreen = green;
    hueWaveBlue = blue;
}

void ConfigManager::SetHueWaveDuration(float duration) {
    hueWaveDuration = duration;
}

void ConfigManager::SetHueWaveMinLight(float min) {
    hueWaveMinLight = min;
}

void ConfigManager::SetHueWaveMaxLight(float max) {
    hueWaveMaxLight = max;
}