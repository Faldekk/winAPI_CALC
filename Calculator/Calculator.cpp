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
#define ID_BTN_BCK 210
#define ID_BTN_DOT 211
#define ID_BTN_ADD 212
#define ID_BTN_SUB 213
#define ID_BTN_MUL 214
#define ID_BTN_DIV 216
#define ID_BTN_EQ  217
#define ID_BTN_CLR 218

HWND hDisplayEdit = nullptr;

std::wstring currentInput = L"0";
double firstOperandValue = 0.0;
wchar_t currentOperator = 0;
bool waitingForSecondOperand = false;
bool justCalculated = false;

std::wstring FormatDouble(double value)
{
    if (std::isnan(value) || std::isinf(value))
        return L"Error";

    std::wostringstream outputStream;
    outputStream << std::fixed << std::setprecision(12) << value;
    std::wstring formattedText = outputStream.str();

    while (!formattedText.empty() && formattedText.back() == L'0')
        formattedText.pop_back();

    if (!formattedText.empty() && formattedText.back() == L'.')
        formattedText.pop_back();

    if (formattedText.empty() || formattedText == L"-0")
        formattedText = L"0";

    return formattedText;
}

void UpdateDisplay()
{
    SetWindowTextW(hDisplayEdit, currentInput.c_str());
}

double ToDouble(const std::wstring& inputText)
{
    try
    {
        return std::stod(inputText);
    }
    catch (...)
    {
        return 0.0;
    }
}

void ClearAll()
{
    currentInput = L"0";
    firstOperandValue = 0.0;
    currentOperator = 0;
    waitingForSecondOperand = false;
    justCalculated = false;
    UpdateDisplay();
}

void AppendDigit(wchar_t digitChar)
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

    currentInput += digitChar;
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

void BackspaceInput()
{
    if (currentInput == L"Error")
    {
        ClearAll();
        return;
    }

    if (waitingForSecondOperand)
        return;

    if (currentInput.size() > 1)
    {
        currentInput.pop_back();
        if (currentInput == L"-")
            currentInput = L"0";
    }
    else
    {
        currentInput = L"0";
    }

    justCalculated = false;
    UpdateDisplay();
}

bool Calculate()
{
    double secondOperandValue = ToDouble(currentInput);
    double calculationResult = 0.0;

    switch (currentOperator)
    {
    case L'+':
        calculationResult = firstOperandValue + secondOperandValue;
        break;
    case L'-':
        calculationResult = firstOperandValue - secondOperandValue;
        break;
    case L'*':
        calculationResult = firstOperandValue * secondOperandValue;
        break;
    case L'/':
        if (secondOperandValue == 0.0)
        {
            currentInput = L"Error";
            firstOperandValue = 0.0;
            currentOperator = 0;
            waitingForSecondOperand = false;
            justCalculated = true;
            UpdateDisplay();
            return false;
        }
        calculationResult = firstOperandValue / secondOperandValue;
        break;
    default:
        return false;
    }

    currentInput = FormatDouble(calculationResult);
    firstOperandValue = calculationResult;
    currentOperator = 0;
    waitingForSecondOperand = false;
    justCalculated = true;
    UpdateDisplay();
    return true;
}

void SetOperation(wchar_t operationChar)
{
    if (currentInput == L"Error")
        ClearAll();

    if (currentOperator != 0 && !waitingForSecondOperand)
    {
        if (!Calculate())
            return;
    }

    firstOperandValue = ToDouble(currentInput);
    currentOperator = operationChar;
    waitingForSecondOperand = true;
    justCalculated = false;
}

void CreateCalcButton(HWND parentWindow, const wchar_t* buttonText, int buttonId, int positionX, int positionY, int width, int height)
{
    CreateWindowW(
        L"BUTTON", buttonText,
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        positionX, positionY, width, height,
        parentWindow, (HMENU)(INT_PTR)buttonId,
        (HINSTANCE)GetWindowLongPtr(parentWindow, GWLP_HINSTANCE),
        nullptr
    );
}

LRESULT CALLBACK WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hDisplayEdit = CreateWindowW(
            L"EDIT", L"0",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_RIGHT | ES_READONLY,
            8, 10, 304, 56,
            windowHandle, (HMENU)(INT_PTR)ID_DISPLAY,
            ((LPCREATESTRUCT)lParam)->hInstance,
            nullptr
        );

        HFONT displayFont = CreateFontW(
            24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"CalcUI"
        );

        SendMessageW(hDisplayEdit, WM_SETFONT, (WPARAM)displayFont, TRUE);

        const int buttonWidth = 70;
        const int buttonHeight = 55;
        const int buttonGap = 8;
        const int buttonsStartX = 8;
        const int buttonsStartY = 80;

        CreateCalcButton(windowHandle, L"C", ID_BTN_CLR, buttonsStartX + 0 * (buttonWidth + buttonGap), buttonsStartY + 0 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"/", ID_BTN_DIV, buttonsStartX + 1 * (buttonWidth + buttonGap), buttonsStartY + 0 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"*", ID_BTN_MUL, buttonsStartX + 2 * (buttonWidth + buttonGap), buttonsStartY + 0 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"-", ID_BTN_SUB, buttonsStartX + 3 * (buttonWidth + buttonGap), buttonsStartY + 0 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);

        CreateCalcButton(windowHandle, L"7", ID_BTN_7, buttonsStartX + 0 * (buttonWidth + buttonGap), buttonsStartY + 1 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"8", ID_BTN_8, buttonsStartX + 1 * (buttonWidth + buttonGap), buttonsStartY + 1 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"9", ID_BTN_9, buttonsStartX + 2 * (buttonWidth + buttonGap), buttonsStartY + 1 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"+", ID_BTN_ADD, buttonsStartX + 3 * (buttonWidth + buttonGap), buttonsStartY + 1 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);

        CreateCalcButton(windowHandle, L"4", ID_BTN_4, buttonsStartX + 0 * (buttonWidth + buttonGap), buttonsStartY + 2 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"5", ID_BTN_5, buttonsStartX + 1 * (buttonWidth + buttonGap), buttonsStartY + 2 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"6", ID_BTN_6, buttonsStartX + 2 * (buttonWidth + buttonGap), buttonsStartY + 2 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"=", ID_BTN_EQ, buttonsStartX + 3 * (buttonWidth + buttonGap), buttonsStartY + 2 * (buttonHeight + buttonGap), buttonWidth, buttonHeight * 2 + buttonGap);

        CreateCalcButton(windowHandle, L"1", ID_BTN_1, buttonsStartX + 0 * (buttonWidth + buttonGap), buttonsStartY + 3 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"2", ID_BTN_2, buttonsStartX + 1 * (buttonWidth + buttonGap), buttonsStartY + 3 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"3", ID_BTN_3, buttonsStartX + 2 * (buttonWidth + buttonGap), buttonsStartY + 3 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);

        CreateCalcButton(windowHandle, L"0", ID_BTN_0, buttonsStartX + 0 * (buttonWidth + buttonGap), buttonsStartY + 4 * (buttonHeight + buttonGap), buttonWidth * 2 + buttonGap, buttonHeight);
        CreateCalcButton(windowHandle, L".", ID_BTN_DOT, buttonsStartX + 2 * (buttonWidth + buttonGap), buttonsStartY + 4 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);
        CreateCalcButton(windowHandle, L"<-", ID_BTN_BCK, buttonsStartX + 3 * (buttonWidth + buttonGap), buttonsStartY + 4 * (buttonHeight + buttonGap), buttonWidth, buttonHeight);

        UpdateDisplay();
        return 0;
    }

    case WM_COMMAND:
    {
        int buttonId = LOWORD(wParam);

        switch (buttonId)
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
        case ID_BTN_BCK: BackspaceInput(); break;

        case ID_BTN_ADD: SetOperation(L'+'); break;
        case ID_BTN_SUB: SetOperation(L'-'); break;
        case ID_BTN_MUL: SetOperation(L'*'); break;
        case ID_BTN_DIV: SetOperation(L'/'); break;

        case ID_BTN_EQ:
            if (currentOperator != 0)
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

    return DefWindowProcW(windowHandle, message, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instanceHandle, HINSTANCE, PWSTR, int showCommand)
{
    const wchar_t WINDOW_CLASS_NAME[] = L"SimpleWinApiCalculator";

    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = instanceHandle;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&windowClass);

    const DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    RECT windowRectangle = { 0, 0, 320, 400 };
    AdjustWindowRect(&windowRectangle, windowStyle, FALSE);

    HWND mainWindow = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        L"DevCalc",
        windowStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRectangle.right - windowRectangle.left,
        windowRectangle.bottom - windowRectangle.top,
        nullptr, nullptr, instanceHandle, nullptr
    );

    if (!mainWindow)
        return 0;

    ShowWindow(mainWindow, showCommand == SW_HIDE ? SW_SHOWNORMAL : showCommand);
    UpdateWindow(mainWindow);

    MSG windowMessage = {};
    while (GetMessageW(&windowMessage, nullptr, 0, 0))
    {
        TranslateMessage(&windowMessage);
        DispatchMessageW(&windowMessage);
    }

    return 0;
}