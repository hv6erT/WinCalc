#pragma once

//C++ standard libraries headers
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>

class Calculator {
public:
    std::wstring input{ L"" };
    std::wstring output{ L"0" };
    std::wstring notation{ L"0" };
    std::vector<std::wstring> history;

    Calculator();
    Calculator(std::wstring inp, std::wstring note);
    bool addToInput(wchar_t symbol);
    bool addToNotation(wchar_t symbol);
    bool addFunctionToNotation(std::wstring functionName);
    bool addBracketToNotation();
    bool addPipeToNotation();
    void tryPutBracketsIfMissing();
    bool pushInputToNotation();
    void popLastNumberFromNotationToInput();
    void clear();
public:
    enum class roundingType {
        noRounding,
        decimalPlaces,
        significantDigits,
        significantDigitsNotation
    };

    void resolve();
    struct resolveOptions {
        std::vector<roundingType> roundingOrder{ roundingType::noRounding, roundingType::decimalPlaces, roundingType::significantDigits, roundingType::significantDigitsNotation };
        std::vector<unsigned int> roundingOrderLimits{ 8, 16, 20};

        unsigned int decimalPlacesPrec = 4;
        unsigned int significantDigitsPrec = 6;
        unsigned int significantDigitsNotationPrec = 9;
    };
    void resolve(resolveOptions options);
public:
    static bool isDigit(const wchar_t& symbol);
    static bool isConstant(const wchar_t& symbol);
    static bool isFunction(const std::wstring& symbol);
    static bool isArithmeticSymbol(const wchar_t& symbol);
    static bool isCommaSymbol(const wchar_t& symbol);
};
