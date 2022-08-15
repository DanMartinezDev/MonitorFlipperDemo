// MonitorFlipper.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#pragma comment(lib, "dxva2.lib")
#include <windows.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <strsafe.h>
#include <sstream>
#include <cstdlib>

using namespace std;

void PrintLastError(string customMessage);
void PrintCapabilities(HANDLE hPhysicalMonitor);
BOOL CALLBACK MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData);

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static void PrintLastError(string customMessage = "No Custome Message")
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();

    if (errorMessageID == 0) {
        cout << "No error message has been recorded" << endl;
        return;
    }

    cout << customMessage << endl;

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    cout << message << endl;
}

static void PrintCapabilities(HANDLE hPhysicalMonitor)
{
    cout << "Attempting to get capabilities for monitor: " << hPhysicalMonitor << endl;

    DWORD pdwMonitorCapabilities = 0;
    DWORD pdwSupportedColorTemperatures = 0;

    if (!GetMonitorCapabilities(hPhysicalMonitor, &pdwMonitorCapabilities, &pdwSupportedColorTemperatures))
    {
        PrintLastError();
        return;
    }

    cout << pdwMonitorCapabilities << endl;

    return;
}

const wstring LEFT_ASUS = L"\\\\.\\DISPLAY2";
const wstring RIGHT_ASUS = L"\\\\.\\DISPLAY1";
const wstring LG = L"\\\\.\\DISPLAY3";

const DWORD HDMI = 17;
const DWORD DisplayPort = 15;

static void SwitchInput(HANDLE hPhysicalMonitor, DWORD currentInputValue, wstring displayName)
{
    DWORD newInputValue;

    if (displayName == LEFT_ASUS) {
        if (currentInputValue == HDMI) {
            newInputValue = DisplayPort;
        }
        else if (currentInputValue == DisplayPort) {
            newInputValue = HDMI;
        }
        else {
            cout << "Can't communicate with monitor. Skipping." << endl;
            return;
        }

        cout << "Setting Left Monitor to " << newInputValue << endl;
        if (!SetVCPFeature(hPhysicalMonitor, 0x60, newInputValue)) PrintLastError();
    }

    if (displayName == LG) {
        if (currentInputValue == HDMI) {
            newInputValue = DisplayPort;
        }
        else if (currentInputValue == DisplayPort) {
            newInputValue = HDMI;
        }
        else {
            cout << "Can't communicate with monitor. Skipping." << endl;
            return;
        }

        cout << "Setting Center Monitor to " << newInputValue << endl;
        if (!SetVCPFeature(hPhysicalMonitor, 0x60, newInputValue)) PrintLastError();
    }
}

static DWORD GetVCPFeatureCurrentValue(HANDLE hPhysicalMonitor)
{
    DWORD pdwCurrentValue;
    DWORD pdwMaximumValue;

    if (!GetVCPFeatureAndVCPFeatureReply(hPhysicalMonitor, 0x60, NULL, &pdwCurrentValue, &pdwMaximumValue)) PrintLastError();

    cout << "Current Input Value: " << pdwCurrentValue << endl;
    cout << "Maximum Input Value: " << pdwMaximumValue << endl;

    return pdwCurrentValue;
}

static wstring PrintMonitorName(HMONITOR hMonitor)
{
    MONITORINFOEX mi{};
    mi.cbSize = sizeof mi;

    if (!GetMonitorInfoW(hMonitor, &mi)) PrintLastError("Monitor Name Error");
    wcout << "Display Monitor Name: " << mi.szDevice << endl;

    return wstring(mi.szDevice);
}

static BOOL CALLBACK MonitorEnum(HMONITOR hMonitor, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
{
    wstring monitorName = PrintMonitorName(hMonitor);
    cout << "-----------------------------------------" << endl;

    DWORD nMonitorCount = 0;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, &nMonitorCount)) PrintLastError();

    PHYSICAL_MONITOR* physicalMonitors = new PHYSICAL_MONITOR[nMonitorCount];

    if (!GetPhysicalMonitorsFromHMONITOR(hMonitor, nMonitorCount, physicalMonitors)) PrintLastError();
    
    
    DWORD currentInputValue = GetVCPFeatureCurrentValue(physicalMonitors[0].hPhysicalMonitor);

    SwitchInput(physicalMonitors[0].hPhysicalMonitor, currentInputValue, monitorName);
    
    cout << "-----------------------------------------" << endl;
    cout << "-----------------------------------------" << endl;

    cout << endl;
    cout << endl;

    delete[] physicalMonitors;

    return TRUE;
}



int main()
{
    EnumDisplayMonitors(NULL, NULL, MonitorEnum, 0);

    std::getchar();
}
