#pragma once
#include "scalar.h"

template <typename X>
X squared(const X& x) {
	return x*x;
};

template <typename A, typename B>
double dot(const A& a, const B& b) {
	double sum = 0;
	for (int i = 0; i <= a.MAX; i++)
	{
		sum += a.operator()(i) * b.operator()(i);
	}
	return sum;
}

double mag2(const scalar& s)
{
	return dot(s, s);
}

double dist(const scalar& s1, const scalar& s2 = scalar(0, 0))
{
	return sqrt(mag2(s2 - s1));
}