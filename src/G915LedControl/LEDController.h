#pragma once
#include "LogitechLEDLib.h"
#include <atomic>
#include <vector>

class LEDController {
public:
    LEDController();
    ~LEDController();

    bool Initialize();
    void Shutdown();

    void SetSolidColor(int red, int green, int blue);
    void StartPulseEffect(int red, int green, int blue, float duration);
    void StartRainbowWave();
    void StopEffects();

    void UpdateLockKeys();

private:
    std::atomic<bool> running;
    std::atomic<int> currentRed;
    std::atomic<int> currentGreen;
    std::atomic<int> currentBlue;
    std::atomic<bool> isPulsing;
    std::atomic<bool> isRainbowWave;
    std::atomic<float> pulseDuration;

    const std::vector<LogiLed::KeyName> lockKeys;

    void PulseKeys();
    void RainbowWave();
    void SetStaticLighting();
    void HSVtoRGB(float H, float S, float V, int& R, int& G, int& B);
};
