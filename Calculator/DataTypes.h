#pragma once

#include <windows.h>
#include <string>
#include <algorithm>
#include <cmath>
#include <cstring>

enum class DataType
{
    Int8S = 0,
    Int16S = 1,
    Int32S = 2,
    Int64S = 3,
    Int8U = 4,
    Int16U = 5,
    Int32U = 6,
    Int64U = 7,
    Float16 = 8,
    Float32 = 9,
    Float64 = 10
};

enum class DisplayBase
{
    Decimal = 0,
    Hexadecimal = 1,
    Octal = 2,
    Binary = 3
};

class DataTypeConverter
{
public:
    static constexpr int GetBitCount(DataType type)
    {
        switch (type)
        {
        case DataType::Int8S:
        case DataType::Int8U:
            return 8;
        case DataType::Int16S:
        case DataType::Int16U:
        case DataType::Float16:
            return 16;
        case DataType::Int32S:
        case DataType::Int32U:
        case DataType::Float32:
            return 32;
        case DataType::Int64S:
        case DataType::Int64U:
        case DataType::Float64:
            return 64;
        default:
            return 64;
        }
    }

    static constexpr const wchar_t* GetTypeName(DataType type)
    {
        switch (type)
        {
        case DataType::Int8S:   return L"Int8 (signed)";
        case DataType::Int16S:  return L"Int16 (signed)";
        case DataType::Int32S:  return L"Int32 (signed)";
        case DataType::Int64S:  return L"Int64 (signed)";
        case DataType::Int8U:   return L"Int8 (unsigned)";
        case DataType::Int16U:  return L"Int16 (unsigned)";
        case DataType::Int32U:  return L"Int32 (unsigned)";
        case DataType::Int64U:  return L"Int64 (unsigned)";
        case DataType::Float16: return L"Float16 (half)";
        case DataType::Float32: return L"Float32 (single)";
        case DataType::Float64: return L"Float64 (double)";
        default:                return L"Unknown";
        }
    }

    static constexpr bool IsSigned(DataType type)
    {
        return type <= DataType::Int64S;
    }

    static constexpr bool IsFloat(DataType type)
    {
        return type >= DataType::Float16;
    }

    static std::wstring GetBinaryString(unsigned long long value, int bitCount)
    {
        std::wstring result;
        for (int i = bitCount - 1; i >= 0; --i)
        {
            result += ((value >> i) & 1) ? L'1' : L'0';
        }
        return result;
    }

    static std::wstring GetHexString(unsigned long long value, int bitCount)
    {
        std::wstring result = L"0x";
        int hexDigits = (bitCount + 3) / 4;
        wchar_t buffer[20];
        swprintf_s(buffer, L"%0*llx", hexDigits, value);
        result += buffer;
        return result;
    }

    static std::wstring GetOctalString(unsigned long long value, int bitCount)
    {
        std::wstring result = L"o";
        wchar_t buffer[30];
        swprintf_s(buffer, L"%llo", value);
        result += buffer;
        return result;
    }

    static std::wstring GetDecimalString(unsigned long long value, DataType type)
    {
        wchar_t buffer[100];

        if (IsFloat(type))
        {
            double doubleValue = 0.0;

            if (type == DataType::Float32)
            {
                float f;
                std::memcpy(&f, &value, sizeof(float));
                doubleValue = static_cast<double>(f);
            }
            else if (type == DataType::Float64)
            {
                std::memcpy(&doubleValue, &value, sizeof(double));
            }
            else if (type == DataType::Float16)
            {
                doubleValue = ConvertFloat16ToDouble(static_cast<unsigned short>(value));
            }

            swprintf_s(buffer, L"%.15g", doubleValue);
        }
        else if (IsSigned(type))
        {
            long long signedValue = static_cast<long long>(value);

            int bitCount = GetBitCount(type);
            if (bitCount < 64)
            {
                unsigned long long signBit = 1ULL << (bitCount - 1);
                if (value & signBit)
                {
                    signedValue = static_cast<long long>(value | ~((1ULL << bitCount) - 1));
                }
            }

            swprintf_s(buffer, L"%lld", signedValue);
        }
        else
        {
            swprintf_s(buffer, L"%llu", value);
        }

        return buffer;
    }

    static std::wstring FormatValue(unsigned long long value, DataType type, DisplayBase base)
    {
        int bitCount = GetBitCount(type);

        switch (base)
        {
        case DisplayBase::Hexadecimal:
            return GetHexString(value, bitCount);
        case DisplayBase::Octal:
            return GetOctalString(value, bitCount);
        case DisplayBase::Binary:
            return L"b" + GetBinaryString(value, bitCount);
        case DisplayBase::Decimal:
        default:
            return GetDecimalString(value, type);
        }
    }

    static unsigned long long ParseValue(const std::wstring& text, DataType type, DisplayBase base)
    {
        try
        {
            std::wstring trimmed = text;

            while (!trimmed.empty() && iswspace(trimmed.front()))
                trimmed.erase(trimmed.begin());
            while (!trimmed.empty() && iswspace(trimmed.back()))
                trimmed.pop_back();

            if (trimmed.empty())
                return 0;

            bool negative = false;
            if (!trimmed.empty() && trimmed[0] == L'-')
            {
                negative = true;
                trimmed.erase(0, 1);
            }

            int parseBase = 10;
            if (base == DisplayBase::Hexadecimal) parseBase = 16;
            else if (base == DisplayBase::Octal) parseBase = 8;
            else if (base == DisplayBase::Binary) parseBase = 2;

            if (trimmed.find(L"0x") == 0 || trimmed.find(L"0X") == 0)
            {
                parseBase = 16;
                trimmed = trimmed.substr(2);
            }
            else if (trimmed.find(L"b") == 0 || trimmed.find(L"B") == 0)
            {
                parseBase = 2;
                trimmed = trimmed.substr(1);
            }
            else if (trimmed.find(L"o") == 0 || trimmed.find(L"O") == 0)
            {
                parseBase = 8;
                trimmed = trimmed.substr(1);
            }

            if (trimmed.empty())
                return 0;

            if (IsFloat(type))
            {
                std::wstring floatText = negative ? (L"-" + trimmed) : trimmed;
                double doubleValue = std::wcstod(floatText.c_str(), nullptr);
                unsigned long long result = 0;

                if (type == DataType::Float32)
                {
                    float f = static_cast<float>(doubleValue);
                    std::memcpy(&result, &f, sizeof(float));
                }
                else if (type == DataType::Float64)
                {
                    std::memcpy(&result, &doubleValue, sizeof(double));
                }
                else if (type == DataType::Float16)
                {
                    result = ConvertDoubleToFloat16(doubleValue);
                }

                return result;
            }

            if (IsSigned(type))
            {
                std::wstring signedText = negative ? (L"-" + trimmed) : trimmed;
                long long signedValue = std::stoll(signedText, nullptr, parseBase);
                return static_cast<unsigned long long>(signedValue);
            }

            if (negative)
            {
                long long signedValue = std::stoll(L"-" + trimmed, nullptr, parseBase);
                return static_cast<unsigned long long>(signedValue);
            }

            return std::stoull(trimmed, nullptr, parseBase);
        }
        catch (...)
        {
            return 0;
        }
    }

private:
    static double ConvertFloat16ToDouble(unsigned short half)
    {
        unsigned int sign = (half >> 15) & 0x1;
        unsigned int exponent = (half >> 10) & 0x1F;
        unsigned int mantissa = half & 0x3FF;

        if (exponent == 0)
            return sign ? -0.0 : 0.0;
        if (exponent == 31)
            return (mantissa == 0) ? (sign ? -INFINITY : INFINITY) : NAN;

        return std::ldexp(static_cast<double>(1.0 + mantissa / 1024.0), exponent - 15);
    }

    static unsigned short ConvertDoubleToFloat16(double value)
    {
        int exponent;
        double mantissa = std::frexp(value, &exponent);

        unsigned int sign = std::signbit(value) ? 1 : 0;
        int expValue = (std::max)(-14, (std::min)(15, exponent - 1)) + 15;
        unsigned int exp = static_cast<unsigned int>(expValue);
        unsigned int mant = static_cast<unsigned int>((std::abs(mantissa) * 2.0 - 1.0) * 1024.0);

        return static_cast<unsigned short>((sign << 15) | (exp << 10) | (mant & 0x3FF));
    }
};
