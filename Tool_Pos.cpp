#include "stdafx.h"
#include "Tool_Pos.h"
#include <bitset>
namespace AA {

	Pos Pos::_Origin = Pos(0, 0);

	Pos::Pos(short px, short py)
	{
		/*_pos = 0;
		auto ref = (short*)&_pos;
		x = &ref[0];
		y = &ref[1];
		*x = px;
		*y = py;*/
		_x = px;
		_y = py;
	}
	Pos::Pos(const Pos& p)
	{
		x ( p.x() );
		y ( p.y() );
	}
	Pos::Pos(int& xy)
	{
		short* ref = reinterpret_cast<short*>(&xy);
		_x = ref[0];
		_y = ref[1];
	}

	Pos::Pos()
	{
		x(0);
		y(0);
	}

	short Pos::x() const { return _x; }
	short Pos::y() const { return _y; }
	int Pos::xy() const {
		int res;
		short* it = (short*)&res;
		it[0] = _x;
		it[1] = _y;
		return res;
	}

	void Pos::x(short x) { _x = x; }
	void Pos::y(short y) { _y = y; }

	short Pos::radius() const {
		return SDL_max(x(), y());
	}
	short Pos::angle() const {
		return (int)atan2(y(), x());
	}
	/*staggers bits for (relatively) spatially coherent hashing.
	instead of spiralling, it kind of criss crosses around the
	rings.*/
	int Pos::hash() const {
		std::bitset<7> mx = { static_cast<_ULonglong>(abs(x())) };
		std::bitset<7> my = { static_cast<_ULonglong>(abs(y())) };
		std::bitset<16> p = {};

		p[0] = x() >= 0;
		p[1] = y() >= 0;
		for (int i = 0; i < 7; i++)
		{
			p[(2*i)+2] = mx.test(i);
			p[(2*i)+2 + 1] = my.test(i);
		}
		return p.to_ulong();
	}

	bool Pos::operator==(const Pos& p)
	{
		return p.x() == p.y();
	}

	std::string Pos::serialize() const
	{
		std::stringstream ss;
		ss << "" << x() << " " << y() << "";
		//ss << xy();
		return ss.str();
	}

	Pos& Pos::operator=(const Pos& p)
	{
		/*_pos = p._pos;
		auto ref = (short*)&_pos;
		x = &ref[0];
		y = &ref[1];*/
		x(p.x());
		y(p.y());
		return *this;

	}
}