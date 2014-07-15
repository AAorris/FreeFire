#include "stdafx.h"
#include <fstream>
#include <algorithm>
#include <random>
#include <iomanip>
#include "Facet_Sim.h"
#include "Tool_Pos.h"
/*Bitset is an std implementation that allows for easy access
to bits one at a time(in contrast to bools, which are stored
in char(8 bit) containers), has a hash implementation, and
all sorts of other goodies.*/
#include <bitset>
#include <exception>
#define CLASS Facet_Sim
#include "PIMPL.h"

using AA::Pos;

struct layer {
private:
	int _size=0;
public:
	template <typename T>
	using cell_data = std::unordered_map<AA::Pos, T>;
	
	using timer = double;
	using id = char;

	cell_data<timer> cellTimers;
	cell_data<id> cellIDs;
	
	id& operator()(int x, int y) { return cellIDs[Pos(x, y)]; }
	int size(){
		#ifdef _DEBUG
		if (cellTimers.size() != cellIDs.size())
			throw std::length_error("cell containers don't match in length!");
		#endif
		return _size;
	}
	void add(const Pos& _p, const id& _id, const timer& _t=0)
	{
		cellTimers.insert(std::make_pair(_p, _t));
		cellIDs.insert(std::make_pair(_p, _id));
		_size++;
	}
	timer& getTimer(const Pos& p) { return cellTimers[p]; }
	id& getID(const Pos& p) { return cellIDs[p]; }
};

class _sim::Impl {
public:
	layer Fire;
};

CTOR() : p{ new Impl() } {}
DTOR() {}
void CLASS::update(int ms)
{
	for (auto& pair: p->Fire.cellIDs)
		pair.second = 'F';
	for (auto& pair : p->Fire.cellTimers)
		++pair.second;
}
CLASS::mapUpdate& CLASS::getMap(){
	return p->Fire.cellIDs;
}
void CLASS::loadState(const std::string& path){
}
void CLASS::saveState(const std::string& path){
}
std::string CLASS::getChunk(const Pos& _p){
	std::stringstream ss;
	for (int x = _p.x() - 1; x <= _p.x() + 1; ++x) {
		for (int y = _p.y() - 1; y <= _p.x() + 1; ++y){
			ss << p->Fire(x, y);
		}
	}
	return ss.str();
}
void CLASS::putChunk(const std::string& s, const Pos& p){}
void CLASS::put(const Pos& _p, const char& _c, const int& _rings)
{
	p->Fire.add(_p,_c);
}