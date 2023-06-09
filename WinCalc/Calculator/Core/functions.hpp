#pragma once

#include <vector>
#include <stdexcept>
#include <numbers>

//other elements of library
#include "types.hpp"

namespace Core {
	bool isFunction(const std::wstring& name) {
		return (std::find(Types::functionNames.begin(), Types::functionNames.end(), name) != Types::functionNames.end());
	}
	Decimal functionEvaluator(const std::wstring& fName, Decimal arg) {
		Decimal result{ 0 };

		if (!isFunction(fName))
			throw std::runtime_error("Not recognized function name");
		else if (fName == L"sin") {
			arg %= 2 * Decimal::constants::pi;
			long double argD = arg.toLongDouble();
			result = std::sin(argD);
		}
		else if (fName == L"cos") {
			arg %= 2 * Decimal::constants::pi;
			long  argD = arg.toLongDouble();
			result = std::cos(argD);
		}
		else if (fName == L"tg") {
			arg %= 2 * Decimal::constants::pi;
			long double argD = arg.toLongDouble();

			if ((std::round(std::cos(argD) * 1000.0) / 1000.0) == 0)
				throw std::runtime_error("Unknown operation result");
			else
				result = std::tan(argD);
		}
		else if (fName == L"ctg") {
			arg %= 2 * Decimal::constants::pi;
			long double argD = arg.toLongDouble();

			if ((std::round(std::sin(argD) * 1000.0) / 1000.0) == 0)
				throw std::runtime_error("Unknown operation result");
			else {
				result = std::tan(argD);
				result = Decimal::inverse(result);
			}
		}
		else if (fName == L"log") {
			result = Decimal::log10(arg);
		}
		else if (fName == L"logb") {
			result = Decimal::log2(arg);
		}
		else if (fName == L"ln") {
			result = Decimal::ln(arg);
		}
		else throw std::runtime_error("Cannot resolve function");

		return result;
	}
	bool isConstant(const wchar_t& symbol) {
		return (std::find(Types::constants.begin(), Types::constants.end(), symbol) != Types::constants.end());
	}
	Decimal constantsEvaluator(const wchar_t& symbol) {
		if (!isConstant(symbol))
			throw std::runtime_error("Not recognized constant");
		else if (symbol == L'π')
			return Decimal::constants::pi;
		else if (symbol == L'e')
			return Decimal::constants::e;
		else if (symbol == L'γ')
			return Decimal::constants::egamma;
		else if (symbol == L'φ')
			return Decimal::constants::phi;
		else if (symbol == L'd')
			return Decimal::constants::dottie;
		else throw std::runtime_error("Cannot resolve constant");
	}
}
