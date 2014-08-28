#include "stdafx.h"
#include "scalar.h"
#include "Geometry.h"
#include <tuple>
#include <math.h>

bool operator<(const scalar& l, const scalar& r)
{
	return
		(l.y < r.y) ? true
		: (l.y == r.y && l.x < r.x) ? true
		: false;
}

bool operator==(const scalar& l, const scalar& r)
{
	return l.x == r.x && l.y == r.y;
}

//std::ostream& operator<<(std::ostream& s, const scalar& val)
//{
//	s << val.x << " " << val.y << " ";
//	return s;
//}

std::istream& operator>>(std::istream& is, scalar& val)
{
	std::cin >> val.x >> val.y;
	/* if (no valid object of T found in stream)
	is.setstate(std::ios::failbit);*/
	return is;
}

scalar::scalar(const std::string& s)
{
	std::stringstream ss{ s };
	ss >> x;
	ss >> y;
}

scalar scalar::operator%(const int& mod)
{
	int _x = static_cast<int>((x));
	int _y = static_cast<int>((y));
	return scalar(_x%mod, _y%mod);
}

scalar scalar::round(const scalar& s)
{
	int _x = static_cast<int>(std::round(s.x));
	int _y = static_cast<int>(std::round(s.y));
	return scalar(_x, _y);
}

bool scalar::operator==(const scalar& other)
{
	return other.x == x && other.y == y;
}
bool scalar::operator!=(const scalar& other)
{
	return !operator==(other);
}
void scalar::operator+=(const scalar& other)
{
	x += other.x;
	y += other.y;
}
scalar scalar::operator+(const scalar& other)
{
	return scalar{ x + other.x, y + other.y };
}
scalar scalar::operator+(const scalar& other) const
{
	return scalar{ x + other.x, y + other.y };
}
scalar scalar::operator+(const double& other) const
{
	return scalar{ x + other, y + other };
}
void scalar::operator-=(const scalar& other)
{
	x -= other.x;
	y -= other.y;
}
scalar scalar::operator-(const scalar& other)
{
	return scalar{ x - other.x, y - other.y };
}

scalar scalar::operator-(const scalar& other) const
{
	return scalar{ x - other.x, y - other.y };
}

void scalar::operator=(const double& value)
{
	x = y = value;
}
void scalar::operator*=(const double& value)
{
	x *= value;
	y *= value;
}
void scalar::operator*=(const scalar& other)
{
	x *= other.x;
	y *= other.y;
}
scalar scalar::operator*(const scalar& other)
{
	return scalar{ x*other.x, y*other.y };
}
scalar scalar::operator*(const scalar& other) const
{
	return scalar{ x*other.x, y*other.y };
}
scalar scalar::operator*(const double& value)
{
	return scalar{ x*value, y*value };
}
scalar scalar::operator*(const double& value) const
{
	return scalar{ x*value, y*value };
}
void scalar::operator/=(const double& value)
{
	x /= value;
	y /= value;
}
scalar scalar::operator/(const double& value)
{
	return scalar{ x / value, y / value };
}
scalar scalar::operator/(const double& value) const
{
	return scalar{ x / value, y / value };
}
scalar scalar::operator/(const scalar& value) const
{
	return scalar{ x / value.x, y / value.y };
}
