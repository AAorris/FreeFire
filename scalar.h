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
	const double& scalar::operator()(const int& i) const {
		switch (i){ case X:return x; case Y:return y; }
	}
	const double& scalar::operator[](const int& i) const {
		switch (i){ case X:return x; case Y:return y; }
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
};

bool operator<(const scalar& l, const scalar& r);
bool operator==(const scalar& l, const scalar& r);

template<> struct std::hash<scalar>
{
	size_t operator()(const scalar& s) const {
		unsigned long long val = s.y;
		val = val << sizeof(s.y);
		val |= static_cast<int>(s.x);
		return val;
	}
};