#include "pch.h"
#include "calculator.hpp"
#include "Core/core.hpp"

Calculator::Calculator() {}
Calculator::Calculator(std::wstring inp, std::wstring note) {
    this->input = inp;
    this->notation = note;
}

bool Calculator::addToInput(wchar_t symbol) {
    if (Core::isCommaSymbol(symbol)) {
        if (this->input.length() == 0) {
            this->input = L"0";
            this->input += symbol;
        }
        else {
            std::wstring commaSymbols{ L"" };
            for (unsigned int i = 0; i < Core::Types::commaSymbols.size(); i++)
                commaSymbols += Core::Types::commaSymbols.at(i);

            if (this->input.find_first_of(commaSymbols) == std::string::npos && Core::isConstant(this->input[this->input.length() - 1]) == false)
                this->input += symbol;
            else return false;
        }

        return true;
    }
    else if (Core::isDigit(symbol)) {
        std::wstring commonNumSymbols{ L"-" };

        for (unsigned int i = 0; i < Core::Types::commaSymbols.size(); i++)
            commonNumSymbols += Core::Types::commaSymbols.at(i);

        for (unsigned int i = 0; i < Core::Types::digits.size(); i++)
            commonNumSymbols += Core::Types::digits.at(i);

        if (this->input.length() == 0 || this->input == L"0")
            this->input = symbol;
        else {
            if (this->input.find_first_not_of(commonNumSymbols) == std::string::npos)
                this->input += symbol;
            else return false;
        }

        return true;
    }
    else if (Core::isConstant(symbol)) {
        if (this->input.length() == 0 || this->input == L"0")
            this->input = symbol;
        else return false;

        return true;
    }
    //else: cannot paste arithmetic symbols to input
    return false;
}

bool Calculator::addToNotation(wchar_t symbol) {
    if (Core::isArithmeticSymbol(symbol)) {
        if (this->notation.length() == 0 || this->notation == L"0") {
            if (Core::arithmeticSymbolType(symbol) == Core::arithmeticSymbolsTypes::leftSideOne) {
                this->notation = symbol;
            }
            else {
                this->notation = L'0';
                this->notation += symbol;
            }
        }
        else {
            //if length() > 0 access to previous char would not be denied
            wchar_t previousSymbol = this->notation[this->notation.length() - 1];

            if (Core::isArithmeticSymbol(previousSymbol) && Core::arithmeticSymbolType(previousSymbol) == Core::arithmeticSymbolType(symbol)) {
                //handle dynamic arythmetic symbol changing
                this->notation = this->notation.substr(0, (this->notation.length() - 1)) + symbol;
            }
            else if (Core::isArithmeticSymbol(previousSymbol) && Core::arithmeticSymbolType(previousSymbol) == Core::arithmeticSymbolsTypes::leftSideTwo && Core::arithmeticSymbolType(symbol) == Core::arithmeticSymbolsTypes::leftSideOne) {
                //handle such case +√4
                this->notation += symbol;
            }
            else if (Core::isArithmeticSymbol(previousSymbol) && Core::arithmeticSymbolType(previousSymbol) == Core::arithmeticSymbolsTypes::rightSideOne && Core::arithmeticSymbolType(symbol) == Core::arithmeticSymbolsTypes::leftSideTwo) {
                //handle such case 5!+
                this->notation += symbol;
            }
            else if (Core::isDigit(previousSymbol) || Core::isConstant(previousSymbol) || previousSymbol == L')') {
                //handle standard cases: ...+5*
                this->notation += symbol;
            }
            else if (previousSymbol == L'|' && std::count(this->notation.begin(), this->notation.end(), L'|') % 2 == 0) {
                //handle such case |6|+
                this->notation += symbol;
            }
            else
                return false;
        }
    }
    //else: it is only possible to addToNotation arithmetic symbols directly to notation
    return true;
}

bool Calculator::addFunctionToNotation(std::wstring functionName) {
    if (!Core::isFunction(functionName))
        return false;

    if (this->notation.length() == 0 || this->notation == L"0") {
        this->notation = functionName;
        this->notation += L"(";

        return true;
    }
    else{
        wchar_t lastSymbol = this->notation[this->notation.length() - 1];
        if (Core::isArithmeticSymbol(lastSymbol) && (Core::arithmeticSymbolType(lastSymbol) == Core::arithmeticSymbolsTypes::leftSideOne || Core::arithmeticSymbolType(lastSymbol) == Core::arithmeticSymbolsTypes::leftSideTwo)) {
            this->notation += functionName;
            this->notation += L"(";

            return true;
        }
    }

    return false;
}

bool Calculator::addBracketToNotation() {
    /*
        This function will probably always return true. Although in future there might be cases when it would return false.
    */
    if (this->notation.length() > 0 && this->notation != L"0") {
        wchar_t lastSymbolOfNote = this->notation[this->notation.length() - 1];
        if (Core::isDigit(lastSymbolOfNote) || Core::isConstant(lastSymbolOfNote) || lastSymbolOfNote == L')' || lastSymbolOfNote == L'|') {
            std::string::difference_type numberOfOpenBrackets = std::count(this->notation.begin(), this->notation.end(), L'(');
            std::string::difference_type numberOfCloseBrackets = std::count(this->notation.begin(), this->notation.end(), L')');

            if (numberOfOpenBrackets == numberOfCloseBrackets)
                this->notation += L"×(";
            else
                this->notation += L')';

            return true;
        }
        else if (Core::isArithmeticSymbol(lastSymbolOfNote) || lastSymbolOfNote == L'(') {
            this->notation += L'(';
            return true;
        }
    }
    else {
        this->notation = L"(";
        return true;
    }

    return false;
}

bool Calculator::addPipeToNotation() {
    /*
        This function will probably always return true. Although in future there might be cases when it would return false.
    */
    if (this->notation.length() > 0 && this->notation != L"0") {
        wchar_t lastSymbolOfNote = this->notation[this->notation.length() - 1];
        if (Core::isDigit(lastSymbolOfNote) || Core::isConstant(lastSymbolOfNote) || lastSymbolOfNote == L')') {
            std::string::difference_type numberOfPipes = std::count(this->notation.begin(), this->notation.end(), L'|');

            if (numberOfPipes % 2 == 0)
                this->notation += L"×|";
            else
                this->notation += L'|';

            return true;
        }
        else if (Core::isArithmeticSymbol(lastSymbolOfNote)) {
            std::string::difference_type numberOfPipes = std::count(this->notation.begin(), this->notation.end(), L'|');

            if(Core::arithmeticSymbolType(lastSymbolOfNote) == Core::arithmeticSymbolsTypes::rightSideOne && numberOfPipes % 2 == 1)
                this->notation += L"|";
            else if(Core::arithmeticSymbolType(lastSymbolOfNote) == Core::arithmeticSymbolsTypes::leftSideTwo && numberOfPipes % 2 == 0)
                this->notation += L"|";
        }
        else if(lastSymbolOfNote == L'(')
            this->notation += L"|";
    }
    else {
        this->notation = L"|";
        return true;
    }

    return false;
}

void Calculator::tryPutBracketsIfMissing() {
    std::string::difference_type numberOfOpenBrackets = std::count(this->notation.begin(), this->notation.end(), L'(');
    std::string::difference_type numberOfCloseBrackets = std::count(this->notation.begin(), this->notation.end(), L')');
    std::string::difference_type numberOfPipes = std::count(this->notation.begin(), this->notation.end(), L'|');

    if (numberOfOpenBrackets > numberOfCloseBrackets && numberOfPipes % 2 != 0) {
        size_t indexOfFirstPipe = this->notation.find(L'|');
        size_t indexOfFirstOpenBracket = this->notation.find(L'(');

        if (indexOfFirstPipe < indexOfFirstOpenBracket) {
            for (size_t i = 0; i < (numberOfOpenBrackets - numberOfCloseBrackets); i++)
                this->notation += L")";

            this->notation += L"|";
        }
        else {
            this->notation += L"|";

            for (size_t i = 0; i < (numberOfOpenBrackets - numberOfCloseBrackets); i++)
                this->notation += L")";
        }
    }
    else if (numberOfOpenBrackets > numberOfCloseBrackets) {
        for (size_t i = 0; i < (numberOfOpenBrackets - numberOfCloseBrackets); i++)
            this->notation += L")";
    }
    else if(numberOfPipes % 2 != 0)
        this->notation += L"|";
}

bool Calculator::pushInputToNotation() {
    if (this->input.length() == 0)
        return false;

    //prevent pushing wrong forms of factors
    if (Core::isCommaSymbol(this->input[this->input.length() - 1])) {
        this->input += L"0";
    }

    if (this->notation.length() == 0 || this->notation == L"0")
        this->notation = this->input;
    else {
        //if length() > 0 access to previous char would not be denied
        wchar_t lastSymbolFromNote = this->notation[(this->notation.length() - 1)];

        if (lastSymbolFromNote == L')' || (lastSymbolFromNote == L'|' && std::count(this->notation.begin(), this->notation.end(), L'|') % 2 == 0)) {
            // handle auto × if other not specified
            this->addToNotation(L'×');
        }
        else if (Core::isDigit(lastSymbolFromNote) || Core::isConstant(lastSymbolFromNote)) {
            this->notation = this->input;
            this->input = L"";
            return true;
        }

        //handle input lower than 0 values
        if (this->input[0] == L'-' && this->input.length() > 1 && Core::isDigit(this->input[1])) { 
            //special handle lower than 0 numbers input
            if(lastSymbolFromNote != L'(')
                this->notation += L"(";
            this->notation += this->input + L")";
            this->input = L"";
            return true;
        }
        else if (this->input[0] == L'-') return false; // prevent pushing only minus without digit

        this->notation += this->input;
    }

    this->input = L"";
    return true;
}

void Calculator::popLastNumberFromNotationToInput() {
    std::wstring numSymbols{ L"" };

    for (unsigned int i = 0; i < Core::Types::digits.size(); i++)
        numSymbols += Core::Types::digits.at(i);

    for (unsigned int i = 0; i < Core::Types::constants.size(); i++)
        numSymbols += Core::Types::constants.at(i);

    size_t indexOfLastDigitOfPreviousNumber = this->notation.find_last_of(numSymbols);

    size_t indexOfFirstDigitOfPreviousNumber{ 0 };

    for (int i = (indexOfLastDigitOfPreviousNumber - 1); i >= 0; i--) {
        if (Core::isDigit(notation[i]) == false && Core::isCommaSymbol(notation[i]) == false) {
            indexOfFirstDigitOfPreviousNumber = i + 1;

            //handle lower than 0 numbers
            if (indexOfFirstDigitOfPreviousNumber > 0 && this->notation[(indexOfFirstDigitOfPreviousNumber - 1)] == L'-') {
                if (indexOfFirstDigitOfPreviousNumber == 1) // handle such case: -5+...
                    indexOfFirstDigitOfPreviousNumber--;
                else if (indexOfFirstDigitOfPreviousNumber >= 2) {
                    if (this->notation[(indexOfFirstDigitOfPreviousNumber - 2)] == L'(') //handle such case: .../(-5+...
                        indexOfFirstDigitOfPreviousNumber--;
                    else if (Core::isArithmeticSymbol(notation[(indexOfFirstDigitOfPreviousNumber - 2)])) //handle such case: .../-5
                        indexOfFirstDigitOfPreviousNumber--;
                }
            }

            goto next;
        }
    }

next:

    std::wstring newInput = this->notation.substr(indexOfFirstDigitOfPreviousNumber, (indexOfLastDigitOfPreviousNumber - indexOfFirstDigitOfPreviousNumber + 1));
    if (newInput != L"0")
        this->input = newInput;

    this->notation = this->notation.substr(0, indexOfFirstDigitOfPreviousNumber);

    if (this->notation.length() == 0)
        this->notation = L"0";
}
void Calculator::clear() {
    this->input = L"";
    this->notation = L"0";
}

void Calculator::resolve() {
    resolveOptions options = {};
    Calculator::resolve(options);
}

void Calculator::resolve(resolveOptions options) {
    if (this->notation.length() == 0)
        return;

    Decimal result;

    try {
        result = Core::expressionEvaluator(this->notation);
    }
    catch (std::runtime_error& error) {
        std::string eMessage = error.what();
        if (eMessage.length() == 0)
            this->output = L"Unexpeced error";
        else if (eMessage == "Divide by 0 is unreachable")
            this->output = L"Cannot divide by 0";
        else this->output = std::wstring(eMessage.begin(), eMessage.end());

        return;
    }

    //set result if input is correct
    if (this->history.size() == 0 || this->history[this->history.size() - 1] != this->notation)
        this->history.push_back(this->notation);

    roundingType rType{ options.roundingOrder[options.roundingOrder.size() - 1]};
    for (unsigned int i = 0; i < (options.roundingOrder.size() - 1); i++) {
        if (options.roundingOrderLimits[i] >= result.size()) {
            rType = options.roundingOrder[i];
            goto round;
        }
    }

    round:

    std::wstring resultWstr{ L"" };
    if (rType == roundingType::noRounding)
        resultWstr = result.toWstring();
    else if (rType == roundingType::decimalPlaces)
        resultWstr = Decimal::roundToDecPlaces(result, options.decimalPlacesPrec).toWstring();
    else if (rType == roundingType::significantDigits)
        resultWstr = Decimal::roundToSignificantDigits(result, options.significantDigitsPrec).toWstring();
    else if(rType == roundingType::significantDigitsNotation)
        resultWstr = Decimal::roundToSignificantDigits(result, options.significantDigitsPrec).toNotationWstring();

    this->output = resultWstr;
}

bool Calculator::isDigit(const wchar_t& symbol) { return Core::isDigit(symbol); }
bool Calculator::isConstant(const wchar_t& symbol) { return Core::isConstant(symbol); }
bool Calculator::isFunction(const std::wstring& symbol) { return Core::isFunction(symbol); }
bool Calculator::isArithmeticSymbol(const wchar_t& symbol) { return Core::isArithmeticSymbol(symbol); }
bool Calculator::isCommaSymbol(const wchar_t& symbol) { return Core::isCommaSymbol(symbol); }