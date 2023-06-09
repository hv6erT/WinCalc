#pragma once

#include <string>
#include <deque>
#include <stdexcept>
#include <cmath>
#include <concepts>

template<typename T>
concept arithmetic = std::integral<T> or std::floating_point<T>;

using maxDigitsNumberType = int_fast64_t;
using digitType = int16_t;

class Decimal {
private:
	std::deque<digitType> value;
	maxDigitsNumberType decPoint{ 0 }; //counting from right
	bool sign = 1; // 0 means '-' 1 means '+'
public:
	//Constructors and destructor
	Decimal() {
		this->value.push_back(0);
	}
	Decimal(std::string numStr) {
		if (numStr.length() > 0) {
			if (numStr[0] == '-') {
				this->sign = 0;
				numStr = numStr.substr(1);
			}
			else if (numStr[0] == '+') { // suport for +notation, for example: +567
				this->sign = 1;
				numStr = numStr.substr(1);
			}
		}
		else {
			this->sign = 1;
			this->value.push_back(0);
			return;
		}

		bool isDecPart = false;
		for (char numDigit : numStr) {
			if (isDecPart)
				this->decPoint++;
			
			if (numDigit == '.' || numDigit == ',')
				isDecPart = true;
			else if (static_cast<uint16_t>(numDigit) >= 48 && static_cast<uint16_t>(numDigit) <= 57)
				this->value.push_back(std::stoi(std::string(1, numDigit)));
			else throw std::runtime_error("Unexpected char in string");
		}

		this->unoverflow();
	}
	Decimal(std::wstring numWstr) {
		if (numWstr.length() > 0) {
			if (numWstr[0] == L'-') {
				this->sign = 0;
				numWstr = numWstr.substr(1);
			}
			else if (numWstr[0] == L'+') { // suport for +notation, for example: +567
				this->sign = 1;
				numWstr = numWstr.substr(1);
			}
		}
		else {
			this->sign = 1;
			this->value.push_back(0);
			return;
		}

		bool isDecPart = false;
		for (wchar_t numDigit : numWstr) {
			if (isDecPart)
				this->decPoint++;

			if (numDigit == L'.' || numDigit == L',')
				isDecPart = true;
			else if (static_cast<uint16_t>(numDigit) >= 48 && static_cast<uint16_t>(numDigit) <= 57)
				this->value.push_back(std::stoi(std::wstring(1, numDigit)));
			else throw std::runtime_error("Invalid wchar in wstring");
		}

		this->unoverflow();
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal(convType num) {
		if (num < 0) {
			this->sign = 0;
			num *= -1;
		}
		else if (num == 0) {
			this->value.push_back(0);
			return;
		}

		uint64_t numInt{ static_cast<uint64_t>(num) };

		while (std::fmod(num, 1.0) != 0) {
			num *= 10;

			numInt *= 10;
			numInt += std::fmod(num, 10);

			this->decPoint++;
		}

		while (numInt != 0) {
			this->value.push_front(numInt % 10);

			numInt /= 10;
		}

		this->applyPrecision();
	}
	~Decimal() {
	
	}
	//Basic math methods and operands
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator= (convType right){
		Decimal rightDec = Decimal(right);

		this->value = rightDec.value;
		
		this->decPoint = rightDec.decPoint;
		this->sign = rightDec.sign;

		return *this;
	}
	bool operator== (const Decimal& right) const{
		// DANGER: this function might return wrong result if numbers have digits overflow
		// INFO: use unoverflow() method after all math operations to prevent such a situations

		maxDigitsNumberType leftSize = this->value.size();
		maxDigitsNumberType rightSize = right.value.size();

		maxDigitsNumberType leftDecPoint = this->decPoint;
		maxDigitsNumberType rightDecPoint = right.decPoint;

		if (leftSize == 1 && rightSize == 1 && leftDecPoint == 0 && rightDecPoint == 0 && this->value[0] == 0 && right.value[0] == 0) // special handle for 0 == (-0)
			return true;
		else if (this->sign != right.sign)
			return false;

		if (leftDecPoint != rightDecPoint || leftSize != rightSize)
			return false;

		for (maxDigitsNumberType i = 0; i < leftSize; i++) {
			if (this->value.at(i) != right.value.at(i))
				return false;
		}

		return true;
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator== (const convType& right) const {
		return (*this) == Decimal(right);
	}
	bool operator!= (const Decimal& right) const {
		return !(*this == right);
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator!= (const convType& right) const {
		return (*this) != Decimal(right);
	}
	bool operator> (const Decimal& right) const{
		// DANGER: this function might return wrong result if numbers have digits overflow
		// INFO: use unoverflow() method after all math operations to prevent such a situations

		maxDigitsNumberType leftSize = this->value.size();
		maxDigitsNumberType rightSize = right.value.size();

		if (leftSize == 1 && rightSize == 1 && this->decPoint == 0 && right.decPoint == 0 && this->value[0] == 0 && right.value[0] == 0) // special handle for 0 == (-0)
			return false;
		else if (this->sign == true && right.sign == false)
			return true;
		else if(this->sign == false && right.sign == true)
			return false;

		maxDigitsNumberType leftIntSize = leftSize - this->decPoint;
		maxDigitsNumberType rightIntSize = rightSize - right.decPoint;

		bool signLeftRight = this->sign; // = right.sign

		if (leftIntSize > rightIntSize)
			return true;
		else if(leftIntSize < rightIntSize)
			return false;

		for (maxDigitsNumberType i = 0; i < std::max(leftSize, rightSize); i++) {
			digitType digitLeft{ 0 };
			digitType digitRight{ 0 };

			if (i < leftSize)
				digitLeft = this->value.at(i);

			if (i < rightSize)
				digitRight = right.value.at(i);

			if (signLeftRight == true) {
				if (digitLeft > digitRight)
					return true;
				else if (digitLeft < digitRight)
					return false;
			}
			else {
				if (digitLeft > digitRight)
					return false;
				else if (digitLeft < digitRight)
					return true;
			}
		}

		return false;
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator> (const convType& right) const {
		return (*this) > Decimal(right);
	}
	bool operator>= (const Decimal& right) const{
		return ((*this) > right || (*this) == right);
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator>= (const convType& right) const {
		return (*this) >= Decimal(right);
	}
	bool operator< (const Decimal& right) const {
		// DANGER: this function might return wrong result if numbers have digits overflow
		// INFO: use unoverflow() method after all math operations to prevent such a situations

		maxDigitsNumberType leftSize = this->value.size();
		maxDigitsNumberType rightSize = right.value.size();

		if (leftSize == 1 && rightSize == 1 && this->decPoint == 0 && right.decPoint == 0 && this->value[0] == 0 && right.value[0] == 0) // special handle for 0 == (-0)
			return false;
		else if (this->sign == true && right.sign == false)
			return false;
		else if (this->sign == false && right.sign == true)
			return true;

		bool signLeftRight = this->sign; // = right.sign

		maxDigitsNumberType leftIntSize = leftSize - this->decPoint;
		maxDigitsNumberType rightIntSize = rightSize - right.decPoint;

		if (leftIntSize > rightIntSize)
			return false;
		else if(leftIntSize < rightIntSize) 
			return true;

		for (maxDigitsNumberType i = 0; i < std::max(leftSize, rightSize); i++) {
			digitType digitLeft{ 0 };
			digitType digitRight{ 0 };

			if (i < leftSize)
				digitLeft = this->value.at(i);

			if (i < rightSize)
				digitRight = right.value.at(i);

	
			if (signLeftRight == true) {
				if (digitLeft > digitRight)
					return false;
				else if (digitLeft < digitRight)
					return true;
			}
			else {
				if (digitLeft > digitRight)
					return true;
				else if (digitLeft < digitRight)
					return false;
			}
		}

		return false;
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator< (const convType& right) const {
		return (*this) < Decimal(right);
	}
	bool operator<= (const Decimal& right) const {
		return ((*this) < right || (*this) == right);
	}
	template <typename convType>
		requires arithmetic<convType>
	bool operator<= (const convType& right) const {
		return (*this) <= Decimal(right);
	}
	friend Decimal operator+ (const Decimal& left, const Decimal& right) {
		Decimal result;

		maxDigitsNumberType leftSize = left.value.size();
		maxDigitsNumberType rightSize = right.value.size();

		maxDigitsNumberType leftIntSize = leftSize - left.decPoint;
		maxDigitsNumberType rightIntSize = rightSize - right.decPoint;

		//perform nums to add
		for (maxDigitsNumberType i = result.value.size(); i <= std::max(leftIntSize, rightIntSize); i++)
			result.value.push_front(0);

		for (maxDigitsNumberType i = result.value.size(); i <= std::max(leftSize, rightSize); i++)
			result.value.push_back(0);

		result.decPoint = std::max(left.decPoint, right.decPoint);

		maxDigitsNumberType resultSize = result.value.size();
		maxDigitsNumberType resultIntSize = resultSize - result.decPoint;

		for (maxDigitsNumberType i = ((-1) * resultIntSize); i < std::max(left.decPoint, right.decPoint); i++) {
			maxDigitsNumberType leftI = leftIntSize + i;
			maxDigitsNumberType rightI = rightIntSize + i;
			maxDigitsNumberType resultI = resultIntSize + i;

			if (leftSize > leftI && leftI >= 0) {
				if (left.sign == true)
					result.value[resultI] += left.value.at(leftI);
				else
					result.value[resultI] -= left.value.at(leftI);
			}

			if (rightSize > rightI && rightI >= 0) {
				if (right.sign == true)
					result.value[resultI] += right.value.at(rightI);
				else
					result.value[resultI] -= right.value.at(rightI);
			}
		}		

		result.unoverflow();
		return result;
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator+ (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left + rightDec);
	}
	friend Decimal operator- (const Decimal& left, const Decimal& right) {
		Decimal secD = right;
		secD.sign = !right.sign;

		return (left + secD);
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator- (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left - rightDec);
	}
	friend Decimal operator* (const Decimal& left, const Decimal& right) {
		Decimal result;

		maxDigitsNumberType leftSize = left.value.size();
		maxDigitsNumberType rightSize = right.value.size();

		maxDigitsNumberType leftIntSize = leftSize - left.decPoint;
		maxDigitsNumberType rightIntSize = rightSize - right.decPoint;

		for (maxDigitsNumberType i = result.value.size(); i < (leftSize + rightSize - 1); i++)
			result.value.push_back(0);

		result.decPoint = left.decPoint + right.decPoint;

		//table mathod implementation
		for (maxDigitsNumberType x = 0; x < leftSize; x++) {
			for (maxDigitsNumberType y = 0; y < rightSize; y++) {
				result.value[(x + y)] += (left.value[x] * right.value[y]);
			}
		}

		if (left.sign != right.sign)
			result.sign = false;
		else
			result.sign = true;

		result.unoverflow();
		return result;
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator* (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left * rightDec);
	}
	friend Decimal operator/ (const Decimal& left, const Decimal&right) {
		// DANGER: this function might case small precision lost due the periodic fractions
		// INFO: you cannot eliminate such a lost, but it is possible to minimalize it using Decimal.maxDecPrecisionPoint

		//DANGER: settings Decimal.maxDecPrecisionPoint to high value might cause performance problems or memory errors

		if (right == 0)
			throw std::runtime_error("Cannot divide by 0");

		Decimal result;
		Decimal buffer;

		Decimal divider = right;
		divider.sign = true;

		Decimal number = left;
		number.sign = true;

		while (divider.decPoint != 0 || number.decPoint != 0) {
			divider = divider * 10;
			number = number * 10;
		}
		
		for (maxDigitsNumberType i = 0; i < number.value.size(); i++) {
			result.value.push_back(0);
			buffer.value.push_back(0);

			if(i < number.value.size())
				buffer = buffer + number.value[i];

			while (buffer >= divider) {
				result = result + 1;
				buffer = buffer - divider;
			}
		}
		
		maxDigitsNumberType index = result.value.size() - result.decPoint;
		while (buffer != 0) {
			//prevent infinite loop when periodic fractions are in buffer
			if (result.decPoint >= Decimal::maxDecPrecisionPoint)
				goto finishLoop;

			buffer.value.push_back(0);
			result.value.push_back(0);

			while (buffer >= divider) {
				result.value[index]++;
				buffer = buffer - divider;
			}

			result.decPoint++;
			
			index++;
		}

		finishLoop:

		if (left.sign != right.sign)
			result.sign = false;
		else
			result.sign = true;

		result.unoverflow();
		return result;
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator/ (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left / rightDec);
	}
	friend Decimal operator% (const Decimal& left, const Decimal& right) {
		if (right == 0)
			throw std::runtime_error("Cannot modulo by 0");

		Decimal divider = right;
		divider.sign = true;

		Decimal result = left;
		result.sign = true;

		while (result >= divider)
			result = result - divider;

		if (left.sign != right.sign)
			result.sign = false;
		else
			result.sign = true;

		return result;
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator% (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left % rightDec);
	}
	friend Decimal operator^ (const Decimal& left, const Decimal& right) {
		return Decimal::pow(left, right);
	}
	template <typename convType>
		requires arithmetic<convType>
	friend Decimal operator^ (Decimal left, convType right) {
		Decimal rightDec = Decimal(right);
		return (left ^ rightDec);
	}
	//Simple operators
	Decimal& operator+= (const Decimal& num) {
		(*this) = (*this) + num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator+= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) += numDec);
	}
	Decimal& operator-= (const Decimal& num) {
		(*this) = (*this) - num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator-= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) -= numDec);
	}
	Decimal& operator*= (const Decimal& num) {
		(*this) = (*this) * num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator*= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) *= numDec);
	}
	Decimal& operator/= (const Decimal& num) {
		(*this) = (*this) / num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator/= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) /= numDec);
	}
	Decimal& operator%= (const Decimal& num) {
		(*this) = (*this) % num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator%= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) %= numDec);
	}
	Decimal& operator^= (const Decimal& num) {
		(*this) = (*this) ^ num;
		return (*this);
	}
	template <typename convType>
		requires arithmetic<convType>
	Decimal& operator^= (convType num) {
		Decimal numDec = Decimal(num);
		return ((*this) ^= numDec);
	}
	Decimal& operator++ () {
		maxDigitsNumberType i = this->value.size() - this->decPoint - 1;
		this->value[i]++;
		this->unoverflow();

		return (*this);
	}
	Decimal operator++(int)
	{
		Decimal temp = *this;
		++(*this);
		return temp;
	}
	Decimal& operator-- () {
		maxDigitsNumberType i = this->value.size() - this->decPoint - 1;
		this->value[i]--;
		this->unoverflow();

		return (*this);
	}
	Decimal operator--(int)
	{
		Decimal temp = *this;
		--(*this);
		return temp;
	}
	//Convert functions
	std::string toString() {
		std::string str{ "" };

		if (this->value.size() == 1 && this->decPoint == 1 && this->value[0] == 0)
			return "0";
		if (this->sign == 0)
			str = "-";

		for (maxDigitsNumberType i = 0; i < this->value.size(); i++) {
			str += std::to_string(static_cast<uint16_t>(this->value.at(i)));

			if (i == (this->value.size() - this->decPoint - 1) && decPoint != 0)
				str += ".";
		}

		return str;
	}
	std::wstring toWstring() {
		std::string str = this->toString();
		return std::wstring(str.begin(), str.end());
	}
	std::string toNotationString() {
		std::string str{ "" };

		if (this->value.size() == 1 && this->decPoint == 1 && this->value[0] == 0)
			return "0";
		if (this->sign == 0)
			str = "-";

		Decimal thisAbs = Decimal::abs(*this);

		maxDigitsNumberType offset{ 0 };
		if (thisAbs >= 1) {
			while (thisAbs.decPoint != (thisAbs.value.size() - 1)) {
				thisAbs.decPoint++;
				offset++;
			}

			thisAbs.format();
		}
		else {
			while (thisAbs.value[0] == 0) {
				thisAbs.decPoint--;
				offset--;
				thisAbs.format();
			}
		}

		str += std::to_string(thisAbs.value[0]);

		if(thisAbs.decPoint != 0)
			str += ".";

		for (maxDigitsNumberType i = 1; i < thisAbs.value.size(); i++)
			str += std::to_string(thisAbs.value[i]);

		if(offset != 0)
			str += ("e" + std::to_string(offset));

		return str;
	}
	std::wstring toNotationWstring() {
		std::string str = this->toNotationString();
		return std::wstring(str.begin(), str.end());
	}
	float toFloat() { 
		// DANGER: this method might cause precision lost

		float fl{ 0 };
		for (maxDigitsNumberType i = 0; i < this->size(); i++) {
			fl *= 10;
			fl += this->value.at(i);
		}

		if (this->sign == false)
			fl *= -1;

		float exp{ 1 };
		for (maxDigitsNumberType y = 0; y < this->decPoint; y++)
			exp *= 10;

		return (fl / exp);
	}
	double toDouble() { 
		// DANGER: this method might cause precision lost
		double d{ 0 };
		for (maxDigitsNumberType i = 0; i < this->value.size(); i++) {
			d *= 10;
			d += this->value.at(i);
		}

		if (this->sign == false)
			d *= -1;

		double exp{ 1 };
		for (maxDigitsNumberType y = 0; y < this->decPoint; y++)
			exp *= 10;

		return (d / exp);
	}
	long double toLongDouble() {
		// DANGER: this method might cause precision lost
		long double d{ 0 };
		for (maxDigitsNumberType i = 0; i < this->value.size(); i++) {
			d *= 10;
			d += this->value.at(i);
		}

		if (this->sign == false)
			d *= -1;

		long double exp{ 1 };
		for (maxDigitsNumberType y = 0; y < this->decPoint; y++)
			exp *= 10;

		return (d / exp);
	}
	//Size check
	maxDigitsNumberType size() {
		// DANGER: this method might return wrong value if number is not formated
		return this->value.size();
	}
	//Overflow prevention
private:
	void unoverflow() {
		this->applyPrecision();

		for (maxDigitsNumberType i = 0; i < this->value.size(); i++) {
			if (this->value[i] != 0) {
				if (this->value[i] < 0) {
					this->sign = false;
					goto unoverflow;
				}
				else goto unoverflow;
			}
		}

		unoverflow:

		for (maxDigitsNumberType i = (this->value.size() - 1); i >= 0; i--) {
			const maxDigitsNumberType minIndex{ 0 };
			const maxDigitsNumberType maxIndex = (this->value.size() - 1);

			if (std::abs(this->value[i]) > 9) {
				if (i == minIndex) {
					this->value.push_front(0);
					i++;
					this->value[minIndex] = static_cast<digitType>(this->value[i] / 10);
					this->value[i] = this->value[i] % 10;
				}
				else {
					this->value[(i - 1)] += static_cast<digitType>(this->value[i] / 10);
					this->value[i] = this->value[i] % 10;
				}
			}

			if ((this->value[i] < 0 && this->sign == true)) {
				for (maxDigitsNumberType y = (i - 1); y >= 0; y--) {
					if (this->value[y] > 0) {
						this->value[y]--;
						for (maxDigitsNumberType z = (y + 1); z < i; z++) {
							this->value[z] += 9;
						}

						this->value[i] += 10;
						goto nextLoop;
					}
				}
			}

			if ((this->value[i] > 0 && this->sign == false)) {
				for (maxDigitsNumberType y = (i - 1); y >= 0; y--) {
					if (this->value[y] < 0) {
						this->value[y]++;
						for (maxDigitsNumberType z = (y + 1); z < i; z++) {
							this->value[z] -= 9;
						}

						this->value[i] -= 10;
						goto nextLoop;
					}
				}
			}

		nextLoop:;
		}

		for (maxDigitsNumberType i = 0; i < this->value.size(); i++) {
			this->value[i] = std::abs(this->value[i]);
		}

		this->format(); 
	}
	void format() {

		//Remove unwanted zeroes from start
		for (maxDigitsNumberType a = 0; a < this->value.size(); a++) {
			if (this->value[a] == 0 && a != (this->value.size() - 1 - this->decPoint)) {
				this->value.erase(this->value.begin() + a);
				a--;
			}
			else goto nextStepHandleUnwantedZeroes;
		}

	nextStepHandleUnwantedZeroes:

		//Remove unwanted zeroes from end
		for (maxDigitsNumberType b = (this->value.size() - 1); b >= (this->value.size() - decPoint); b--) {
			if (this->value[b] == 0) {
				this->value.erase(this->value.begin() + b);
				this->decPoint--;
			}
			else goto nextStepHandleUnwantedZeroesFromBack;
		}

	nextStepHandleUnwantedZeroesFromBack:;
	}
	void applyPrecision() {
		// DANGER: this function will case small precision lost
		// INFO: this function prevent memory problems caused by too high precision

		//prevent decPoint == size
		while (this->decPoint >= this->value.size())
			this->value.push_front(0);

		//apply precision
		while (this->decPoint > std::max(Decimal::maxDecPrecisionPoint, 10LL)) {
			this->value.pop_back();
			this->decPoint--;
		}
	}	
public:
	static maxDigitsNumberType maxDecPrecisionPoint;
	static const struct constants {
		static const Decimal e;
		static const Decimal pi;
		static const Decimal egamma;
		static const Decimal phi;
		static const Decimal dottie;
	};
public:
	static Decimal abs(Decimal dec) {
		dec.sign = true;
		return dec;
	}
	static Decimal inverse(const Decimal& dec) {
		Decimal one = Decimal(1);
		return  (one / dec);
	}
	static Decimal ln(Decimal x){
		// DANGER: this function returns average value
		// INFO: you cannot eliminate such a lost, but it is possible to minimalize it using Decimal.maxDecPrecisionPoint

		if (x == Decimal(1))
			return Decimal(0);
		else if (x == Decimal(0))
			throw std::runtime_error("Unspecified operation result");

		Decimal result = 0;

		const unsigned int it = 30 + std::min(std::abs(1.5 * x.toFloat()), 9999.0);
		unsigned int powe = 1;

		Decimal y;

		Decimal xM = (x - 1);
		Decimal xP = (x + 1);
		Decimal z = xP / xM;
		
		Decimal xM2 = (xM * xM);
		Decimal xP2 = (xP * xP);
		Decimal step = xM2 / xP2;

		for (unsigned int i = 1; i < it; i++) {
			z = z * step;
			y = z * (1.0 / powe);

			result = result + y;
			powe = powe + 2;
		}

		result = result * 2;

		return result;
	}
	static Decimal log2(Decimal x) {
		if (x == Decimal(0))
			throw std::runtime_error("Unspecified operation result");

		Decimal result;

		while (x.value[x.value.size() - x.decPoint - 1] % 2 == 0) {
			result++;
			x /= 2;
		}

		Decimal lnX = Decimal::ln(x);
		Decimal ln2 = Decimal::ln(2);

		Decimal decPart = ( lnX / ln2);
		return result + decPart;
	}
	static Decimal log10(Decimal x) {
		if (x == Decimal(0))
			throw std::runtime_error("Unspecified operation result");

		Decimal result;

		while (x.value[x.value.size() - x.decPoint - 1] == 0) {
			result++;
			x /= 10;
		}

		Decimal lnX = Decimal::ln(x);
		Decimal ln10 = Decimal::ln(10);

		Decimal decPart =  (lnX / ln10);
		return result + decPart;
	}
	static Decimal pow(Decimal a, Decimal b){
		// DANGER: this function might return average value
		// INFO: you cannot eliminate such a lost, but it is possible to minimalize it using Decimal.maxDecPrecisionPoint

		if (b % 1 == Decimal(0)) {
			if (b == Decimal(0)) {
				return Decimal(1);
			}

			Decimal result = 1;

			for (Decimal i = 0; i < Decimal::abs(b); i = i + 1) {
				result = result * a;
			}

			if (b.sign == false)
				return Decimal::inverse(result);
			else return result;
		}
		else if (b < 1 && Decimal::inverse(b).decPoint == 0) {
			Decimal result = Decimal::root(a, Decimal::inverse(b));
			return result;
		}
		else {
			Decimal result = 1;
			Decimal tmp = 1;

			Decimal lnA = Decimal::ln(a);

			Decimal m = b * lnA;

			for (maxDigitsNumberType i = 1; i < Decimal::maxDecPrecisionPoint; i++) {
				Decimal d = (m / i);
				tmp = tmp * d;
				result = result + tmp;
			}
			return result;
		}
	}
	static Decimal root(const Decimal& number, Decimal degree) {
		if (number < 0 && degree % 2 == 0)
			throw std::runtime_error("Result out of number scope");
		else if (degree.decPoint != 0)
			throw std::runtime_error("Argument cannot be a fraction");
		else if (degree == 1)
			return number;

		Decimal numberAbs = Decimal::abs(number);

		Decimal start = 0;
		Decimal end = numberAbs;
		Decimal mid;

		Decimal result;

		while (start <= end) {
			mid = (start + end) / 2;
			mid = Decimal::floor(mid);

			Decimal powerRes = Decimal::pow(mid, degree);
			if (powerRes == numberAbs) {
				result = mid;
				break;
			}

			if (powerRes < numberAbs) {
				start = mid + 1;
				result = mid;
			}
			else {
				end = mid - 1;
			}
		}

		Decimal increment = 0.1;
		for (maxDigitsNumberType i = 0; i < Decimal::maxDecPrecisionPoint; i++) {
			while (Decimal::pow(result, degree) <= numberAbs) {
				result += increment;
			}

			result = result - increment;
			increment /= 10;
		}

		result.sign = number.sign;

		return result;
	}
	static const Decimal& min(const Decimal& left, const Decimal& right) {
		if (left < right)
			return left;
		else return right;
	}
	static const Decimal& max(const Decimal& left, const Decimal& right) {
		if (left > right)
			return left;
		else return right;
	}
	static Decimal floor(Decimal dec) {
		while (dec.decPoint != 0) {
			dec.value.pop_back();
			dec.decPoint--;
		}

		return dec;
	}
	static Decimal ceil(Decimal dec) {
		dec.format();

		if (dec.decPoint != 0) {
			dec.value[dec.value.size() - dec.decPoint - 1]++;
		}

		while (dec.decPoint != 0) {
			dec.value.pop_back();
			dec.decPoint--;
		}

		dec.unoverflow();
		return dec;
	}
	static Decimal roundToDecPlaces(Decimal dec, maxDigitsNumberType decPoints) {
		if (dec.decPoint <= decPoints)
			return dec;

		while (dec.decPoint > (decPoints + 1)) {
			dec.value.pop_back();
			dec.decPoint--;
		}

		maxDigitsNumberType lastDigit = dec.value[(dec.value.size() - 1)];
		dec.value.pop_back();
		dec.decPoint--;

		if (lastDigit >= 5)
			dec.value[(dec.value.size() - 1)]++;

		dec.unoverflow();
		return dec;
	}
	static Decimal roundToSignificantDigits(Decimal dec, maxDigitsNumberType digits) {
		maxDigitsNumberType significantDigits{ 0 };

		for (maxDigitsNumberType i = 0; i < dec.value.size(); i++) {
			if (dec.value[i] != 0) {
				significantDigits++;

				if (significantDigits == digits && i != (dec.value.size() - 1)) {
					if (dec.value[i + 1] >= 5)
						dec.value[i]++;
				}
			}

			if (significantDigits > digits)
				dec.value[i] = 0;
		}

		dec.unoverflow();
		return dec;
	}
};

//decimal precision point
maxDigitsNumberType Decimal::maxDecPrecisionPoint = 31;

//math constants 
const Decimal Decimal::constants::e("2.718281828458563411277850606202642376785584483618617451918618203");
const Decimal Decimal::constants::pi("3.1415926535897932384626433832795028841971693993751058209749445923");
const Decimal Decimal::constants::egamma("0.577215664901532860606512090082402431042159335939923598805");
const Decimal Decimal::constants::phi("1.618033988749895"); 
const Decimal Decimal::constants::dottie(".739085133215160641655312087673873404013411758900757464965680");