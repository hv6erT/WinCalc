#pragma once

#include <vector>
#include <string>
#include <stdexcept>

//fixed-point type library
#include "Decimal/Decimal.hpp"

namespace Core {
	namespace Types {
		const std::vector<wchar_t> digits{ L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'0' };
		const std::vector<wchar_t> arytmeticSymbols{ L'+', L'-', L'*', L'×', L'/', L'÷', L'%', L'^', L'√', L'∛', L'!' };
		const std::vector<wchar_t> commaSymbols{ L',', L'.' };
		const std::vector<wchar_t> constants{ L'π', L'e', L'γ', L'φ', L'd'};

		const std::vector<std::wstring> functionNames{ L"sin", L"cos", L"tg", L"ctg", L"arcsin", L"arccos", L"arctg", L"arcctg", L"log", L"logb", L"ln" };
	}
	int64_t decPrecision() {
		return Decimal::maxDecPrecisionPoint - 1;
	}
	void decPrecision(int64_t precision) {
		if (precision < 0)
			throw std::runtime_error("Invalid precision value");

		Decimal::maxDecPrecisionPoint = precision + 1;
	}
}
