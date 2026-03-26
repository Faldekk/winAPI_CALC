#include "UICallbacks.h"
#include "CalcGlobal.h"
#include "DataTypes.h"
#include <windowsx.h>
#include <algorithm>

const wchar_t MAIN_CLASS_NAME[] = L"SimpleWinApiCalculator";
const wchar_t DISPLAY_CLASS_NAME[] = L"CalcDisplayControl";
const wchar_t BIT_DISPLAY_CLASS_NAME[] = L"BitDisplayControl";

namespace
{
    struct LayoutItem
    {
        int id;
        int row;
        int column;
        int columnSpan;
    };

    constexpr LayoutItem BasicLayout[] = {
        { UiIds::ButtonClear, 0, 0, 1 }, { UiIds::ButtonBackspace, 0, 1, 1 }, { UiIds::ButtonDiv, 0, 2, 1 }, { UiIds::ButtonMul, 0, 3, 1 },
        { UiIds::Button7, 1, 0, 1 }, { UiIds::Button8, 1, 1, 1 }, { UiIds::Button9, 1, 2, 1 }, { UiIds::ButtonSub, 1, 3, 1 },
        { UiIds::Button4, 2, 0, 1 }, { UiIds::Button5, 2, 1, 1 }, { UiIds::Button6, 2, 2, 1 }, { UiIds::ButtonAdd, 2, 3, 1 },
        { UiIds::Button1, 3, 0, 1 }, { UiIds::Button2, 3, 1, 1 }, { UiIds::Button3, 3, 2, 1 },
        { UiIds::Button0, 4, 0, 1 }, { UiIds::ButtonDot, 4, 1, 1 }, { UiIds::ButtonEq, 4, 2, 2 }
    };

    constexpr LayoutItem ProgrammerLayout[] = {
        { UiIds::ButtonClear, 0, 0, 1 }, { UiIds::ButtonBackspace, 0, 1, 1 }, { UiIds::ButtonDiv, 0, 2, 1 }, { UiIds::ButtonMul, 0, 3, 1 },
        { UiIds::Button7, 1, 0, 1 }, { UiIds::Button8, 1, 1, 1 }, { UiIds::Button9, 1, 2, 1 }, { UiIds::ButtonSub, 1, 3, 1 },
        { UiIds::Button4, 2, 0, 1 }, { UiIds::Button5, 2, 1, 1 }, { UiIds::Button6, 2, 2, 1 }, { UiIds::ButtonAdd, 2, 3, 1 },
        { UiIds::Button1, 3, 0, 1 }, { UiIds::Button2, 3, 1, 1 }, { UiIds::Button3, 3, 2, 1 }, { UiIds::ButtonDot, 3, 3, 1 },
        { UiIds::ButtonA, 4, 0, 1 }, { UiIds::ButtonB, 4, 1, 1 }, { UiIds::ButtonCHex, 4, 2, 1 }, { UiIds::ButtonD, 4, 3, 1 },
        { UiIds::Button0, 5, 0, 1 }, { UiIds::ButtonE, 5, 1, 1 }, { UiIds::ButtonF, 5, 2, 1 }, { UiIds::ButtonEq, 5, 3, 1 }
    };

    void ApplyLayout(HWND mainWindow, const LayoutItem* layout, int layoutSize, int buttonsX, int buttonsY, int cellWidth, int cellHeight)
    {
        HDWP deferHandle = BeginDeferWindowPos(static_cast<int>(calculatorButtons.size()));

        for (int i = 0; i < layoutSize; ++i)
        {
            const auto& item = layout[i];
            const HWND buttonHandle = GetDlgItem(mainWindow, item.id);
            if (!buttonHandle)
                continue;

            const int x = buttonsX + item.column * (cellWidth + UI_PADDING);
            const int y = buttonsY + item.row * (cellHeight + UI_PADDING);
            const int width = cellWidth * item.columnSpan + UI_PADDING * (item.columnSpan - 1);

            deferHandle = DeferWindowPos(
                deferHandle,
                buttonHandle,
                nullptr,
                x,
                y,
                width,
                cellHeight,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }

        EndDeferWindowPos(deferHandle);
    }
}

void LayoutControls(HWND mainWindow)
{
    RECT clientRect{};
    GetClientRect(mainWindow, &clientRect);

    const int clientWidth = clientRect.right - clientRect.left;
    const int clientHeight = clientRect.bottom - clientRect.top;

    const int displayHeight = (clientHeight - UI_PADDING * 4) / 4;
    const int displayX = UI_PADDING;
    const int displayY = UI_PADDING;
    const int displayWidth = clientWidth - UI_PADDING * 2;

    if (hDisplayControl)
        SetWindowPos(hDisplayControl, nullptr, displayX, displayY, displayWidth, displayHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    const int comboX = UI_PADDING;
    const int comboY = displayY + displayHeight + UI_PADDING;
    const int comboWidth = displayWidth;
    const int comboDropdownHeight = 220;

    if (hTypeComboBox)
        SetWindowPos(hTypeComboBox, nullptr, comboX, comboY, comboWidth, comboDropdownHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    const int bitDisplayX = UI_PADDING;
    const int bitDisplayY = comboY + 28 + UI_PADDING;
    const int bitDisplayWidth = displayWidth;
    const int bitDisplayHeight = 80;

    if (hBitDisplay)
        SetWindowPos(hBitDisplay, nullptr, bitDisplayX, bitDisplayY, bitDisplayWidth, bitDisplayHeight, SWP_NOZORDER | SWP_NOACTIVATE);

    const int topSectionBottom = programmerMode ? (bitDisplayY + bitDisplayHeight) : (displayY + displayHeight);
    const int buttonsX = UI_PADDING;
    const int buttonsY = topSectionBottom + UI_PADDING;
    const int buttonsWidth = clientWidth - UI_PADDING * 2;
    const int buttonsHeight = clientHeight - buttonsY - UI_PADDING;

    const int gridRows = programmerMode ? 6 : 5;
    const int gridColumns = 4;
    const int cellWidth = (buttonsWidth - UI_PADDING * (gridColumns - 1)) / gridColumns;
    const int cellHeight = (buttonsHeight - UI_PADDING * (gridRows - 1)) / gridRows;

    if (programmerMode)
        ApplyLayout(mainWindow, ProgrammerLayout, static_cast<int>(std::size(ProgrammerLayout)), buttonsX, buttonsY, cellWidth, cellHeight);
    else
        ApplyLayout(mainWindow, BasicLayout, static_cast<int>(std::size(BasicLayout)), buttonsX, buttonsY, cellWidth, cellHeight);
}

void UpdateDisplayFonts(HWND displayWindow)
{
    RECT rect{};
    GetClientRect(displayWindow, &rect);
    const int height = rect.bottom - rect.top;

    const int historySize = (std::max)(12, height / 7);
    const int mainSize = (std::max)(18, height / 3);

    if (hDisplayHistoryFont)
        DeleteObject(hDisplayHistoryFont);
    if (hDisplayMainFont)
        DeleteObject(hDisplayMainFont);

    hDisplayHistoryFont = CreateFontW(
        -historySize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    hDisplayMainFont = CreateFontW(
        -mainSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

LRESULT CALLBACK DisplayWndProc(HWND displayWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        UpdateDisplayFonts(displayWindow);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT paintStruct{};
        HDC hdc = BeginPaint(displayWindow, &paintStruct);

        RECT clientRect{};
        GetClientRect(displayWindow, &clientRect);

        HBRUSH background = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(hdc, &clientRect, background);
        DeleteObject(background);

        RECT historyRect = clientRect;
        historyRect.left += UI_PADDING;
        historyRect.right -= UI_PADDING;
        historyRect.top += UI_PADDING;
        historyRect.bottom = (clientRect.bottom - clientRect.top) / 2;

        RECT inputRect = clientRect;
        inputRect.left += UI_PADDING;
        inputRect.right -= UI_PADDING;
        inputRect.top = historyRect.bottom;
        inputRect.bottom -= UI_PADDING;

        SetBkMode(hdc, TRANSPARENT);

        if (hDisplayHistoryFont)
            SelectObject(hdc, hDisplayHistoryFont);
        SetTextColor(hdc, RGB(90, 90, 90));
        DrawTextW(hdc, operationHistory.c_str(), -1, &historyRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        if (hDisplayMainFont)
            SelectObject(hdc, hDisplayMainFont);
        SetTextColor(hdc, RGB(0, 0, 0));

        std::wstring displayText = currentInput;
        if (!precisionWarning.empty())
            displayText += L" " + precisionWarning;

        DrawTextW(hdc, displayText.c_str(), -1, &inputRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        EndPaint(displayWindow, &paintStruct);
        return 0;
    }

    case WM_DESTROY:
        if (hDisplayHistoryFont)
            DeleteObject(hDisplayHistoryFont);
        if (hDisplayMainFont)
            DeleteObject(hDisplayMainFont);
        hDisplayHistoryFont = nullptr;
        hDisplayMainFont = nullptr;
        return 0;
    }

    return DefWindowProcW(displayWindow, message, wParam, lParam);
}

LRESULT CALLBACK BitDisplayWndProc(HWND bitWindow, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT paintStruct{};
        HDC hdc = BeginPaint(bitWindow, &paintStruct);

        RECT clientRect{};
        GetClientRect(bitWindow, &clientRect);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH background = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(memDC, &clientRect, background);
        DeleteObject(background);

        const int bitCount = DataTypeConverter::GetBitCount(currentDataType);
        const int rawBoxWidth = (clientRect.right - UI_PADDING * 2 - UI_PADDING * (bitCount - 1)) / bitCount;
        const int boxWidth = rawBoxWidth > 8 ? rawBoxWidth : 8;
        const int boxHeight = clientRect.bottom - UI_PADDING * 2;

        const bool isFloat = DataTypeConverter::IsFloat(currentDataType);
        const int exponentBits = (currentDataType == DataType::Float64) ? 11 : (currentDataType == DataType::Float32 ? 8 : 0);

        HFONT bitFont = CreateFontW(-(std::max)(6, boxHeight / 3), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier New");
        HFONT indexFont = CreateFontW(-(std::max)(4, boxHeight / 4), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier New");

        SetBkMode(memDC, TRANSPARENT);

        for (int i = 0; i < bitCount; ++i)
        {
            const int bitPos = bitCount - 1 - i;
            const int x = UI_PADDING + i * (boxWidth + UI_PADDING);
            const int y = UI_PADDING;

            RECT bitRect = { x, y, x + boxWidth, y + boxHeight };

            COLORREF color = RGB(200, 200, 200);
            if (isFloat)
            {
                if (bitPos == bitCount - 1)
                    color = RGB(255, 200, 200);
                else if (bitPos >= bitCount - 1 - exponentBits)
                    color = RGB(200, 255, 200);
                else
                    color = RGB(200, 200, 255);
            }

            HBRUSH bitBrush = CreateSolidBrush(color);
            FillRect(memDC, &bitRect, bitBrush);
            DeleteObject(bitBrush);

            HPEN bitPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            HPEN oldPen = (HPEN)SelectObject(memDC, bitPen);
            MoveToEx(memDC, bitRect.left, bitRect.top, nullptr);
            LineTo(memDC, bitRect.right, bitRect.top);
            LineTo(memDC, bitRect.right, bitRect.bottom);
            LineTo(memDC, bitRect.left, bitRect.bottom);
            LineTo(memDC, bitRect.left, bitRect.top);
            SelectObject(memDC, oldPen);
            DeleteObject(bitPen);

            const wchar_t bitChar = ((currentBitValue >> bitPos) & 1ULL) ? L'1' : L'0';
            RECT bitCharRect = bitRect;
            bitCharRect.bottom = bitCharRect.top + boxHeight * 2 / 3;

            SelectObject(memDC, bitFont);
            SetTextColor(memDC, RGB(0, 0, 0));
            DrawTextW(memDC, &bitChar, 1, &bitCharRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            wchar_t indexStr[4];
            swprintf_s(indexStr, L"%d", bitPos);
            RECT indexRect = bitRect;
            indexRect.top = bitCharRect.bottom;

            SelectObject(memDC, indexFont);
            SetTextColor(memDC, RGB(100, 100, 100));
            DrawTextW(memDC, indexStr, -1, &indexRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        DeleteObject(indexFont);
        DeleteObject(bitFont);

        BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);

        EndPaint(bitWindow, &paintStruct);
        return 0;
    }

    case WM_LBUTTONDOWN:
    {
        const int bitCount = DataTypeConverter::GetBitCount(currentDataType);
        RECT clientRect{};
        GetClientRect(bitWindow, &clientRect);

        const int rawBoxWidth = (clientRect.right - UI_PADDING * 2 - UI_PADDING * (bitCount - 1)) / bitCount;
        const int boxWidth = rawBoxWidth > 8 ? rawBoxWidth : 8;
        const int xClick = static_cast<int>(GET_X_LPARAM(lParam));
        const int bitClicked = (xClick - UI_PADDING) / (boxWidth + UI_PADDING);

        if (bitClicked >= 0 && bitClicked < bitCount)
        {
            const int bitPos = bitCount - 1 - bitClicked;
            currentBitValue ^= (1ULL << bitPos);
            currentInput = DataTypeConverter::FormatValue(currentBitValue, currentDataType, currentBase);
            UpdatePrecisionWarning();
            UpdateDisplay();
        }
        return 0;
    }
    }

    return DefWindowProcW(bitWindow, message, wParam, lParam);
}
