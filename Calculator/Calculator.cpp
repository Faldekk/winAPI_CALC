#include <windows.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <windowsx.h>
#include <cmath>

#define ID_DISPLAY 100
#define ID_BTN_0   200
#define ID_BTN_1   201
#define ID_BTN_2   202
#define ID_BTN_3   203
#define ID_BTN_4   204
#define ID_BTN_5   205
#define ID_BTN_6   206
#define ID_BTN_7   207
#define ID_BTN_8   208
#define ID_BTN_9   209
#define ID_BTN_DOT 210
#define ID_BTN_ADD 211
#define ID_BTN_SUB 212
#define ID_BTN_MUL 213
#define ID_BTN_DIV 214
#define ID_BTN_EQ  215
#define ID_BTN_CLR 216

HWND hDisplay = nullptr;

std::wstring currentInput = L"0";
double firstValue = 0.0;
wchar_t currentOp = 0;
bool waitingForSecondOperand = false;
bool justCalculated = false;

std::wstring FormatDouble(double value)
{
    if (std::isnan(value) || std::isinf(value))
        return L"Error";

    std::wostringstream oss;
    oss << std::fixed << std::setprecision(12) << value;
    std::wstring s = oss.str();

    while (!s.empty() && s.back() == L'0')
        s.pop_back();

    if (!s.empty() && s.back() == L'.')
        s.pop_back();

    if (s.empty() || s == L"-0")
        s = L"0";

    return s;
}

void UpdateDisplay()
{
    SetWindowTextW(hDisplay, currentInput.c_str());
}

double ToDouble(const std::wstring& text)
{
    try
    {
        return std::stod(text);
    }
    catch (...)
    {
        return 0.0;
    }
}

void ClearAll()
{
    currentInput = L"0";
    firstValue = 0.0;
    currentOp = 0;
    waitingForSecondOperand = false;
    justCalculated = false;
    UpdateDisplay();
}

void AppendDigit(wchar_t ch)
{
    if (justCalculated)
    {
        currentInput = L"";
        justCalculated = false;
    }

    if (waitingForSecondOperand)
    {
        currentInput = L"";
        waitingForSecondOperand = false;
    }

    if (currentInput == L"0")
        currentInput = L"";

    currentInput += ch;
    UpdateDisplay();
}

void AppendDot()
{
    if (justCalculated)
    {
        currentInput = L"0";
        justCalculated = false;
    }

    if (waitingForSecondOperand)
    {
        currentInput = L"0";
        waitingForSecondOperand = false;
    }

    if (currentInput.find(L'.') == std::wstring::npos)
    {
        currentInput += L'.';
        UpdateDisplay();
    }
}

bool Calculate()
{
    double secondValue = ToDouble(currentInput);
    double result = 0.0;

    switch (currentOp)
    {
    case L'+':
        result = firstValue + secondValue;
        break;
    case L'-':
        result = firstValue - secondValue;
        break;
    case L'*':
        result = firstValue * secondValue;
        break;
    case L'/':
        if (secondValue == 0.0)
        {
            currentInput = L"Error";
            firstValue = 0.0;
            currentOp = 0;
            waitingForSecondOperand = false;
            justCalculated = true;
            UpdateDisplay();
            return false;
        }
        result = firstValue / secondValue;
        break;
    default:
        return false;
    }

    currentInput = FormatDouble(result);
    firstValue = result;
    currentOp = 0;
    waitingForSecondOperand = false;
    justCalculated = true;
    UpdateDisplay();
    return true;
}

void SetOperation(wchar_t op)
{
    if (currentInput == L"Error")
        ClearAll();

    if (currentOp != 0 && !waitingForSecondOperand)
    {
        if (!Calculate())
            return;
    }

    firstValue = ToDouble(currentInput);
    currentOp = op;
    waitingForSecondOperand = true;
    justCalculated = false;
}

void CreateCalcButton(HWND hwnd, const wchar_t* text, int id, int x, int y, int w, int h)
{
    CreateWindowW(
        L"BUTTON", text,
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        x, y, w, h,
        hwnd, (HMENU)id,
        (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
        nullptr
    );
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        hDisplay = CreateWindowW(
            L"EDIT", L"0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT | ES_READONLY,
            8, 10, 304, 56,
            hwnd, (HMENU)ID_DISPLAY,
            ((LPCREATESTRUCT)lParam)->hInstance,
            nullptr
        );

        HFONT hFont = CreateFontW(
            24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"CalcUI"
        );

        SendMessageW(hDisplay, WM_SETFONT, (WPARAM)hFont, TRUE);

        const int bw = 70;
        const int bh = 55;
        const int gap = 8;
        const int startX = 8;
        const int startY = 80;

        CreateCalcButton(hwnd, L"C", ID_BTN_CLR, startX + 0 * (bw + gap), startY + 0 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"/", ID_BTN_DIV, startX + 1 * (bw + gap), startY + 0 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"*", ID_BTN_MUL, startX + 2 * (bw + gap), startY + 0 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"-", ID_BTN_SUB, startX + 3 * (bw + gap), startY + 0 * (bh + gap), bw, bh);

        CreateCalcButton(hwnd, L"7", ID_BTN_7, startX + 0 * (bw + gap), startY + 1 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"8", ID_BTN_8, startX + 1 * (bw + gap), startY + 1 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"9", ID_BTN_9, startX + 2 * (bw + gap), startY + 1 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"+", ID_BTN_ADD, startX + 3 * (bw + gap), startY + 1 * (bh + gap), bw, bh);

        CreateCalcButton(hwnd, L"4", ID_BTN_4, startX + 0 * (bw + gap), startY + 2 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"5", ID_BTN_5, startX + 1 * (bw + gap), startY + 2 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"6", ID_BTN_6, startX + 2 * (bw + gap), startY + 2 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"=", ID_BTN_EQ, startX + 3 * (bw + gap), startY + 2 * (bh + gap), bw, bh * 2 + gap);

        CreateCalcButton(hwnd, L"1", ID_BTN_1, startX + 0 * (bw + gap), startY + 3 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"2", ID_BTN_2, startX + 1 * (bw + gap), startY + 3 * (bh + gap), bw, bh);
        CreateCalcButton(hwnd, L"3", ID_BTN_3, startX + 2 * (bw + gap), startY + 3 * (bh + gap), bw, bh);

        CreateCalcButton(hwnd, L"0", ID_BTN_0, startX + 0 * (bw + gap), startY + 4 * (bh + gap), bw * 2 + gap, bh);
        CreateCalcButton(hwnd, L".", ID_BTN_DOT, startX + 2 * (bw + gap), startY + 4 * (bh + gap), bw, bh);

        UpdateDisplay();
        return 0;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);

        switch (id)
        {
        case ID_BTN_0: AppendDigit(L'0'); break;
        case ID_BTN_1: AppendDigit(L'1'); break;
        case ID_BTN_2: AppendDigit(L'2'); break;
        case ID_BTN_3: AppendDigit(L'3'); break;
        case ID_BTN_4: AppendDigit(L'4'); break;
        case ID_BTN_5: AppendDigit(L'5'); break;
        case ID_BTN_6: AppendDigit(L'6'); break;
        case ID_BTN_7: AppendDigit(L'7'); break;
        case ID_BTN_8: AppendDigit(L'8'); break;
        case ID_BTN_9: AppendDigit(L'9'); break;
        case ID_BTN_DOT: AppendDot(); break;

        case ID_BTN_ADD: SetOperation(L'+'); break;
        case ID_BTN_SUB: SetOperation(L'-'); break;
        case ID_BTN_MUL: SetOperation(L'*'); break;
        case ID_BTN_DIV: SetOperation(L'/'); break;

        case ID_BTN_EQ:
            if (currentOp != 0)
                Calculate();
            break;

        case ID_BTN_CLR:
            ClearAll();
            break;
        }

        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"SimpleWinApiCalculator";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    const DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    RECT windowRect = { 0, 0, 320, 400 };
    AdjustWindowRect(&windowRect, windowStyle, FALSE);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"DevCalc",
        windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hwnd)
        return 0;

    ShowWindow(hwnd, nCmdShow == SW_HIDE ? SW_SHOWNORMAL : nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}