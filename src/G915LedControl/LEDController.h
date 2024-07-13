#pragma once
#include <thread>
#include <atomic>
#include <vector>
#include "LogitechLEDLib.h"
#include "ConfigManager.h"

/**
 * @class LEDController
 * @brief Controls the LED lighting effects for Logitech peripherals.
 *
 * This class interfaces with the Logitech LED SDK to apply various
 * lighting effects to compatible devices. It handles different effect types
 * such as solid colors, pulsing effects, rainbow waves, and hue waves.
 * It also manages the state of lock key indicators.
 */
class LEDController {
public:
    LEDController();
    ~LEDController();
    bool Initialize();
    void Shutdown();
    void SetSolidColor(int red, int green, int blue);
    void StartPulseEffect(int red, int green, int blue, float duration, float minLight, float maxLight);
    void StartRainbowWave();
    void StartHueWave(int red, int green, int blue, float duration, float minLight, float maxLight);
    void StopEffects();
    void UpdateLockKeys();

    Effect GetCurrentEffect() const;
    void GetSolidColor(int& red, int& green, int& blue) const;
    void GetPulseSettings(int& red, int& green, int& blue, float& duration, float& minLight, float& maxLight) const;

private:
    std::atomic<bool> running;
    std::atomic<int> currentRed;
    std::atomic<int> currentGreen;
    std::atomic<int> currentBlue;
    std::atomic<Effect> currentEffect;
    std::atomic<float> pulseDuration;
    std::atomic<float> pulseMinLight;
    std::atomic<float> pulseMaxLight;
    std::atomic<bool> isEffectRunning;
    const std::vector<LogiLed::KeyName> lockKeys;

    void PulseKeys();
    void RainbowWave();
    void SetStaticLighting();
    void HSVtoRGB(float H, float S, float V, int& R, int& G, int& B);
    std::tuple<int, int, int> GetCurrentColor() const;
};