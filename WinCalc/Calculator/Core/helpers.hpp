#pragma once

#include <string>

namespace Core {
    std::wstring getExpressionInsideBrackets(std::wstring expression, size_t startI) {
        size_t indexOfOpenBracket = expression.find(L'(', startI);
        if (indexOfOpenBracket == std::string::npos)
            return L"";

        size_t indexOfCloseBracket = expression.length() - 1;

        size_t numberOfAdditionalOpenBrackets{ 0 };
        for (size_t y = (indexOfOpenBracket + 1); y < expression.length(); y++) {
            wchar_t processedChar = expression[y];

            if (processedChar == L'(')
                numberOfAdditionalOpenBrackets++;
            else if (processedChar == L')') {
                if (numberOfAdditionalOpenBrackets > 0)
                    numberOfAdditionalOpenBrackets--;
                else {
                    indexOfCloseBracket = y;
                    goto next;
                }
            }
        }

    next:

        return expression.substr((indexOfOpenBracket + 1), (indexOfCloseBracket - indexOfOpenBracket - 1));
    }
    std::wstring getExpressionInsidePipes(std::wstring expression, size_t startI) {
        size_t indexOfOpenPipe = expression.find(L'|', startI);
        if (indexOfOpenPipe == std::string::npos)
            return L"";

        size_t indexOfClosePipe = expression.length() - 1;

        size_t numberOfAdditionalPipes{ 1 }; // there is one open pipe - in indexOfOpenPipe position. This variable can only hold 0 or 1

        for (size_t y = (indexOfOpenPipe + 1); y < expression.length(); y++) {
            wchar_t processedChar = expression[y];

            if (processedChar == L'|') {
                if (numberOfAdditionalPipes == 0)
                    numberOfAdditionalPipes++;
                else if (numberOfAdditionalPipes == 1) {
                    numberOfAdditionalPipes--;
                    indexOfClosePipe = y;
                }
            }
        }

        return expression.substr((indexOfOpenPipe + 1), (indexOfClosePipe - indexOfOpenPipe - 1));
    }
}
