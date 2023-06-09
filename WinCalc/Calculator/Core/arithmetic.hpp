#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

//other elements of library
#include "types.hpp"

namespace Core {
    bool isDigit(const wchar_t& symbol) {
        return (std::find(Types::digits.begin(), Types::digits.end(), symbol) != Types::digits.end());
    }
    bool isArithmeticSymbol(const wchar_t& symbol) {
        return (std::find(Types::arytmeticSymbols.begin(), Types::arytmeticSymbols.end(), symbol) != Types::arytmeticSymbols.end());
    }
    bool isCommaSymbol(const wchar_t& symbol) {
        return (std::find(Types::commaSymbols.begin(), Types::commaSymbols.end(), symbol) != Types::commaSymbols.end());
    }
    enum class arithmeticSymbolsTypes {
        leftSideOne,
        leftSideTwo,
        rightSideOne,
        undefinedSymbol
    };
    arithmeticSymbolsTypes arithmeticSymbolType(const wchar_t& symbol) {
        if (!isArithmeticSymbol(symbol))
            throw std::runtime_error("Symbol must exist in arithmetic symbol group");

        using enum arithmeticSymbolsTypes;

        switch (symbol)
        {
        case L'+':
        case L'-':
        case L'*':
        case L'×':
        case L'/':
        case L'÷':
        case L'%':
        case L'^':
            return leftSideTwo;
        case L'√':
        case L'∛':
            return leftSideOne;
        case L'!':
            return rightSideOne;
        default:
            return undefinedSymbol;
        }
    }
}