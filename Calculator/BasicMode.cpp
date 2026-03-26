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

bool HandleBasicChar(wchar_t key)
{
    if (key >= L'0' && key <= L'9')
    {
        AppendDigit(key);
        return true;
    }

    switch (key)
    {
    case L'.':
    case L',':
        AppendDot();
        return true;
    case L'+':
        SetOperation(L'+');
        return true;
    case L'-':
        SetOperation(L'-');
        return true;
    case L'*':
        SetOperation(L'*');
        return true;
    case L'/':
        SetOperation(L'/');
        return true;
    case L'=':
        if (currentOperator != 0)
            Calculate();
        return true;
    }

    return false;
}

void EnterBasicMode(HWND mainWindow)
{
    programmerMode = false;
    currentMode = 0;
    currentBase = DisplayBase::Decimal;

    if (hTypeComboBox)
        ShowWindow(hTypeComboBox, SW_HIDE);

    if (hBitDisplay)
        ShowWindow(hBitDisplay, SW_HIDE);

    for (int id : HexButtonIds)
        ShowWindow(GetDlgItem(mainWindow, id), SW_HIDE);

    HMENU menu = GetMenu(mainWindow);
    if (menu)
    {
        CheckMenuItem(menu, UiIds::MenuModeBasic, MF_BYCOMMAND | MF_CHECKED);
        CheckMenuItem(menu, UiIds::MenuModeProgrammer, MF_BYCOMMAND | MF_UNCHECKED);

        for (int id : BaseMenuIds)
            EnableMenuItem(menu, id, MF_BYCOMMAND | MF_GRAYED);
    }

    LayoutControls(mainWindow);
    ClearAll();
}
