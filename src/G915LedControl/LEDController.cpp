#include "stdafx.h"
#include "LEDController.h"
#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>

LEDController::LEDController()
    : running(true), currentRed(0), currentGreen(0), currentBlue(0),
    isPulsing(false), isRainbowWave(false), isHueWave(false), pulseDuration(2000.0f),
    pulseMinLight(0.0f), pulseMaxLight(100.0f),
    lockKeys({ LogiLed::KeyName::NUM_LOCK, LogiLed::KeyName::CAPS_LOCK, LogiLed::KeyName::SCROLL_LOCK }) {}

LEDController::~LEDController() {
    Shutdown();
}

bool LEDController::Initialize() {
    return LogiLedInitWithName("Logitech LED Control");
}

void LEDController::Shutdown() {
    running = false;
    LogiLedShutdown();
}

void LEDController::SetSolidColor(int red, int green, int blue) {
    StopEffects();
    currentRed = red;
    currentGreen = green;
    currentBlue = blue;

    // Make sure the values are within the correct range (0-100)
    red = max(0, min(100, red));
    green = max(0, min(100, green));
    blue = max(0, min(100, blue));

    // Convert from 0-100 range to 0-255 range
    int r = static_cast<int>(red * 2.55f);
    int g = static_cast<int>(green * 2.55f);
    int b = static_cast<int>(blue * 2.55f);

    // Set the color using the Logitech SDK
    LogiLedSetLighting(r, g, b);
}

void LEDController::StartPulseEffect(int red, int green, int blue, float duration, float minLight, float maxLight) {
    // Ensure values are within correct range
    red = max(0, min(100, red));
    green = max(0, min(100, green));
    blue = max(0, min(100, blue));
    duration = max(500.0f, min(5000.0f, duration));
    minLight = max(0.0f, min(100.0f, minLight));
    maxLight = max(0.0f, min(100.0f, maxLight));

    StopEffects();

    currentRed = red;
    currentGreen = green;
    currentBlue = blue;
    pulseDuration = duration;
    pulseMinLight = minLight;
    pulseMaxLight = maxLight;
    isPulsing = true;
    isHueWave = false;
    isEffectRunning = true;

    std::thread(&LEDController::PulseKeys, this).detach();
}

void LEDController::StartHueWave(int red, int green, int blue, float duration, float minLight, float maxLight) {
    // Ensure values are within correct range
    red = max(0, min(100, red));
    green = max(0, min(100, green));
    blue = max(0, min(100, blue));
    duration = max(500.0f, min(5000.0f, duration));
    minLight = max(0.0f, min(100.0f, minLight));
    maxLight = max(0.0f, min(100.0f, maxLight));

    StopEffects();

    currentRed = red;
    currentGreen = green;
    currentBlue = blue;
    pulseDuration = duration;
    pulseMinLight = minLight;
    pulseMaxLight = maxLight;
    isPulsing = true;
    isHueWave = true;
    isEffectRunning = true;

    std::thread(&LEDController::PulseKeys, this).detach();
}

void LEDController::StopEffects() {
    isPulsing = false;
    isRainbowWave = false;
    isHueWave = false;
    isEffectRunning = false;
    // Give the effect thread time to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


void LEDController::StartRainbowWave() {
    StopEffects();
    isRainbowWave = true;
    std::thread(&LEDController::RainbowWave, this).detach();
}

void LEDController::UpdateLockKeys() {
    while (running) {
        bool numLock = (GetKeyState(VK_NUMLOCK) & 1) != 0;
        bool capsLock = (GetKeyState(VK_CAPITAL) & 1) != 0;
        bool scrollLock = (GetKeyState(VK_SCROLL) & 1) != 0;

        LogiLedSetLightingForKeyWithKeyName(LogiLed::KeyName::NUM_LOCK, numLock ? 0 : 0, numLock ? 100 : 0, numLock ? 100 : 0);
        LogiLedSetLightingForKeyWithKeyName(LogiLed::KeyName::CAPS_LOCK, capsLock ? 0 : 0, capsLock ? 100 : 0, capsLock ? 100 : 0);
        LogiLedSetLightingForKeyWithKeyName(LogiLed::KeyName::SCROLL_LOCK, scrollLock ? 0 : 0, scrollLock ? 100 : 0, scrollLock ? 100 : 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
}

void LEDController::PulseKeys() {
    auto startTime = std::chrono::high_resolution_clock::now();
    const float hueWaveDuration = 10000.0f;  // Duration of one complete hue cycle in ms

    while (isEffectRunning) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float, std::milli>(currentTime - startTime).count();

        int r_this_frame, g_this_frame, b_this_frame;
        if (isHueWave) {
            float hue = fmodf(elapsedTime / hueWaveDuration * 360.0f, 360.0f);
            HSVtoRGB(hue, 100, 100, r_this_frame, g_this_frame, b_this_frame);
        }
        else {
            r_this_frame = currentRed * 2.5;
            g_this_frame = currentGreen * 2.5;
            b_this_frame = currentBlue * 2.5;
        }

        float t = fmodf(elapsedTime, pulseDuration.load()) / pulseDuration.load();
        float intensity = (std::cos(t * 2 * 3.14159f) + 1) / 2;
        intensity = pulseMinLight / 100.0f + intensity * (pulseMaxLight - pulseMinLight) / 100.0f;

        unsigned char bitmap[LOGI_LED_BITMAP_SIZE] = { 0 };
        int r = static_cast<int>(r_this_frame * intensity);
        int g = static_cast<int>(g_this_frame * intensity);
        int b = static_cast<int>(b_this_frame * intensity);

        // Populate the bitmap and update lighting
        for (int i = 0; i < LOGI_LED_BITMAP_WIDTH * LOGI_LED_BITMAP_HEIGHT; ++i) {
            int index = i * LOGI_LED_BITMAP_BYTES_PER_KEY;
            bitmap[index] = r;
            bitmap[index + 1] = g;
            bitmap[index + 2] = b;
            bitmap[index + 3] = 255;
        }

        LogiLedSetLightingFromBitmap(bitmap);
        LogiLedExcludeKeysFromBitmap(const_cast<LogiLed::KeyName*>(lockKeys.data()), lockKeys.size());

        std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30 fps
    }
}

void LEDController::SetStaticLighting() {
    unsigned char bitmap[LOGI_LED_BITMAP_SIZE] = { 0 };

    int r = static_cast<int>(currentRed * 2.55f);
    int g = static_cast<int>(currentGreen * 2.55f);
    int b = static_cast<int>(currentBlue * 2.55f);

    for (int i = 0; i < LOGI_LED_BITMAP_WIDTH * LOGI_LED_BITMAP_HEIGHT; ++i) {
        int index = i * LOGI_LED_BITMAP_BYTES_PER_KEY;
        bitmap[index] = r;
        bitmap[index + 1] = g;
        bitmap[index + 2] = b;
        bitmap[index + 3] = 255;  // Full brightness
    }

    LogiLedSetLightingFromBitmap(bitmap);
    LogiLedExcludeKeysFromBitmap(const_cast<LogiLed::KeyName*>(lockKeys.data()), lockKeys.size());
}


void LEDController::HSVtoRGB(float H, float S, float V, int& R, int& G, int& B) {
    if (H > 360 || H < 0 || S>100 || S < 0 || V>100 || V < 0) {
        return;
    }
    float s = S / 100;
    float v = V / 100;
    float C = s * v;
    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
    float m = v - C;
    float r, g, b;
    if (H >= 0 && H < 60) {
        r = C, g = X, b = 0;
    }
    else if (H >= 60 && H < 120) {
        r = X, g = C, b = 0;
    }
    else if (H >= 120 && H < 180) {
        r = 0, g = C, b = X;
    }
    else if (H >= 180 && H < 240) {
        r = 0, g = X, b = C;
    }
    else if (H >= 240 && H < 300) {
        r = X, g = 0, b = C;
    }
    else {
        r = C, g = 0, b = X;
    }
    R = (r + m) * 255;
    G = (g + m) * 255;
    B = (b + m) * 255;
}

void LEDController::RainbowWave() {
    const int width = LOGI_LED_BITMAP_WIDTH;
    const float waveDuration = 5000.0f;  // Duration of one complete wave cycle in ms
    const float waveWidth = 360.0f;  // Width of the wave in degrees

    auto startTime = std::chrono::high_resolution_clock::now();

    while (running && isRainbowWave) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float elapsedTime = std::chrono::duration<float, std::milli>(currentTime - startTime).count();

        unsigned char bitmap[LOGI_LED_BITMAP_SIZE] = { 0 };

        for (int x = 0; x < width; x++) {
            float hue = fmodf(((float)x / width * waveWidth) - (elapsedTime / waveDuration * waveWidth), 360.0f);
            if (hue < 0) hue += 360.0f;

            int r, g, b;
            HSVtoRGB(hue, 100, 100, r, g, b);

            for (int y = 0; y < LOGI_LED_BITMAP_HEIGHT; y++) {
                int index = (y * LOGI_LED_BITMAP_WIDTH + x) * LOGI_LED_BITMAP_BYTES_PER_KEY;
                bitmap[index] = r;
                bitmap[index + 1] = g;
                bitmap[index + 2] = b;
                bitmap[index + 3] = 255;  // Full brightness
            }
        }

        LogiLedSetLightingFromBitmap(bitmap);
        LogiLedExcludeKeysFromBitmap(const_cast<LogiLed::KeyName*>(lockKeys.data()), lockKeys.size());

        std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30 fps
    }
}