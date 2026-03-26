#include <windows.h>
#include <string>
#include <vector>
#include <windowsx.h>
#include <cwctype>
#include <cstring>
#include "Resource.h"
#include "CalcGlobal.h"
#include "UICallbacks.h"
#include "Modes.h"

int currentMode = 0;
DataType currentDataType = DataType::Int64S;
DisplayBase currentBase = DisplayBase::Decimal;
bool windowAlwaysOnTop = false;

HWND hDisplayControl = nullptr;
HWND hBitDisplay = nullptr;
HWND hTypeComboBox = nullptr;
std::vector<HWND> calculatorButtons;
HACCEL hAccelerators = nullptr;

HFONT hDisplayHistoryFont = nullptr;
HFONT hDisplayMainFont = nullptr;

std::wstring currentInput = L"0";
std::wstring operationHistory;
std::wstring precisionWarning;
unsigned long long currentBitValue = 0;
double firstOperandValue = 0.0;
wchar_t currentOperator = 0;
bool waitingForSecondOperand = false;
bool justCalculated = false;
bool programmerMode = false;

HWND hToolTip = nullptr;
int hoveredBit = -1;

constexpr int CommandCopy = 32774;
constexpr int CommandPaste = 32773;

std::wstring TrimSpaces(const std::wstring& text)
{
    size_t left = 0;
    size_t right = text.size();

    while (left < right && iswspace(text[left]))
        left++;

    while (right > left && iswspace(text[right - 1]))
        right--;

    return text.substr(left, right - left);
}

unsigned long long MaskValue(unsigned long long value)
{
    int bits = DataTypeConverter::GetBitCount(currentDataType);
    if (bits >= 64)
        return value;
    return value & ((1ULL << bits) - 1ULL);
}

void ChangeBase(HWND windowHandle, DisplayBase base)
{
    currentBase = base;

    CheckMenuItem(GetMenu(windowHandle), UiIds::MenuBaseDec, MF_BYCOMMAND | (base == DisplayBase::Decimal ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(GetMenu(windowHandle), UiIds::MenuBaseHex, MF_BYCOMMAND | (base == DisplayBase::Hexadecimal ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(GetMenu(windowHandle), UiIds::MenuBaseOct, MF_BYCOMMAND | (base == DisplayBase::Octal ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(GetMenu(windowHandle), UiIds::MenuBaseBin, MF_BYCOMMAND | (base == DisplayBase::Binary ? MF_CHECKED : MF_UNCHECKED));

    currentInput = DataTypeConverter::FormatValue(currentBitValue, currentDataType, currentBase);
    UpdatePrecisionWarning();
    UpdateDisplay();
}

bool CopyToClip(HWND owner)
{
    if (!OpenClipboard(owner))
        return false;

    EmptyClipboard();

    SIZE_T sizeInBytes = (currentInput.size() + 1) * sizeof(wchar_t);
    HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeInBytes);
    if (!mem)
    {
        CloseClipboard();
        return false;
    }

    void* ptr = GlobalLock(mem);
    if (!ptr)
    {
        GlobalFree(mem);
        CloseClipboard();
        return false;
    }

    memcpy(ptr, currentInput.c_str(), sizeInBytes);
    GlobalUnlock(mem);

    if (!SetClipboardData(CF_UNICODETEXT, mem))
    {
        GlobalFree(mem);
        CloseClipboard();
        return false;
    }

    CloseClipboard();
    return true;
}

bool TryParseInt(const std::wstring& sourceText, unsigned long long& outValue)
{
    std::wstring text = TrimSpaces(sourceText);
    if (text.empty())
        return false;

    bool negative = false;
    int base = 10;

    if (text[0] == L'-')
    {
        negative = true;
        text.erase(0, 1);
    }

    if (text.size() >= 2 && text[0] == L'0' && (text[1] == L'x' || text[1] == L'X'))
    {
        base = 16;
        text.erase(0, 2);
    }
    else if (!text.empty() && (text[0] == L'b' || text[0] == L'B'))
    {
        base = 2;
        text.erase(0, 1);
    }
    else if (!text.empty() && (text[0] == L'o' || text[0] == L'O'))
    {
        base = 8;
        text.erase(0, 1);
    }
    else
    {
        if (currentBase == DisplayBase::Hexadecimal) base = 16;
        if (currentBase == DisplayBase::Octal) base = 8;
        if (currentBase == DisplayBase::Binary) base = 2;
    }

    if (text.empty())
        return false;

    size_t used = 0;

    if (negative)
    {
        long long v = std::stoll(L"-" + text, &used, base);
        if (used != text.size() + 1)
            return false;
        outValue = static_cast<unsigned long long>(v);
    }
    else
    {
        outValue = std::stoull(text, &used, base);
        if (used != text.size())
            return false;
    }

    outValue = MaskValue(outValue);
    return true;
}

bool PasteFromClip(HWND owner)
{
    if (!OpenClipboard(owner))
        return false;

    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (!h)
    {
        CloseClipboard();
        return false;
    }

    const wchar_t* data = static_cast<const wchar_t*>(GlobalLock(h));
    if (!data)
    {
        CloseClipboard();
        return false;
    }

    std::wstring pasted(data);
    GlobalUnlock(h);
    CloseClipboard();

    pasted = TrimSpaces(pasted);
    if (pasted.empty())
        return false;

    try
    {
        if (DataTypeConverter::IsFloat(currentDataType))
        {
            for (auto& c : pasted)
                if (c == L',') c = L'.';

            double check = std::stod(pasted);
            if (std::isnan(check) || std::isinf(check))
                return false;

            currentBitValue = DataTypeConverter::ParseValue(pasted, currentDataType, DisplayBase::Decimal);
            currentInput = DataTypeConverter::FormatValue(currentBitValue, currentDataType, currentBase);
        }
        else
        {
            unsigned long long v = 0;
            if (!TryParseInt(pasted, v))
                return false;

            currentBitValue = v;
            currentInput = DataTypeConverter::FormatValue(currentBitValue, currentDataType, currentBase);
        }

        operationHistory.clear();
        waitingForSecondOperand = false;
        justCalculated = false;
        UpdatePrecisionWarning();
        UpdateDisplay();
        return true;
    }
    catch (...)
    {
        return false;
    }
}

LRESULT CALLBACK MainWndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        SetMenu(windowHandle, CreateApplicationMenu());

        hDisplayControl = CreateWindowExW(
            0, DISPLAY_CLASS_NAME, L"",
            WS_VISIBLE | WS_CHILD,
            0, 0, 0, 0,
            windowHandle,
            (HMENU)(INT_PTR)UiIds::Display,
            ((LPCREATESTRUCT)lParam)->hInstance,
            nullptr);

        hTypeComboBox = CreateWindowW(
            L"COMBOBOX", L"",
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT,
            0, 0, 0, 0,
            windowHandle,
            (HMENU)(INT_PTR)UiIds::ComboDataType,
            (HINSTANCE)GetWindowLongPtr(windowHandle, GWLP_HINSTANCE),
            nullptr);

        SendMessageW(hTypeComboBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
        SendMessageW(hTypeComboBox, CB_SETITEMHEIGHT, (WPARAM)-1, 24);
        SendMessageW(hTypeComboBox, CB_SETITEMHEIGHT, 0, 22);

        for (int i = 0; i <= 10; ++i)
            SendMessageW(hTypeComboBox, CB_ADDSTRING, 0, (LPARAM)DataTypeConverter::GetTypeName(static_cast<DataType>(i)));

        SendMessageW(hTypeComboBox, CB_SETDROPPEDWIDTH, 260, 0);
        SendMessageW(hTypeComboBox, CB_SETCURSEL, static_cast<WPARAM>(currentDataType), 0);

        hBitDisplay = CreateWindowExW(
            0, BIT_DISPLAY_CLASS_NAME, L"",
            WS_VISIBLE | WS_CHILD,
            0, 0, 0, 0,
            windowHandle,
            (HMENU)(INT_PTR)UiIds::BitDisplay,
            ((LPCREATESTRUCT)lParam)->hInstance,
            nullptr);

        CreateCalcButton(windowHandle, L"C", UiIds::ButtonClear);
        CreateCalcButton(windowHandle, L"←", UiIds::ButtonBackspace);
        CreateCalcButton(windowHandle, L"/", UiIds::ButtonDiv);
        CreateCalcButton(windowHandle, L"*", UiIds::ButtonMul);

        CreateCalcButton(windowHandle, L"7", UiIds::Button7);
        CreateCalcButton(windowHandle, L"8", UiIds::Button8);
        CreateCalcButton(windowHandle, L"9", UiIds::Button9);
        CreateCalcButton(windowHandle, L"-", UiIds::ButtonSub);

        CreateCalcButton(windowHandle, L"4", UiIds::Button4);
        CreateCalcButton(windowHandle, L"5", UiIds::Button5);
        CreateCalcButton(windowHandle, L"6", UiIds::Button6);
        CreateCalcButton(windowHandle, L"+", UiIds::ButtonAdd);

        CreateCalcButton(windowHandle, L"1", UiIds::Button1);
        CreateCalcButton(windowHandle, L"2", UiIds::Button2);
        CreateCalcButton(windowHandle, L"3", UiIds::Button3);

        CreateCalcButton(windowHandle, L"A", UiIds::ButtonA);
        CreateCalcButton(windowHandle, L"B", UiIds::ButtonB);
        CreateCalcButton(windowHandle, L"C", UiIds::ButtonCHex);
        CreateCalcButton(windowHandle, L"D", UiIds::ButtonD);

        CreateCalcButton(windowHandle, L"0", UiIds::Button0);
        CreateCalcButton(windowHandle, L"E", UiIds::ButtonE);
        CreateCalcButton(windowHandle, L"F", UiIds::ButtonF);
        CreateCalcButton(windowHandle, L".", UiIds::ButtonDot);

        CreateCalcButton(windowHandle, L"=", UiIds::ButtonEq);

        SwitchMode(0);

        LayoutControls(windowHandle);
        SetFocus(windowHandle);
        UpdateDisplay();
        return 0;
    }

    case WM_SIZE:
        LayoutControls(windowHandle);
        return 0;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
        RECT minimumRect{ 0, 0, MIN_CLIENT_WIDTH, MIN_CLIENT_HEIGHT };

        const DWORD style = static_cast<DWORD>(GetWindowLongPtr(windowHandle, GWL_STYLE));
        const DWORD exStyle = static_cast<DWORD>(GetWindowLongPtr(windowHandle, GWL_EXSTYLE));
        AdjustWindowRectEx(&minimumRect, style, GetMenu(windowHandle) != nullptr, exStyle);

        minMaxInfo->ptMinTrackSize.x = minimumRect.right - minimumRect.left;
        minMaxInfo->ptMinTrackSize.y = minimumRect.bottom - minimumRect.top;
        return 0;
    }

    case WM_COMMAND:
    {
        int commandId = LOWORD(wParam);

        if (IsCalculatorButtonId(commandId))
        {
            PerformButtonAction(windowHandle, commandId);
            return 0;
        }

        if (commandId == UiIds::ComboDataType && HIWORD(wParam) == CBN_SELCHANGE)
        {
            int selection = (int)SendMessageW(hTypeComboBox, CB_GETCURSEL, 0, 0);
            DataType oldType = currentDataType;
            currentDataType = static_cast<DataType>(selection);
            ConvertCurrentValueToType(oldType, currentDataType);
            return 0;
        }

        switch (commandId)
        {
        case UiIds::MenuModeBasic:
            SwitchMode(0);
            return 0;

        case UiIds::MenuModeProgrammer:
            SwitchMode(1);
            return 0;

        case UiIds::MenuBaseDec:
        case UiIds::MenuBaseHex:
        case UiIds::MenuBaseOct:
        case UiIds::MenuBaseBin:
            if (programmerMode)
            {
                DisplayBase base = DisplayBase::Decimal;
                if (commandId == UiIds::MenuBaseHex) base = DisplayBase::Hexadecimal;
                else if (commandId == UiIds::MenuBaseOct) base = DisplayBase::Octal;
                else if (commandId == UiIds::MenuBaseBin) base = DisplayBase::Binary;

                ChangeBase(windowHandle, base);
            }
            return 0;

        case UiIds::MenuEditClear:
            ClearAll();
            SetFocus(windowHandle);
            return 0;

        case UiIds::MenuWindowTopMost:
            windowAlwaysOnTop = !windowAlwaysOnTop;
            CheckMenuItem(GetMenu(windowHandle), UiIds::MenuWindowTopMost,
                         windowAlwaysOnTop ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
            SetWindowPos(windowHandle, windowAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                        0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            return 0;

        case CommandCopy:
            CopyToClip(windowHandle);
            return 0;

        case CommandPaste:
            PasteFromClip(windowHandle);
            return 0;
        }

        return 0;
    }

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            ClearAll();
            return 0;
        case VK_RETURN:
            if (currentOperator != 0)
                Calculate();
            return 0;
        case VK_BACK:
            BackspaceInput();
            return 0;
        }
        break;

    case WM_CHAR:
    {
        wchar_t key = static_cast<wchar_t>(wParam);

        bool handled = programmerMode ? HandleProgrammerChar(key) : HandleBasicChar(key);
        if (handled)
            return 0;

        break;
    }
    break;

    case WM_SETFOCUS:
        SetFocus(windowHandle);
        return 0;

    case WM_DESTROY:
        if (hAccelerators)
        {
            DestroyAcceleratorTable(hAccelerators);
            hAccelerators = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(windowHandle, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instanceHandle, HINSTANCE, PWSTR, int showCommand)
{
    WNDCLASSEXW displayClass{};
    displayClass.cbSize = sizeof(displayClass);
    displayClass.style = CS_HREDRAW | CS_VREDRAW;
    displayClass.lpfnWndProc = DisplayWndProc;
    displayClass.hInstance = instanceHandle;
    displayClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    displayClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    displayClass.lpszClassName = DISPLAY_CLASS_NAME;
    RegisterClassExW(&displayClass);

    WNDCLASSEXW bitDisplayClass{};
    bitDisplayClass.cbSize = sizeof(bitDisplayClass);
    bitDisplayClass.style = CS_HREDRAW | CS_VREDRAW;
    bitDisplayClass.lpfnWndProc = BitDisplayWndProc;
    bitDisplayClass.hInstance = instanceHandle;
    bitDisplayClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    bitDisplayClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    bitDisplayClass.lpszClassName = BIT_DISPLAY_CLASS_NAME;
    RegisterClassExW(&bitDisplayClass);

    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = MainWndProc;
    windowClass.hInstance = instanceHandle;
    windowClass.hIcon = LoadIconW(instanceHandle, MAKEINTRESOURCEW(IDI_CALCULATOR));
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszClassName = MAIN_CLASS_NAME;
    windowClass.hIconSm = LoadIconW(instanceHandle, MAKEINTRESOURCEW(IDI_SMALL));
    RegisterClassExW(&windowClass);

    const DWORD windowStyle = WS_OVERLAPPEDWINDOW;
    RECT windowRectangle = { 0, 0, MIN_CLIENT_WIDTH, MIN_CLIENT_HEIGHT };
    AdjustWindowRectEx(&windowRectangle, windowStyle, TRUE, 0);

    HWND mainWindow = CreateWindowExW(
        0, MAIN_CLASS_NAME, L"DevCalc",
        windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRectangle.right - windowRectangle.left,
        windowRectangle.bottom - windowRectangle.top,
        nullptr, nullptr, instanceHandle, nullptr);

    if (!mainWindow)
        return 0;

    hAccelerators = CreateApplicationAccelerators();

    ShowWindow(mainWindow, showCommand == SW_HIDE ? SW_SHOWNORMAL : showCommand);
    UpdateWindow(mainWindow);

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0))
    {
        if (!hAccelerators || !TranslateAcceleratorW(mainWindow, hAccelerators, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    return static_cast<int>(message.wParam);
}
