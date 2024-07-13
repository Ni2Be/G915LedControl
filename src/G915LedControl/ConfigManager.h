#pragma once
#include "stdafx.h"
#include <windows.h>
#include <string>

enum class Effect {
    Solid,
    Pulse,
    Rainbow,
    HueWave, 
    None
};

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();

    void Load();
    void Save();


    // Getters
    int GetSolidRed() const { return solidRed; }
    int GetSolidGreen() const { return solidGreen; }
    int GetSolidBlue() const { return solidBlue; }
    int GetPulseRed() const { return pulseRed; }
    int GetPulseGreen() const { return pulseGreen; }
    int GetPulseBlue() const { return pulseBlue; }
    float GetPulseDuration() const { return pulseDuration; }
    float GetPulseMinLight() const { return pulseMinLight; }
    float GetPulseMaxLight() const { return pulseMaxLight; }
    Effect GetEffect() const { return currentEffect; }

    // Setters
    void SetSolidColor(int red, int green, int blue);
    void SetPulseColor(int red, int green, int blue);
    void SetPulseDuration(float duration);
    void SetPulseMinLight(float min) { pulseMinLight = min; }
    void SetPulseMaxLight(float max) { pulseMaxLight = max; }
    void SetEffect(Effect effect) { currentEffect = effect; }

    // New methods for HueWave (if needed)
    void SetHueWaveColor(int red, int green, int blue);
    void SetHueWaveDuration(float duration);
    void SetHueWaveMinLight(float min);
    void SetHueWaveMaxLight(float max);
    int GetHueWaveRed() const { return hueWaveRed; }
    int GetHueWaveGreen() const { return hueWaveGreen; }
    int GetHueWaveBlue() const { return hueWaveBlue; }
    float GetHueWaveDuration() const { return hueWaveDuration; }
    float GetHueWaveMinLight() const { return hueWaveMinLight; }
    float GetHueWaveMaxLight() const { return hueWaveMaxLight; }

private:
    std::wstring GetConfigFilePath();
    void CreateConfigDirectory();

    int solidRed = 50, solidGreen = 80, solidBlue = 0;
    int pulseRed = 0, pulseGreen = 100, pulseBlue = 0;
    float pulseDuration = 2000.0f;
    float pulseMinLight = 20.0f, pulseMaxLight = 100.0f;
    int hueWaveRed = 0, hueWaveGreen = 0, hueWaveBlue = 100;
    float hueWaveDuration = 3000.0f;
    float hueWaveMinLight = 30.0f, hueWaveMaxLight = 100.0f;
    Effect currentEffect = Effect::Solid;
};