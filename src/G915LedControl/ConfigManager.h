#pragma once
#include "stdafx.h"
#include <windows.h>
#include <string>

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    void Load();
    void Save();

    enum class Effect {
		Solid,
		Pulse,
        Rainbow,
	};

    // Getters
    int GetSolidRed() const { return solidRed; }
    int GetSolidGreen() const { return solidGreen; }
    int GetSolidBlue() const { return solidBlue; }
    int GetPulseRed() const { return pulseRed; }
    int GetPulseGreen() const { return pulseGreen; }
    int GetPulseBlue() const { return pulseBlue; }
    float GetPulseDuration() const { return pulseDuration; }
    Effect GetEffect() const { return currentEffect; }

    // Setters
    void SetSolidColor(int red, int green, int blue);
    void SetPulseColor(int red, int green, int blue);
    void SetPulseDuration(float duration);
    void SetEffect(Effect effect) { currentEffect = effect; }
private:
    std::wstring GetConfigFilePath();
    void CreateConfigDirectory();

    int solidRed = 100, solidGreen = 0, solidBlue = 0;
    int pulseRed = 0, pulseGreen = 100, pulseBlue = 0;
    float pulseDuration = 2000.0f;
    Effect currentEffect = Effect::Solid;
};