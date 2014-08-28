#pragma once
#include <memory>
#include "Algorithms.h"
enum INDEX { X, Y, Z, W };

class scalar {
public:
	static const int MAX = Y;
	double x;
	double y;
	scalar(double _x, double _y) : x{ _x }, y{ _y }{}
	scalar(const std::string& s);
	scalar() : x{ 0 }, y{ 0 }{}
	/*round brackets for doubles*/
	const double& scalar::operator()(const int& i) const {
		if (i == 0) return x;
		if (i == 1) return y;
	}
	/*square brackets for integers*/
	int scalar::operator[](const int& i) const {
		if (i == 0) return static_cast<int>(x);
		if (i == 1) return static_cast<int>(y);
	}
	//memory
	bool scalar::operator!=(const scalar& other);
	bool scalar::operator==(const scalar& other);
	void scalar::operator=(const double& value);
	//memory interfacing
	void scalar::operator+=(const scalar& other);
	void scalar::operator-=(const scalar& other);
	void scalar::operator*=(const double& value);
	void scalar::operator*=(const scalar& other);
	void scalar::operator/=(const double& value);
	//(const)arithmetic
	scalar scalar::operator+(const scalar& other) const;
	scalar scalar::operator+(const double& other) const;
	scalar scalar::operator-(const scalar& other) const;
	scalar scalar::operator*(const double& value) const;
	scalar scalar::operator*(const scalar& other) const;
	scalar scalar::operator/(const double& value) const;
	scalar scalar::operator/(const scalar& other) const;
	//arithmetic (scalar)
	scalar scalar::operator-(const scalar& other);
	scalar scalar::operator+(const scalar& other);
	scalar scalar::operator*(const scalar& other);
	scalar scalar::operator%(const int& mod);
	//arithmetic (1d)
	scalar scalar::operator*(const double& value);
	scalar scalar::operator/(const double& value);
	static scalar scalar::round(const scalar& s);
};


bool operator<(const scalar& l, const scalar& r);
bool operator==(const scalar& l, const scalar& r);
inline std::ostream& operator<<(std::ostream& stream, const scalar& s) {
	stream << s.x << " " << s.y;
	return stream;
}

template<> struct std::hash<scalar>
{
	size_t operator()(const scalar& s) const {
		unsigned long long val = static_cast<unsigned long long>(s.y);
		val = val << sizeof(s.y);
		val |= static_cast<int>(s.x);
		return size_t(val);
	}
};