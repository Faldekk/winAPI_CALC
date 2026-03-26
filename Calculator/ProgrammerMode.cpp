#include "CalcGlobal.h"
#include "Modes.h"

namespace
{
    const int HexButtonIds[] = {
        UiIds::ButtonA,
        UiIds::ButtonB,
        UiIds::ButtonCHex,
        UiIds::ButtonD,
        UiIds::ButtonE,
        UiIds::ButtonF
    };

    const int BaseMenuIds[] = {
        UiIds::MenuBaseDec,
        UiIds::MenuBaseHex,
        UiIds::MenuBaseOct,
        UiIds::MenuBaseBin
    };
}

bool HandleProgrammerChar(wchar_t key)
{
    if (HandleBasicChar(key))
        return true;

    if (key >= L'A' && key <= L'F')
    {
        AppendDigit(key);
        return true;
    }

    if (key >= L'a' && key <= L'f')
    {
        AppendDigit(static_cast<wchar_t>(key - L'a' + L'A'));
        return true;
    }

    return false;
}

void EnterProgrammerMode(HWND mainWindow)
{
    programmerMode = true;
    currentMode = 1;

    if (hTypeComboBox)
        ShowWindow(hTypeComboBox, SW_SHOW);

    if (hBitDisplay)
        ShowWindow(hBitDisplay, SW_SHOW);

    for (int id : HexButtonIds)
        ShowWindow(GetDlgItem(mainWindow, id), SW_SHOW);

    HMENU menu = GetMenu(mainWindow);
    if (menu)
    {
        CheckMenuItem(menu, UiIds::MenuModeBasic, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(menu, UiIds::MenuModeProgrammer, MF_BYCOMMAND | MF_CHECKED);

        for (int id : BaseMenuIds)
            EnableMenuItem(menu, id, MF_BYCOMMAND | MF_ENABLED);
    }

    LayoutControls(mainWindow);
    ClearAll();
}

void SwitchMode(int newMode)
{
    HWND mainWindow = GetParent(hDisplayControl);
    if (!mainWindow)
        return;

    if (newMode == 0)
        EnterBasicMode(mainWindow);
    else
        EnterProgrammerMode(mainWindow);
}
