#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <stdexcept>

//other elements of library
#include "types.hpp"
#include "arithmetic.hpp"
#include "functions.hpp"
#include "helpers.hpp"

namespace Core {
    Decimal expressionEvaluator(std::wstring expression) {
        if (expression.length() == 0)
            return Decimal(0);

        std::vector<Decimal> numberStack;
        std::vector<wchar_t> operandStack;

        //parser and primary evaluator (evaluating bracket sign expressions, functions)
        for (size_t i = 0; i < expression.length(); i++) {
            wchar_t previousChar = L' ';
            if (i != 0)
                previousChar = expression[i - 1];

            wchar_t currentChar = expression[i];

            if (currentChar == L'(') { // handle bracket sign, the bracket expression will be evaluated in new context
                std::wstring newExpression = getExpressionInsideBrackets(expression, i);
                numberStack.push_back(expressionEvaluator(newExpression)); //create new context to evaluate breacket expression

                i += (newExpression.length() + 1);
            }
            else if (currentChar == L'|') {
                std::wstring newExpression = getExpressionInsidePipes(expression, i);
                numberStack.push_back(Decimal::abs(expressionEvaluator(newExpression))); //create new context to evaluate breacket expression

                i += (newExpression.length() + 1);
            }
            else if (currentChar == L'-' && (isArithmeticSymbol(previousChar) || previousChar == L' ')) { //handle low than 0 nums (with "-" as first char)
                size_t indexOfFirstDigit = i;
                size_t indexOfLastDigit = expression.find_first_not_of(L"1234567890,.", indexOfFirstDigit + 1) - 1;

                std::wstring numStr = expression.substr(indexOfFirstDigit, (indexOfLastDigit - indexOfFirstDigit + 1));
                if (numStr.find(L",") != std::string::npos)
                    numStr.replace(numStr.find(L","), 1, L".");

                Decimal num(std::string(numStr.begin(), numStr.end()));
                numberStack.push_back(num);

                i += (indexOfLastDigit - indexOfFirstDigit);
            }
            else if (isDigit(currentChar)) { //handle nums 
                size_t indexOfFirstDigit = i;
                size_t indexOfLastDigit = expression.find_first_not_of(L"1234567890,.", indexOfFirstDigit) - 1;

                std::wstring numStr = expression.substr(indexOfFirstDigit, (indexOfLastDigit - indexOfFirstDigit + 1));
                if (numStr.find(L",") != std::string::npos)
                    numStr.replace(numStr.find(L","), 1, L".");

                Decimal num(std::string(numStr.begin(), numStr.end()));
                numberStack.push_back(num);

                i += (indexOfLastDigit - indexOfFirstDigit);
            }
            else if (isArithmeticSymbol(currentChar)) {
                operandStack.push_back(currentChar);
            }
            else if (isConstant(currentChar)) {
                numberStack.push_back(constantsEvaluator(currentChar));
            }
            else if (static_cast<unsigned short>(currentChar) >= 97 && static_cast<unsigned short>(currentChar) <= 122) { //handle functions
                std::wstring funcName{ L"" };
                Decimal funcArg{ 0 };

                for (size_t y = i; y < expression.length(); y++) { //get function name
                    wchar_t currChar = expression[y];
                    unsigned short currCharCode = static_cast<unsigned short>(currChar);
                    if (currCharCode >= 97 && currCharCode <= 122) {
                        funcName += currChar;
                        i++;
                    }
                    else break;
                }

                if (expression[i] == L'(') {
                    std::wstring newExpression = getExpressionInsideBrackets(expression, i);
                    funcArg = expressionEvaluator(newExpression); //create new context to evaluate breacket expression

                    i += newExpression.length();

                    if (expression[i + 1] == L')')
                        i++;
                    else throw std::runtime_error("Wrong specified argument");

                    //invoke function
                    numberStack.push_back(functionEvaluator(funcName, funcArg));

                }
                else
                    throw std::runtime_error("Specified function need an argument");
            }
            else if (static_cast<unsigned short>(currentChar) != 32) // handle spaces
                throw std::runtime_error("Invalid character in expression");
        }
        //tests for parser
        /*
        std::cout << "Operator stack: ";
        for (size_t i = 0; i < operandStack.size(); i++)
            std::wcout << operandStack[i] << L" ";
        std::cout << std::endl;
        std::cout << "Number stack: ";
        for (size_t i = 0; i < numberStack.size(); i++)
            std::cout << numberStack[i] << " ";
        std::cout << std::endl;
        */

        //basic validator
        if (std::count(expression.begin(), expression.end(), L'(') != std::count(expression.begin(), expression.end(), L')'))
            throw std::runtime_error("Invalid expression");

        //precedence mechanism
        std::vector<unsigned short> precedenceStack;
        unsigned short maxPrecedence{ 0 };

        for (size_t i = 0; i < operandStack.size(); i++) {
            wchar_t& op = operandStack[i];

            if (op == L'+' || op == L'-')
                precedenceStack.push_back(1);
            else if (op == L'*' || op == L'×' || op == L'/' || op == L'÷' || op == L'%')
                precedenceStack.push_back(2);
            else if (op == L'^' || op == L'√' || op == L'∛')
                precedenceStack.push_back(3);
            else if (op == L'!')
                precedenceStack.push_back(4);
        }
        if (precedenceStack.size() > 0)
            maxPrecedence = *std::max_element(precedenceStack.begin(), precedenceStack.end());

        //evaluator
        for (unsigned short prec = maxPrecedence; prec >= 1; prec--) {
            for (size_t i = 0; i < precedenceStack.size(); i++) {
                //resolver
                if (precedenceStack[i] == prec) {
                    //tests for evaluator
                    /*
                    std::cout << "Number stack: ";
                    for (size_t z = 0; z < numberStack.size(); z++)
                        std::cout << numberStack[z] << " ";
                    std::cout << std::endl;
                    std::cout << "Operand stack: ";
                    for (size_t z = 0; z < operandStack.size(); z++)
                        std::wcout << operandStack[z] << " ";
                    std::cout << std::endl;
                    std::cout << "Precedence stack: ";
                    for (size_t z = 0; z < precedenceStack.size(); z++)
                        std::cout << precedenceStack[z] << " ";
                    std::cout << std::endl;
                    */

                    if (i >= numberStack.size()) {
                        throw std::runtime_error("Invalid expression");
                    }

                    const auto operandType = arithmeticSymbolType(operandStack[i]);
                    if (operandType == arithmeticSymbolsTypes::undefinedSymbol)
                        throw std::runtime_error("Undefined operad type");
                    else if (operandType == arithmeticSymbolsTypes::leftSideTwo) {
                        if ((i+1) >= numberStack.size()) {
                            throw std::runtime_error("Invalid expression");
                        }

                        if (operandStack[i] == L'^')
                            numberStack[i] = Decimal::roundToDecPlaces(Decimal::pow(numberStack[i], numberStack[i + 1]), Decimal::maxDecPrecisionPoint-1);
                        else if (operandStack[i] == L'*' || operandStack[i] == L'×')
                            numberStack[i] = numberStack[i] * numberStack[i + 1];
                        else if (operandStack[i] == L'/' || operandStack[i] == L'÷') {
                            if (numberStack[i + 1] != 0)
                                numberStack[i] = numberStack[i] / numberStack[i + 1];
                            else throw std::runtime_error("Divide by 0 is unreachable");
                        }
                        else if (operandStack[i] == L'%') {
                            if (numberStack[i + 1] != 0)
                                numberStack[i] = numberStack[i] % numberStack[i + 1];
                            else throw std::runtime_error("Modulo from dividing by 0 is unreachable");
                        }
                        else if (operandStack[i] == L'+')
                            numberStack[i] = numberStack[i] + numberStack[i + 1];
                        else if (operandStack[i] == L'-')
                            numberStack[i] = numberStack[i] - numberStack[i + 1];

                        numberStack.erase((numberStack.begin() + i + 1));
                    }
                    else if (operandType == arithmeticSymbolsTypes::rightSideOne) {
                        if (operandStack[i] == L'!') {
                            Decimal result{ 1 };
                            if (numberStack[i] == 0) //handle 0!
                                result = 0;

                            for (Decimal y{ 1 }; y <= numberStack[i]; y++) {
                                result = result * y;
                            }

                            numberStack[i] = result;
                        }
                    }
                    else if (operandType == arithmeticSymbolsTypes::leftSideOne) {
                        if (operandStack[i] == L'√')
                            numberStack[i] = Decimal::root(numberStack[i], 2);
                        else if (operandStack[i] == L'∛')
                            numberStack[i] = Decimal::root(numberStack[i], 3);
                    }

                    operandStack.erase((operandStack.begin() + i));
                    precedenceStack.erase((precedenceStack.begin() + i));
                    i--;
                }
            }
        }

        return numberStack[0];
    }
}