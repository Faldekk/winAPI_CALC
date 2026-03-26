#include "CalcGlobal.h"
#include "DataTypes.h"
#include <cmath>
#include <sstream>
#include <iomanip>

unsigned long long GetMaskFromBits(int bits)
{
    if (bits >= 64)
    {
        return ~0ULL;
    }
    return (1ULL << bits) - 1ULL;
}

std::wstring FormatDouble(double value)
{
    if (std::isnan(value) || std::isinf(value))
    {
        return L"Error";
    }

    std::wostringstream stream;
    stream << std::fixed << std::setprecision(12) << value;

    std::wstring text = stream.str();

    while (!text.empty() && text.back() == L'0')
    {
        text.pop_back();
    }

    if (!text.empty() && text.back() == L'.')
    {
        text.pop_back();
    }

    if (text.empty() || text == L"-0")
    {
        text = L"0";
    }

    return text;
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

void UpdateDisplay()
{
    if (hDisplayControl)
    {
        InvalidateRect(hDisplayControl, nullptr, FALSE);
    }

    if (hBitDisplay)
    {
        InvalidateRect(hBitDisplay, nullptr, FALSE);
    }
}

void UpdatePrecisionWarning()
{
    precisionWarning.clear();

    if (!DataTypeConverter::IsFloat(currentDataType))
    {
        return;
    }

    try
    {
        unsigned long long parsed = DataTypeConverter::ParseValue(currentInput, currentDataType, DisplayBase::Decimal);
        std::wstring stored = DataTypeConverter::FormatValue(parsed, currentDataType, DisplayBase::Decimal);

        if (stored != currentInput)
        {
            precisionWarning = L"(stored: " + stored + L")";
        }
    }
    catch (...)
    {
    }
}

void ClearAll()
{
    currentInput = L"0";
    operationHistory.clear();
    precisionWarning.clear();
    currentBitValue = 0;
    firstOperandValue = 0.0;
    currentOperator = 0;
    waitingForSecondOperand = false;
    justCalculated = false;
    UpdateDisplay();
}

void AppendDigit(wchar_t digitChar)
{
    if (currentInput == L"Error")
    {
        ClearAll();
    }

    if (justCalculated)
    {
        currentInput = L"";
        operationHistory.clear();
        precisionWarning.clear();
        justCalculated = false;
    }

    if (waitingForSecondOperand)
    {
        currentInput = L"";
        waitingForSecondOperand = false;
    }

    if (currentInput == L"0")
    {
        currentInput = L"";
    }

    currentInput += digitChar;

    UpdatePrecisionWarning();
    UpdateDisplay();
}

void AppendDot()
{
    if (currentInput == L"Error")
    {
        ClearAll();
    }

    if (justCalculated)
    {
        currentInput = L"0";
        operationHistory.clear();
        precisionWarning.clear();
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
    {
        return;
    }

    if (currentInput.size() > 1)
    {
        currentInput.pop_back();

        if (currentInput == L"-")
        {
            currentInput = L"0";
        }
    }
    else
    {
        currentInput = L"0";
    }

    justCalculated = false;
    UpdatePrecisionWarning();
    UpdateDisplay();
}

bool Calculate()
{
    if (currentOperator == 0)
    {
        return false;
    }

    double secondValue = ToDouble(currentInput);
    double firstValueBefore = firstOperandValue;
    std::wstring secondText = currentInput;
    double result = 0.0;

    if (currentOperator == L'+')
    {
        result = firstOperandValue + secondValue;
    }
    else if (currentOperator == L'-')
    {
        result = firstOperandValue - secondValue;
    }
    else if (currentOperator == L'*')
    {
        result = firstOperandValue * secondValue;
    }
    else if (currentOperator == L'/')
    {
        if (secondValue == 0.0)
        {
            operationHistory = FormatDouble(firstValueBefore) + L" / " + secondText + L" =";
            currentInput = L"Error";
            firstOperandValue = 0.0;
            currentOperator = 0;
            waitingForSecondOperand = false;
            justCalculated = true;
            precisionWarning.clear();
            UpdateDisplay();
            return false;
        }

        result = firstOperandValue / secondValue;
    }
    else
    {
        return false;
    }

    operationHistory = FormatDouble(firstValueBefore) + L" " + std::wstring(1, currentOperator) + L" " + secondText + L" =";

    currentInput = FormatDouble(result);
    firstOperandValue = result;
    currentOperator = 0;
    waitingForSecondOperand = false;
    justCalculated = true;

    UpdatePrecisionWarning();
    UpdateDisplay();
    return true;
}

void SetOperation(wchar_t operationChar)
{
    if (currentInput == L"Error")
    {
        ClearAll();
    }

    if (currentOperator != 0 && !waitingForSecondOperand)
    {
        if (!Calculate())
        {
            return;
        }
    }

    firstOperandValue = ToDouble(currentInput);
    currentOperator = operationChar;
    waitingForSecondOperand = true;
    justCalculated = false;
    operationHistory = currentInput + L" " + std::wstring(1, operationChar);
    precisionWarning.clear();
    UpdateDisplay();
}

void ConvertCurrentValueToType(DataType oldType, DataType newType)
{
    try
    {
        unsigned long long oldRawValue = DataTypeConverter::ParseValue(currentInput, oldType, currentBase);

        if (DataTypeConverter::IsFloat(oldType) && !DataTypeConverter::IsFloat(newType))
        {
            std::wstring textAsDecimal = DataTypeConverter::FormatValue(oldRawValue, oldType, DisplayBase::Decimal);
            double temp = ToDouble(textAsDecimal);
            currentBitValue = static_cast<unsigned long long>(static_cast<long long>(temp));
        }
        else if (!DataTypeConverter::IsFloat(oldType) && DataTypeConverter::IsFloat(newType))
        {
            std::wstring textAsDecimal = DataTypeConverter::FormatValue(oldRawValue, oldType, DisplayBase::Decimal);
            currentBitValue = DataTypeConverter::ParseValue(textAsDecimal, newType, DisplayBase::Decimal);
        }
        else
        {
            currentBitValue = oldRawValue;

            int oldBits = DataTypeConverter::GetBitCount(oldType);
            int newBits = DataTypeConverter::GetBitCount(newType);

            if (newBits < oldBits)
            {
                currentBitValue = currentBitValue & GetMaskFromBits(newBits);
            }
            else if (DataTypeConverter::IsSigned(oldType) && oldBits < 64)
            {
                unsigned long long signBit = 1ULL << (oldBits - 1);

                if (oldRawValue & signBit)
                {
                    currentBitValue = oldRawValue | (~GetMaskFromBits(oldBits));
                }
            }
        }

        currentBitValue = currentBitValue & GetMaskFromBits(DataTypeConverter::GetBitCount(newType));
        currentInput = DataTypeConverter::FormatValue(currentBitValue, newType, currentBase);

        UpdatePrecisionWarning();
        UpdateDisplay();
    }
    catch (...)
    {
        currentBitValue = 0;
        currentInput = L"0";
        UpdateDisplay();
    }
}
