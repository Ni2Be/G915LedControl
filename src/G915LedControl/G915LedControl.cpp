
#include "stdafx.h"
#include "LogitechLEDLib.h"
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <cmath>
#include "LEDController.h"
#include <commctrl.h>
#include <string>
#include "resource.h"
#include "ConfigManager.h"

#pragma comment(lib, "comctl32.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_EXIT 1001
#define ID_SOLID 1002
#define ID_PULSE 1003
#define ID_RAINBOW 1004
#define ID_CONFIG_SOLID 1005
#define ID_CONFIG_PULSE 1006
#define ID_HUE_WAVE 1007

LEDController ledController;
NOTIFYICONDATA nid = {};
HMENU hMenu;

// Global variables to store current configuration
int solidRed = 100, solidGreen = 0, solidBlue = 0;
int pulseRed = 0, pulseGreen = 100, pulseBlue = 0;
float pulseDuration = 2000.0f;
float pulseMinLight = 20.0f, pulseMaxLight = 100.0f;
ConfigManager::Effect currentEffect = ConfigManager::Effect::Solid;
ConfigManager configManager;

// Function prototypes
INT_PTR CALLBACK ConfigSolidDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfigPulseDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
        }
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_EXIT:
            DestroyWindow(hwnd);
            return 0;

        case ID_SOLID:
            ledController.SetSolidColor(solidRed, solidGreen, solidBlue);
            configManager.SetEffect(ConfigManager::Effect::Solid);
            return 0;

        case ID_PULSE:
            ledController.StartPulseEffect(pulseRed, pulseGreen, pulseBlue, pulseDuration, pulseMinLight, pulseMaxLight);
            configManager.SetEffect(ConfigManager::Effect::Pulse);
            return 0;

        case ID_RAINBOW:
            ledController.StartRainbowWave();
            configManager.SetEffect(ConfigManager::Effect::Rainbow);
            return 0;

        case ID_HUE_WAVE:
            ledController.StartHueWave(pulseRed, pulseGreen, pulseBlue, pulseDuration, pulseMinLight, pulseMaxLight);
            configManager.SetEffect(ConfigManager::Effect::HueWave);
            return 0;

        case ID_CONFIG_SOLID:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_SOLID), hwnd, ConfigSolidDlgProc);
            return 0;

        case ID_CONFIG_PULSE:
            DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_PULSE), hwnd, ConfigPulseDlgProc);
            return 0;
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
INT_PTR CALLBACK ConfigSolidDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_INITDIALOG:
        // Initialize sliders
        SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        // Set initial positions
        SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETPOS, TRUE, solidRed);
        SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETPOS, TRUE, solidGreen);
        SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETPOS, TRUE, solidBlue);
        // Update static text
        SetDlgItemInt(hwnd, IDC_STATIC_RED, solidRed, FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_GREEN, solidGreen, FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_BLUE, solidBlue, FALSE);
        return (INT_PTR)TRUE;

    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_RED) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_GREEN) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_BLUE))
        {
            int redVal = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
            int greenVal = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
            int blueVal = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
            SetDlgItemInt(hwnd, IDC_STATIC_RED, redVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_GREEN, greenVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_BLUE, blueVal, FALSE);
            // Preview color change
            ledController.SetSolidColor(redVal, greenVal, blueVal);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            solidRed = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
            solidGreen = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
            solidBlue = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
            configManager.SetSolidColor(solidRed, solidGreen, solidBlue);
            configManager.Save();
            ledController.SetSolidColor(solidRed, solidGreen, solidBlue);
            EndDialog(hwnd, IDOK);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            // Revert to original color
            ledController.SetSolidColor(solidRed, solidGreen, solidBlue);
            EndDialog(hwnd, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ConfigPulseDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_INITDIALOG:
        // Initialize sliders
        SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_SETRANGE, TRUE, MAKELPARAM(500, 5000));
        SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
        SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));

        // Set initial positions
        SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETPOS, TRUE, pulseRed);
        SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETPOS, TRUE, pulseGreen);
        SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETPOS, TRUE, pulseBlue);
        SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pulseDuration)));
        SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pulseMinLight)));
        SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pulseMaxLight)));

        // Update static text
        SetDlgItemInt(hwnd, IDC_STATIC_RED, pulseRed, FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_GREEN, pulseGreen, FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_BLUE, pulseBlue, FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_DURATION, static_cast<UINT>(pulseDuration), FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_MIN_LIGHT, static_cast<UINT>(pulseMinLight), FALSE);
        SetDlgItemInt(hwnd, IDC_STATIC_MAX_LIGHT, static_cast<UINT>(pulseMaxLight), FALSE);

        return (INT_PTR)TRUE;

    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_RED) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_GREEN) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_BLUE) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_DURATION) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_MIN_LIGHT) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_MAX_LIGHT))
        {
            int redVal = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
            int greenVal = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
            int blueVal = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
            int durationVal = SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_GETPOS, 0, 0);
            int minLightVal = SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_GETPOS, 0, 0);
            int maxLightVal = SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_GETPOS, 0, 0);
            SetDlgItemInt(hwnd, IDC_STATIC_RED, redVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_GREEN, greenVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_BLUE, blueVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_DURATION, durationVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_MIN_LIGHT, minLightVal, FALSE);
            SetDlgItemInt(hwnd, IDC_STATIC_MAX_LIGHT, maxLightVal, FALSE);

            ledController.StartPulseEffect(redVal, greenVal, blueVal, durationVal, minLightVal, maxLightVal);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            pulseRed = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
            pulseGreen = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
            pulseBlue = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
            pulseDuration = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_GETPOS, 0, 0));
            pulseMinLight = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_GETPOS, 0, 0));
            pulseMaxLight = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_GETPOS, 0, 0));

            configManager.SetPulseColor(pulseRed, pulseGreen, pulseBlue);
            configManager.SetPulseDuration(pulseDuration);
            configManager.SetPulseMinLight(pulseMinLight);
            configManager.SetPulseMaxLight(pulseMaxLight);
            configManager.Save();
            ledController.StartPulseEffect(pulseRed, pulseGreen, pulseBlue, pulseDuration, pulseMinLight, pulseMaxLight);
            EndDialog(hwnd, IDOK);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Wait for 15 seconds otherwise the program will not start if in autostart folder
    Sleep(15000);

    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    // Test if windows service LGHUBUpdaterService is running
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (schSCManager == NULL)
    {
        MessageBox(NULL, L"Failed to open service control manager.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    SC_HANDLE schService = OpenService(schSCManager, L"LGHUBUpdaterService", SERVICE_QUERY_STATUS);
    if (schService == NULL)
    {
        MessageBox(NULL, L"Failed to open LGHUBUpdaterService.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    // Wait up to 5 minutes for the service to start
    SERVICE_STATUS_PROCESS serviceStatus;
    DWORD dwBytesNeeded;
    DWORD dwTimeout = 300000; // 5 minutes
    DWORD dwStartTime = GetTickCount();
    while (1)
    {
        if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)&serviceStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
        {
            MessageBox(NULL, L"Failed to query service status.", L"Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        if (serviceStatus.dwCurrentState == SERVICE_RUNNING)
        {
            break;
        }

        if (GetTickCount() - dwStartTime > dwTimeout)
        {
            MessageBox(NULL, L"Timed out waiting for LGHUBUpdaterService to start.", L"Error", MB_OK | MB_ICONERROR);
            return 0;
        }

        Sleep(serviceStatus.dwWaitHint);
    }

    bool isInitialized = false;
    // Try 5 minutes to initialize LED controller
    for (int i = 0; i < 300; i++)
    {
        isInitialized = ledController.Initialize();
        if (isInitialized)
        {
            break;
        }

        Sleep(1000);
    }

    if (!isInitialized)
    {
        // Failed to initialize LED controller
        // Fail silently to not annoy the user
        return 0;
    }

    // Load configuration
    configManager.Load();

    // Initialize LED controller with loaded settings
    solidRed = configManager.GetSolidRed();
    solidGreen = configManager.GetSolidGreen();
    solidBlue = configManager.GetSolidBlue();
    pulseRed = configManager.GetPulseRed();
    pulseGreen = configManager.GetPulseGreen();
    pulseBlue = configManager.GetPulseBlue();
    pulseDuration = configManager.GetPulseDuration();
    pulseMinLight = configManager.GetPulseMinLight();
    pulseMaxLight = configManager.GetPulseMaxLight();
    currentEffect = configManager.GetEffect();

    // First set all keys to solid color
    ledController.SetSolidColor(solidRed, solidGreen, solidBlue);

    switch (currentEffect)
    {
    case ConfigManager::Effect::Solid:
        ledController.SetSolidColor(solidRed, solidGreen, solidBlue);
        break;
    case ConfigManager::Effect::Pulse:
        ledController.StartPulseEffect(pulseRed, pulseGreen, pulseBlue, pulseDuration, pulseMinLight, pulseMaxLight);
        break;
    case ConfigManager::Effect::Rainbow:
        ledController.StartRainbowWave();
        break;
    case ConfigManager::Effect::HueWave:
        ledController.StartHueWave(pulseRed, pulseGreen, pulseBlue, pulseDuration, pulseMinLight, pulseMaxLight);
        break;
    default:
        break;
    }

    // Create a hidden window
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LEDControlTrayApp";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"LEDControlTrayApp", L"LED Control",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, L"Window creation failed.", L"Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    // Create tray icon
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    // Load the custom icon
    HICON hIcon = (HICON)LoadImage(hInstance,
        MAKEINTRESOURCE(IDI_TRAYICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);

    if (hIcon)
    {
        nid.hIcon = hIcon;
    }
    else
    {
        // Fallback to default icon if custom icon fails to load
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    wcscpy_s(nid.szTip, L"LED Control");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Create tray menu
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, ID_SOLID, L"Set Solid Color");
    AppendMenu(hMenu, MF_STRING, ID_CONFIG_SOLID, L"Configure Solid Color");
    AppendMenu(hMenu, MF_STRING, ID_PULSE, L"Set Pulse Effect");
    AppendMenu(hMenu, MF_STRING, ID_CONFIG_PULSE, L"Configure Pulse Effect");
    AppendMenu(hMenu, MF_STRING, ID_RAINBOW, L"Set Rainbow Wave");
    AppendMenu(hMenu, MF_STRING, ID_HUE_WAVE, L"Set Hue Wave");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_EXIT, L"Exit");

    // Start the lock keys update thread
    std::thread(&LEDController::UpdateLockKeys, &ledController).detach();

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ledController.Shutdown();

    return 0;
}