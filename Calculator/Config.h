#pragma once

#include <windows.h>
#include <string>

const wchar_t CONFIG_FILE[] = L"config.ini";
const wchar_t CONFIG_SECTION[] = L"Settings";

struct ProgrammerSettings
{
    int windowX = CW_USEDEFAULT;
    int windowY = CW_USEDEFAULT;
    int windowWidth = 400;
    int windowHeight = 600;
    int dataType = 3;
    int displayBase = 0;
    bool alwaysOnTop = false;
    int mode = 0;
};

class ConfigManager
{
public:
    static void SaveSettings(const ProgrammerSettings& settings)
    {
        WritePrivateProfileIntW(L"windowX", settings.windowX, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"windowY", settings.windowY, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"windowWidth", settings.windowWidth, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"windowHeight", settings.windowHeight, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"dataType", settings.dataType, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"displayBase", settings.displayBase, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"alwaysOnTop", settings.alwaysOnTop ? 1 : 0, CONFIG_SECTION, CONFIG_FILE);
        WritePrivateProfileIntW(L"mode", settings.mode, CONFIG_SECTION, CONFIG_FILE);
    }

    static ProgrammerSettings LoadSettings()
    {
        ProgrammerSettings settings;
        settings.windowX = GetPrivateProfileIntW(L"windowX", CW_USEDEFAULT, CONFIG_SECTION, CONFIG_FILE);
        settings.windowY = GetPrivateProfileIntW(L"windowY", CW_USEDEFAULT, CONFIG_SECTION, CONFIG_FILE);
        settings.windowWidth = GetPrivateProfileIntW(L"windowWidth", 400, CONFIG_SECTION, CONFIG_FILE);
        settings.windowHeight = GetPrivateProfileIntW(L"windowHeight", 600, CONFIG_SECTION, CONFIG_FILE);
        settings.dataType = GetPrivateProfileIntW(L"dataType", 3, CONFIG_SECTION, CONFIG_FILE);
        settings.displayBase = GetPrivateProfileIntW(L"displayBase", 0, CONFIG_SECTION, CONFIG_FILE);
        settings.alwaysOnTop = GetPrivateProfileIntW(L"alwaysOnTop", 0, CONFIG_SECTION, CONFIG_FILE) != 0;
        settings.mode = GetPrivateProfileIntW(L"mode", 0, CONFIG_SECTION, CONFIG_FILE);
        return settings;
    }

private:
    static void WritePrivateProfileIntW(const wchar_t* key, int value, const wchar_t* section, const wchar_t* file)
    {
        wchar_t buffer[32];
        swprintf_s(buffer, L"%d", value);
        WritePrivateProfileStringW(section, key, buffer, file);
    }

    static int GetPrivateProfileIntW(const wchar_t* key, int defaultValue, const wchar_t* section, const wchar_t* file)
    {
        wchar_t buffer[32];
        GetPrivateProfileStringW(section, key, L"", buffer, static_cast<DWORD>(std::size(buffer)), file);
        if (buffer[0] == L'\0')
            return defaultValue;
        return _wtoi(buffer);
    }
};
