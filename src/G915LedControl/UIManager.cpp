
#include "stdafx.h"
#include "UIManager.h"
#include <windowsx.h>
#include <strsafe.h>


UIManager::UIManager(HINSTANCE hInstance, ConfigManager& configManager, LEDController& ledController)
    : m_hInstance(hInstance), m_configManager(configManager), m_ledController(ledController)
{
    m_solidRed = configManager.GetSolidRed();
    m_solidGreen = configManager.GetSolidGreen();
    m_solidBlue = configManager.GetSolidBlue();
    m_pulseRed = configManager.GetPulseRed();
    m_pulseGreen = configManager.GetPulseGreen();
    m_pulseBlue = configManager.GetPulseBlue();
    m_pulseDuration = configManager.GetPulseDuration();
    m_pulseMinLight = configManager.GetPulseMinLight();
    m_pulseMaxLight = configManager.GetPulseMaxLight();
    m_currentEffect = configManager.GetEffect();
}

UIManager::~UIManager()
{
    Shell_NotifyIcon(NIM_DELETE, &m_nid);
}

bool UIManager::Initialize()
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = L"LEDControlTrayApp";
    RegisterClass(&wc);

    m_hwnd = CreateWindowEx(0, L"LEDControlTrayApp", L"LED Control",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, m_hInstance, NULL);

    if (m_hwnd == NULL)
    {
        MessageBox(NULL, L"Window creation failed.", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }

    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    CreateTrayIcon();
    CreateTrayMenu();

    // Set initial effect
    switch (m_currentEffect)
    {
    case Effect::Solid:
        m_ledController.SetSolidColor(m_solidRed, m_solidGreen, m_solidBlue);
        break;
    case Effect::Pulse:
        m_ledController.StartPulseEffect(m_pulseRed, m_pulseGreen, m_pulseBlue, m_pulseDuration, m_pulseMinLight, m_pulseMaxLight);
        break;
    case Effect::Rainbow:
        m_ledController.StartRainbowWave();
        break;
    case Effect::HueWave:
        m_ledController.StartHueWave(m_pulseRed, m_pulseGreen, m_pulseBlue, m_pulseDuration, m_pulseMinLight, m_pulseMaxLight);
        break;
    }

    return true;
}

void UIManager::Run()
{
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void UIManager::CreateTrayIcon()
{
    m_nid.cbSize = sizeof(NOTIFYICONDATA);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = ID_TRAYICON;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;

    HICON hIcon = (HICON)LoadImage(m_hInstance,
        MAKEINTRESOURCE(IDI_TRAYICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    if (hIcon)
    {
        m_nid.hIcon = hIcon;
    }
    else
    {
        m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    wcscpy_s(m_nid.szTip, L"LED Control");
    Shell_NotifyIcon(NIM_ADD, &m_nid);
}

void UIManager::CreateTrayMenu()
{
    m_hMenu = CreatePopupMenu();
    AppendMenu(m_hMenu, MF_STRING, ID_SOLID, L"Solid Color");
    AppendMenu(m_hMenu, MF_STRING, ID_PULSE, L"Pulse Effect");
    AppendMenu(m_hMenu, MF_STRING, ID_RAINBOW, L"Rainbow Wave");
    AppendMenu(m_hMenu, MF_STRING, ID_HUE_WAVE, L"Hue Wave");
    AppendMenu(m_hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(m_hMenu, MF_STRING, ID_CONFIG, L"Configure Effect");
    AppendMenu(m_hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(m_hMenu, MF_STRING, ID_EXIT, L"Exit");
}

LRESULT CALLBACK UIManager::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UIManager* pThis = reinterpret_cast<UIManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (pThis)
    {
        switch (uMsg)
        {
        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP)
            {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(pThis->m_hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
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
                pThis->m_ledController.SetSolidColor(pThis->m_solidRed, pThis->m_solidGreen, pThis->m_solidBlue);
                pThis->m_configManager.SetEffect(Effect::Solid);
                return 0;
            case ID_PULSE:
                pThis->m_ledController.StartPulseEffect(pThis->m_pulseRed, pThis->m_pulseGreen, pThis->m_pulseBlue, pThis->m_pulseDuration, pThis->m_pulseMinLight, pThis->m_pulseMaxLight);
                pThis->m_configManager.SetEffect(Effect::Pulse);
                return 0;
            case ID_RAINBOW:
                pThis->m_ledController.StartRainbowWave();
                pThis->m_configManager.SetEffect(Effect::Rainbow);
                return 0;
            case ID_HUE_WAVE:
                pThis->m_ledController.StartHueWave(pThis->m_pulseRed, pThis->m_pulseGreen, pThis->m_pulseBlue, pThis->m_pulseDuration, pThis->m_pulseMinLight, pThis->m_pulseMaxLight);
                pThis->m_configManager.SetEffect(Effect::HueWave);
                return 0;
            case ID_CONFIG:
                if (pThis) {
                    pThis->ShowConfigDialog(hwnd);
                }
                return 0;
        }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Helper functions

void UIManager::UpdateStaticText(HWND hwnd)
{
    SetDlgItemInt(hwnd, IDC_STATIC_RED, SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0), FALSE);
    SetDlgItemInt(hwnd, IDC_STATIC_GREEN, SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0), FALSE);
    SetDlgItemInt(hwnd, IDC_STATIC_BLUE, SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0), FALSE);
    SetDlgItemInt(hwnd, IDC_STATIC_DURATION, SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_GETPOS, 0, 0), FALSE);
    SetDlgItemInt(hwnd, IDC_STATIC_MIN_LIGHT, SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_GETPOS, 0, 0), FALSE);
    SetDlgItemInt(hwnd, IDC_STATIC_MAX_LIGHT, SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_GETPOS, 0, 0), FALSE);
}

void UIManager::ShowEffectControls(HWND hwnd, Effect effect)
{
    // Show/hide controls based on the selected effect
    BOOL showColorControls = (effect != Effect::Rainbow);
    BOOL showPulseControls = (effect == Effect::Pulse || effect == Effect::HueWave);

    // Color controls
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_RED), showColorControls);
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_GREEN), showColorControls);
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_BLUE), showColorControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_RED), showColorControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_GREEN), showColorControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_BLUE), showColorControls);

    // Pulse and HueWave specific controls
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_DURATION), showPulseControls);
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_MIN_LIGHT), showPulseControls);
    ShowWindow(GetDlgItem(hwnd, IDC_SLIDER_MAX_LIGHT), showPulseControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_DURATION), showPulseControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_MIN_LIGHT), showPulseControls);
    ShowWindow(GetDlgItem(hwnd, IDC_STATIC_MAX_LIGHT), showPulseControls);

    // Update the dialog layout
    InvalidateRect(hwnd, NULL, TRUE);
}

void UIManager::PreviewEffect(HWND hwnd, Effect effect)
{
    int redVal = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
    int greenVal = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
    int blueVal = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
    int durationVal = SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_GETPOS, 0, 0);
    int minLightVal = SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_GETPOS, 0, 0);
    int maxLightVal = SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_GETPOS, 0, 0);

    switch (effect)
    {
    case Effect::Solid:
        m_ledController.SetSolidColor(redVal, greenVal, blueVal);
        break;
    case Effect::Pulse:
        m_ledController.StartPulseEffect(redVal, greenVal, blueVal, durationVal, minLightVal, maxLightVal);
        break;
    case Effect::Rainbow:
        m_ledController.StartRainbowWave();
        break;
    case Effect::HueWave:
        m_ledController.StartHueWave(redVal, greenVal, blueVal, durationVal, minLightVal, maxLightVal);
        break;
    }
}

void UIManager::SaveSettings(HWND hwnd, Effect effect)
{
    int redVal = SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_GETPOS, 0, 0);
    int greenVal = SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_GETPOS, 0, 0);
    int blueVal = SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_GETPOS, 0, 0);
    float durationVal = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_GETPOS, 0, 0));
    float minLightVal = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_GETPOS, 0, 0));
    float maxLightVal = static_cast<float>(SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_GETPOS, 0, 0));

    m_configManager.SetEffect(effect);

    switch (effect)
    {
    case Effect::Solid:
        m_solidRed = redVal;
        m_solidGreen = greenVal;
        m_solidBlue = blueVal;
        m_configManager.SetSolidColor(m_solidRed, m_solidGreen, m_solidBlue);
        break;
    case Effect::Pulse:
    case Effect::HueWave:
        m_pulseRed = redVal;
        m_pulseGreen = greenVal;
        m_pulseBlue = blueVal;
        m_pulseDuration = durationVal;
        m_pulseMinLight = minLightVal;
        m_pulseMaxLight = maxLightVal;
        m_configManager.SetPulseColor(m_pulseRed, m_pulseGreen, m_pulseBlue);
        m_configManager.SetPulseDuration(m_pulseDuration);
        m_configManager.SetPulseMinLight(m_pulseMinLight);
        m_configManager.SetPulseMaxLight(m_pulseMaxLight);
        break;
    case Effect::Rainbow:
        // No additional settings for Rainbow effect
        break;
    }

    m_configManager.Save();
    PreviewEffect(hwnd, effect);
}

INT_PTR CALLBACK UIManager::ConfigDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    UIManager* pThis = nullptr;

    if (Message == WM_INITDIALOG)
    {
        pThis = reinterpret_cast<UIManager*>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<UIManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (!pThis && Message != WM_INITDIALOG) return FALSE;

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

        // Set initial positions based on current effect
        if (pThis->m_currentEffect == Effect::Solid) {
            SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETPOS, TRUE, pThis->m_solidRed);
            SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETPOS, TRUE, pThis->m_solidGreen);
            SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETPOS, TRUE, pThis->m_solidBlue);
            CheckRadioButton(hwnd, IDC_RADIO_SOLID, IDC_RADIO_HUEWAVE, IDC_RADIO_SOLID);
        }
        else {
            SendDlgItemMessage(hwnd, IDC_SLIDER_RED, TBM_SETPOS, TRUE, pThis->m_pulseRed);
            SendDlgItemMessage(hwnd, IDC_SLIDER_GREEN, TBM_SETPOS, TRUE, pThis->m_pulseGreen);
            SendDlgItemMessage(hwnd, IDC_SLIDER_BLUE, TBM_SETPOS, TRUE, pThis->m_pulseBlue);
            SendDlgItemMessage(hwnd, IDC_SLIDER_DURATION, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pThis->m_pulseDuration)));
            SendDlgItemMessage(hwnd, IDC_SLIDER_MIN_LIGHT, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pThis->m_pulseMinLight)));
            SendDlgItemMessage(hwnd, IDC_SLIDER_MAX_LIGHT, TBM_SETPOS, TRUE, static_cast<LPARAM>(static_cast<int>(pThis->m_pulseMaxLight)));
            CheckRadioButton(hwnd, IDC_RADIO_SOLID, IDC_RADIO_HUEWAVE, IDC_RADIO_PULSE + static_cast<int>(pThis->m_currentEffect) - 1);
        }

        // Update static text
        pThis->UpdateStaticText(hwnd);

        // Show/hide controls based on current effect
        pThis->ShowEffectControls(hwnd, pThis->m_currentEffect);

        return (INT_PTR)TRUE;

    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_RED) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_GREEN) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_BLUE) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_DURATION) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_MIN_LIGHT) ||
            (HWND)lParam == GetDlgItem(hwnd, IDC_SLIDER_MAX_LIGHT))
        {
            pThis->UpdateStaticText(hwnd);
            pThis->PreviewEffect(hwnd, pThis->m_currentEffect);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_RADIO_SOLID:
        case IDC_RADIO_PULSE:
        case IDC_RADIO_RAINBOW:
        case IDC_RADIO_HUEWAVE:
            pThis->m_currentEffect = static_cast<Effect>(LOWORD(wParam) - IDC_RADIO_SOLID);
            pThis->ShowEffectControls(hwnd, pThis->m_currentEffect);
            pThis->PreviewEffect(hwnd, pThis->m_currentEffect);
            return (INT_PTR)TRUE;

        case IDOK:
            pThis->SaveSettings(hwnd, pThis->m_currentEffect);
            EndDialog(hwnd, IDOK);
            return (INT_PTR)TRUE;

        case IDCANCEL:
            // Revert to original settings
            if (pThis->m_configManager.GetEffect() == Effect::Solid) {
                pThis->m_ledController.SetSolidColor(pThis->m_solidRed, pThis->m_solidGreen, pThis->m_solidBlue);
            }
            else {
                pThis->m_ledController.StartPulseEffect(pThis->m_pulseRed, pThis->m_pulseGreen, pThis->m_pulseBlue, pThis->m_pulseDuration, pThis->m_pulseMinLight, pThis->m_pulseMaxLight);
            }
            EndDialog(hwnd, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

