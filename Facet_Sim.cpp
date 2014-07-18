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

using AA::Pos;

enum GROUPS {
	GROUP_NONE='0',
	GROUP_OBJECTS='O',
	GROUP_UNITS='U',
	GROUP_FIRE='F',
};

struct tile {
	typedef char id_type;
	id_type group_id;
	id_type tile_id;
	scalar location;
	tile() : tile_id{ 0 }, group_id{ GROUP_NONE }, location{ 0, 0 } {}
	tile(id_type p_group_id, id_type p_tile_id, scalar p_location)
		: group_id{ p_group_id },
		tile_id{ p_tile_id },
		location{ p_location }
	{
	}
	virtual std::ostream& operator<<(std::ostream& s) const {
		s
			<< group_id << " "
			<< tile_id << " "
			<< location << " ";
		return s;
	}
	virtual std::istream& operator>>(std::istream& i) {
		i
			>> group_id >> tile_id >> location;
		return i;
	}
};

struct fire : public tile {
	char heat;
	fire() : tile(), heat{ 0 } {}
	fire(scalar plocation, char pheat = 0) : tile(GROUP_FIRE,'f',plocation) {
		heat = pheat; 
	}
	virtual std::ostream& operator<<(std::ostream& s) const { 
		tile::operator<<(s) 
			<< heat << " ";
		return s; 
	}
	virtual std::istream& operator>>(std::istream& i) {
		tile::operator>>(i) 
			>> heat; 
		return i; 
	}
};

struct unit : public tile {
	const static char TILE_ID = 'u';
	enum UNIT_STATE {
		IDLE = 'i',
		FIGHT = 'f',
		MOBILIZE = 'b',
		MOVE = 'm',
		DEMOBILIZE = 'e',
	}; 
	id_type unit_id;
	id_type action;
	char actionTime;
	scalar destination;
	
	unit() : tile(), unit_id{ 0 }, action{ 0 }, actionTime{ 0 }, destination{ 0, 0 } {}
	unit(scalar _location, id_type punit_id) 
		: tile(GROUP_UNITS, unit::TILE_ID, _location)
		, unit_id{ punit_id } 
	{
		action = IDLE;
		actionTime = 0;
		destination = _location;
	}

	virtual std::ostream& operator<<(std::ostream& s) const { 
		tile::operator<<(s)
			<< unit_id << " "
			<< action << " "
			<< actionTime << " "
			<< destination << " ";
		return s; 
	}
	virtual std::istream& operator>>(std::istream& i) {
		tile::operator>>(i) 
			>> unit_id 
			>> action 
			>> actionTime 
			>> destination; 
		return i; 
	}
};

#define TO_TILE dynamic_cast<tile*>
std::ostream& operator<<(std::ostream& s, tile* val) { val->operator<<(s); return s; }
std::istream& operator>>(std::istream& is, tile* val) { val->operator>>(is); return is; }

struct grid {
	using data = std::map<scalar, tile*>;
	data contents;
	/* Don't use this unless you gotta! */
	data* operator*() { return &contents; }
	template <typename T>
	void set(const scalar& s, T* val) {
		auto& result = contents.insert( std::make_pair(s, val) );
		if (!result.second)
			result.first->second = val; // <iterator<key,val>,bool> : we want key.
	}
	template <typename T>
	T* get(const scalar& s) {
		auto& it = data.find(s);
		if (it != end(data))
			return dynamic_cast<T*>(iterator.second);
		else
			return nullptr;
	}
};

struct layer {
private:
	int _size=0;
public:
	template <typename T>
	using cell_data = std::map<AA::Pos, T>;
	
	using timer = double;
	using id = char;
	using action = char;
	using unit = std::pair<id, action>;

	cell_data<timer> cellTimers;
	cell_data<unit> units;
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
	void addUnit(const Pos& _p, const id& _id)
	{
		units.insert(std::make_pair(_p, std::make_pair(_id,0)));
	}

	timer& getTimer(const Pos& p) { return cellTimers[p]; }
	id& getID(const Pos& p) {
		return cellIDs[p]; 
	}
	unit& getUnit(const Pos& p)
	{
		return units[p];
	}
};

struct stack {

	using appliable = void(*)(tile* data);
	tile* get(scalar point);
	std::vector<tile*> getAround(scalar point);
	// update burnables around fire such that burnables become fire.
	// for_each ( begin(neighbours), end(neighbours), applyFire )
	void forAround(char onLayer, scalar atPoint, appliable func);

};

class _sim::Impl {
public:
	layer Fire;
};

CTOR() : p{ new Impl() } {

	auto units = grid();
	unit* u = new unit(scalar(0, 0), 'b');
	unit* u2 = new unit(scalar(0, 0), '1');
	//fire* f = new fire(scalar(0, 0), 'b');
	units.set(scalar(0, 1), u);
	units.set(scalar(0, 1), u2);
	//units.set(scalar(0, 1), f);

}
DTOR() {}
void CLASS::update(int ms)
{
	for (auto& pair : p->Fire.cellTimers){

		auto key = pair.first;
		char id = p->Fire.cellIDs[key];
		p->Fire.getTimer(key)+=1/60.0;
		if (p->Fire.cellIDs[key] != 'F')
			continue;

		if (pair.second > 4)
		{
			for (int x = key.x() - 1; x <= key.x() + 1; x++)
			{
				for (int y = key.y() - 1; y <= key.y() + 1; y++)
				{
					if (p->Fire.cellIDs[AA::Pos(x, y)] > '0' && p->Fire.cellIDs[AA::Pos(x, y)] < 'F')
					p->Fire.cellIDs[AA::Pos(x, y)] = 'F';
					p->Fire.cellTimers[AA::Pos(x, y)] = 0;
				}
			}
			pair.second = 0;
		}
	}
}
//CLASS::mapUpdate& CLASS::getMap(){
	//return p->Fire.cellIDs;
//}
CLASS::mapUpdate* CLASS::getMap(){
	return &p->Fire.cellIDs;
}
CLASS::mapUpdate CLASS::getInCamera(camera_data data)
{
	using std::make_pair;
	auto result = CLASS::mapUpdate{};
	int paddingRings = 1;
	auto range = data.getRange(paddingRings);
	int x0 = range.first.x;
	int x1 = range.second.x;
	int y0 = range.first.y;
	int y1 = range.second.y;
	for (int x=x0; x < x1; x++)
	{
		for (int y=y0; y < y1; y++)
		{
			AA::Pos spot = AA::Pos(x, y);
			if (p->Fire.cellIDs.find(spot)!=p->Fire.cellIDs.end())
				result.insert(make_pair(spot, p->Fire.getID(spot)));
		}
	}
	return result;
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