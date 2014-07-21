#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include <random>
#include <iomanip>
#include "Facet_Sim.h"
#include "Tool_Pos.h"
#include "Tool_Configurable.h"
#include "scalar.h"
/*Bitset is an std implementation that allows for easy access
to bits one at a time(in contrast to bools, which are stored
in char(8 bit) containers), has a hash implementation, and
all sorts of other goodies.*/
#include <bitset>
#include <exception>
#define CLASS Facet_Sim
#include "PIMPL.h"

/*Iterate in a square from x1y1 to x2y2 giving you x and y.
 ! Remember 'forEnd'!*/
#define forArea(begin,end)\
for(int y = begin.y; y < end.y; y++){\
	for(int x = begin.x; x < end.x; x++)
/*Just to close the outer for loop*/
#define forEnd }

CTOR() : data{} {}

Tool_Data& CLASS::operator()(const scalar& pos)
{
	return data[pos];
}

std::vector<Tool_Data*> CLASS::around(const scalar& pos)
{
	std::vector<Tool_Data*> result{};

	auto begin = pos - scalar(1, 1);
	auto end = pos + scalar(1, 1);
	forArea(begin, end) {
		auto sample = scalar( x, y );
		result.push_back(&data[sample]);
	} forEnd

	return result;
}

void CLASS::update(int ms)
{
	for (auto item : data)
	{
		continue;
	}
}

void CLASS::set(const scalar& pos, const char& id)
{

}


#undef CLASS