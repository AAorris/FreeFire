#pragma once
#include <iostream>

struct pos
{
	signed short int x;
	signed short int y;
	pos();
	pos(short _x, short _y);
};

class poshash
{
public:
	std::size_t operator()(pos const& p) const
	{
		unsigned int pos = 0;
		short *it = (short*)(&pos);
		it[0] = p.x;
		it[1] = p.y;
		return pos;
	}
};

class poseq
{
public:
	bool operator() (pos const& p, pos const& p2) const
	{
		return (p.x == p2.x && p.y == p2.y);
	}
};