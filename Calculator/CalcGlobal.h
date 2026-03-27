#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "DataTypes.h"

namespace UiIds
{
    constexpr int Display = 100;
    constexpr int Button0 = 200;
    constexpr int Button1 = 201;
    constexpr int Button2 = 202;
    constexpr int Button3 = 203;
    constexpr int Button4 = 204;
    constexpr int Button5 = 205;
    constexpr int Button6 = 206;
    constexpr int Button7 = 207;
    constexpr int Button8 = 208;
    constexpr int Button9 = 209;
    constexpr int ButtonA = 210;
    constexpr int ButtonB = 211;
    constexpr int ButtonCHex = 212;
    constexpr int ButtonD = 213;
    constexpr int ButtonE = 214;
    constexpr int ButtonF = 215;
    constexpr int ButtonBackspace = 216;
    constexpr int ButtonDot = 217;
    constexpr int ButtonAdd = 218;
    constexpr int ButtonSub = 219;
    constexpr int ButtonMul = 220;
    constexpr int ButtonDiv = 221;
    constexpr int ButtonEq = 222;
    constexpr int ButtonClear = 223;

    constexpr int ComboDataType = 300;
    constexpr int BitDisplay = 301;

    constexpr int MenuModeBasic = 40001;
    constexpr int MenuModeProgrammer = 40002;
    constexpr int MenuEditClear = 40003;
    constexpr int MenuBaseDec = 40010;
    constexpr int MenuBaseHex = 40011;
    constexpr int MenuBaseOct = 40012;
    constexpr int MenuBaseBin = 40013;
    constexpr int MenuWindowTopMost = 40020;
}

inline bool IsCalculatorButtonId(int id)
{
    return id >= UiIds::Button0 && id <= UiIds::ButtonClear;
}

extern int currentMode;
extern DataType currentDataType;
extern DisplayBase currentBase;
extern bool windowAlwaysOnTop;

extern HWND hDisplayControl;
extern HWND hBitDisplay;
extern HWND hTypeComboBox;
extern std::vector<HWND> calculatorButtons;
extern HACCEL hAccelerators;

extern HFONT hDisplayHistoryFont;
extern HFONT hDisplayMainFont;

extern std::wstring currentInput;
extern std::wstring operationHistory;
extern std::wstring precisionWarning;
extern unsigned long long currentBitValue;
extern double firstOperandValue;
extern wchar_t currentOperator;
extern bool waitingForSecondOperand;
extern bool justCalculated;
extern bool programmerMode;
extern HWND hToolTip;
extern int hoveredBit;

void UpdateDisplay();
void UpdatePrecisionWarning();
void LayoutControls(HWND mainWindow);
void UpdateDisplayFonts(HWND displayWindow);

std::wstring FormatDouble(double value);
double ToDouble(const std::wstring& inputText);
void ClearAll();
void AppendDigit(wchar_t digitChar);
void AppendDot();
void BackspaceInput();
bool Calculate();
void SetOperation(wchar_t operationChar);

void SwitchMode(int newMode);
void ConvertCurrentValueToType(DataType oldType, DataType newType);

HMENU CreateApplicationMenu();
HACCEL CreateApplicationAccelerators();
void CreateCalcButton(HWND parentWindow, const wchar_t* buttonText, int buttonId);
void PerformButtonAction(HWND mainWindow, int buttonId);
