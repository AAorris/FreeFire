#pragma once
#include <iosfwd>
#include <tuple>

namespace AA {
	class Pos
	{
	private:
		short _x;
		short _y;
	public:
		static Pos _Origin;
		Pos();
		Pos(short x, short y);
		Pos(int& xy);
		Pos(const Pos& p);
		short x() const;
		short y() const;
		int xy() const;
		short radius() const;
		short angle() const;
		int hash() const;
		void x(short px);
		void y(short py);
		Pos& operator=(const Pos& p);
		bool operator==(const Pos& p);
		Pos operator*(const Pos& p);
		~Pos() = default;
		std::string serialize() const;
		friend std::ostream& operator<<(std::ostream& os, Pos const& p)
		{
			return os << p.x() << " " << p.y();
		}
		friend std::istream& operator>>(std::istream& is, Pos& p)
		{
			int px, py;
			is >> px;
			is >> py;
			p.x(px);
			p.y(py);
			return is;
		}
	};

	typedef Pos v2;
	typedef Pos point;
}

namespace std {
	template<> struct hash<AA::Pos> {
		size_t operator()(const AA::Pos& p) const {
			return p.hash();
		}
	};
	template<> struct less<AA::Pos> {
		bool operator()(const AA::Pos& L, const AA::Pos& R) const {
			return
				std::forward_as_tuple(L.radius(), L.angle()) <
				std::forward_as_tuple(R.radius(), R.angle())
			;
		}
	};
	template<> struct equal_to<AA::Pos> {
		bool operator()(const AA::Pos& p1, const AA::Pos& p2) const {
			return p1.xy() == p2.xy();
		}
	};
}