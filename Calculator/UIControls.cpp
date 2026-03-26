#include "CalcGlobal.h"
#include "UICallbacks.h"

void PerformButtonAction(HWND mainWindow, int buttonId)
{
    switch (buttonId)
    {
    case UiIds::Button0: AppendDigit(L'0'); break;
    case UiIds::Button1: AppendDigit(L'1'); break;
    case UiIds::Button2: AppendDigit(L'2'); break;
    case UiIds::Button3: AppendDigit(L'3'); break;
    case UiIds::Button4: AppendDigit(L'4'); break;
    case UiIds::Button5: AppendDigit(L'5'); break;
    case UiIds::Button6: AppendDigit(L'6'); break;
    case UiIds::Button7: AppendDigit(L'7'); break;
    case UiIds::Button8: AppendDigit(L'8'); break;
    case UiIds::Button9: AppendDigit(L'9'); break;
    case UiIds::ButtonA: AppendDigit(L'A'); break;
    case UiIds::ButtonB: AppendDigit(L'B'); break;
    case UiIds::ButtonCHex: AppendDigit(L'C'); break;
    case UiIds::ButtonD: AppendDigit(L'D'); break;
    case UiIds::ButtonE: AppendDigit(L'E'); break;
    case UiIds::ButtonF: AppendDigit(L'F'); break;
    case UiIds::ButtonDot: AppendDot(); break;
    case UiIds::ButtonBackspace: BackspaceInput(); break;

    case UiIds::ButtonAdd: SetOperation(L'+'); break;
    case UiIds::ButtonSub: SetOperation(L'-'); break;
    case UiIds::ButtonMul: SetOperation(L'*'); break;
    case UiIds::ButtonDiv: SetOperation(L'/'); break;

    case UiIds::ButtonEq:
        if (currentOperator != 0)
            Calculate();
        break;

    case UiIds::ButtonClear:
        ClearAll();
        break;
    }

    SetFocus(mainWindow);
}

HMENU CreateApplicationMenu()
{
    HMENU mainMenu = CreateMenu();
    HMENU modeMenu = CreatePopupMenu();
    HMENU baseMenu = CreatePopupMenu();
    HMENU editMenu = CreatePopupMenu();
    HMENU windowMenu = CreatePopupMenu();

    AppendMenuW(modeMenu, MF_STRING | MF_CHECKED, UiIds::MenuModeBasic, L"Basic\tCtrl+1");
    AppendMenuW(modeMenu, MF_STRING, UiIds::MenuModeProgrammer, L"Programmer\tCtrl+2");

    AppendMenuW(baseMenu, MF_STRING | MF_CHECKED, UiIds::MenuBaseDec, L"Decimal\tCtrl+D");
    AppendMenuW(baseMenu, MF_STRING, UiIds::MenuBaseHex, L"Hexadecimal\tCtrl+H");
    AppendMenuW(baseMenu, MF_STRING, UiIds::MenuBaseOct, L"Octal\tCtrl+O");
    AppendMenuW(baseMenu, MF_STRING, UiIds::MenuBaseBin, L"Binary\tCtrl+B");

    AppendMenuW(editMenu, MF_STRING, UiIds::MenuEditClear, L"Clear\tEsc");

    AppendMenuW(windowMenu, MF_STRING, UiIds::MenuWindowTopMost, L"Always on Top");

    AppendMenuW(mainMenu, MF_POPUP, (UINT_PTR)modeMenu, L"Mode");
    AppendMenuW(mainMenu, MF_POPUP, (UINT_PTR)baseMenu, L"Base");
    AppendMenuW(mainMenu, MF_POPUP, (UINT_PTR)editMenu, L"Edit");
    AppendMenuW(mainMenu, MF_POPUP, (UINT_PTR)windowMenu, L"Window");

    return mainMenu;
}

HACCEL CreateApplicationAccelerators()
{
    ACCEL accelerators[] = {
        { FVIRTKEY, VK_ESCAPE, UiIds::MenuEditClear },
        { FVIRTKEY | FCONTROL, '1', UiIds::MenuModeBasic },
        { FVIRTKEY | FCONTROL, '2', UiIds::MenuModeProgrammer },
        { FVIRTKEY | FCONTROL, 'D', UiIds::MenuBaseDec },
        { FVIRTKEY | FCONTROL, 'H', UiIds::MenuBaseHex },
        { FVIRTKEY | FCONTROL, 'O', UiIds::MenuBaseOct },
        { FVIRTKEY | FCONTROL, 'B', UiIds::MenuBaseBin },
        { FVIRTKEY | FCONTROL, 'C', 32774 },
        { FVIRTKEY | FCONTROL, 'V', 32773 }
    };

    return CreateAcceleratorTableW(accelerators, static_cast<int>(std::size(accelerators)));
}

void CreateCalcButton(HWND parentWindow, const wchar_t* buttonText, int buttonId)
{
    HWND buttonHandle = CreateWindowW(
        L"BUTTON", buttonText,
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 0, 0,
        parentWindow,
        (HMENU)(INT_PTR)buttonId,
        (HINSTANCE)GetWindowLongPtr(parentWindow, GWLP_HINSTANCE),
        nullptr);

    if (buttonHandle)
        calculatorButtons.push_back(buttonHandle);
}
