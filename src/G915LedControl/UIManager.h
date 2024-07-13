#pragma once
#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include "ConfigManager.h"
#include "LEDController.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

#ifndef TBM_GETPOS
#define TBM_GETPOS (WM_USER)
#endif

#ifndef TBM_SETPOS
#define TBM_SETPOS (WM_USER+5)
#endif

#ifndef TBM_SETRANGE
#define TBM_SETRANGE (WM_USER+6)
#endif

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_EXIT 1001
#define ID_SOLID 1002
#define ID_PULSE 1003
#define ID_RAINBOW 1004
#define ID_HUE_WAVE 1005
#define ID_CONFIG 1006

/**
 * @class UIManager
 * @brief Manages the user interface for the LED control application.
 *
 * This class is responsible for creating and managing the application's
 * system tray icon, context menu, and configuration dialog. It handles
 * user interactions, updates the UI based on current settings, and
 * communicates with the ConfigManager and LEDController to apply
 * user-selected effects.
 */
class UIManager {
public:
    UIManager(HINSTANCE hInstance, ConfigManager& configManager, LEDController& ledController);
    ~UIManager();
    bool Initialize();
    void Run();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

    void ShowConfigDialog(HWND parentHwnd) {
        DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG_COMBINED), parentHwnd,
            ConfigDlgProc, reinterpret_cast<LPARAM>(this));
    }

    void CreateTrayIcon();
    void CreateTrayMenu();
    void ShowEffectControls(HWND hwnd, Effect effect);
    void UpdateStaticText(HWND hwnd);
    void PreviewEffect(HWND hwnd, Effect effect);
    void SaveSettings(HWND hwnd, Effect effect);

    HINSTANCE m_hInstance;
    HWND m_hwnd;
    HMENU m_hMenu;
    NOTIFYICONDATA m_nid;
    ConfigManager& m_configManager;
    LEDController& m_ledController;

    // Current settings
    int m_solidRed, m_solidGreen, m_solidBlue;
    int m_pulseRed, m_pulseGreen, m_pulseBlue;
    float m_pulseDuration, m_pulseMinLight, m_pulseMaxLight;
    Effect m_currentEffect;
};
