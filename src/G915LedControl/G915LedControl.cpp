
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
#include "UIManager.h"

#pragma comment(lib, "comctl32.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
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

    LEDController ledController;
    ConfigManager configManager;

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

    // Create and initialize UIManager
    UIManager uiManager(hInstance, configManager, ledController);
    if (!uiManager.Initialize())
    {
        return 0;
    }

    // Start the lock keys update thread
    std::thread(&LEDController::UpdateLockKeys, &ledController).detach();

    // Run the message loop
    uiManager.Run();

    ledController.Shutdown();
    return 0;
}