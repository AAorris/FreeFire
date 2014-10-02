#include "stdafx.h"
#include "Tool_Data.h"
#include "scalar.h"
#include <set>

namespace tile {
	std::unordered_map<std::string, int> Fire::fireCounter;

	Template::Template() :
		group{ 0 },
		id{ 0 },
		assets{ assets_type{} },
		properties{ properties_type{} }
	{

	}
	Template::Template(const group_type& p_group, const id_type& p_id, const assets_type& p_assets, const properties_type& p_properties) :
		group{ p_group },
		id{ p_id },
		assets{ p_assets },
		properties{ p_properties }
	{
	}

	//---------------------------------------------------------------------
	
	Data::Data(const tile::Template* config, const scalar& posRef) : root{ config }
	{
		UID = reinterpret_cast<unsigned long>(this);
		if (config == nullptr)
			return;
		id = root->id;
		pos = posRef;
		properties = root->properties;
	}
	void Data::operator=(const Template* p_root)
	{
		root = p_root;
		id = root->id;
		properties = properties_type{};
	}
	bool Data::operator==(const Data& other) {
		return id == other.id;
	}
	bool Data::operator<(const Data& other) {
		return id < other.id;
	}
	void Data::update(int ms)
	{
	}

	Unit::Unit(const tile::Template* config, const scalar& posRef) : tile::Data(config, posRef)
	{
		position = posRef;
	}
	bool Data::hasProperty(const std::string& prop)
	{
#ifdef _DEBUG
		//std::string props = root->properties;
#endif
		return root->properties.find(prop) != root->properties.not_found();
		//return false;
	}
	void Data::setProperty(const std::string& prop, double value)
	{
		properties.put(prop, value);
	}
	void Unit::update(int ms)
	{
		Data::update(ms);
		move_time += ms;
		status_time += ms;
		if (destination.is_initialized())
		{
			status = "Moving";
			if (move_time > 1000)
			{
				auto delta = destination.get() - position;
				if ((abs(delta.x) == 0 || abs(delta.y) == 0) || abs(delta.x)+abs(delta.y) > 0)
				{
					if (abs(delta.x) > abs(delta.y))
						dir = (delta.x > 0) ? DIR_RIGHT : DIR_LEFT;
					else if (abs(delta.x) + abs(delta.y) > 0)
						dir = (delta.y > 0) ? DIR_UP : DIR_DOWN;

				}

				if (dir == DIR_UP)
					position.y += 1;
				if (dir == DIR_DOWN)
					position.y -= 1;
				if (dir == DIR_LEFT)
					position.x -= 1;
				if (dir == DIR_RIGHT)
					position.x += 1;

				scalar dest = destination.get_value_or(position);
				if (position == dest) {
					destination.reset();
					dir = DIR_NONE;
				}

				move_time = 0;
			}
		}
	}

	void Fire::initFire() {
		Fire::fireCounter = std::unordered_map<std::string, int>();
	}

	Fire::Fire(const tile::Template* config, const scalar& posRef, const std::string& p_region) : tile::Data(config, posRef), firstFire{ this }
	{
		region = p_region;
		Fire::fireCounter[region] = Fire::fireCounter[region]+1;
		regionID = Fire::fireCounter[region];
		fireTime = 0;
		isRoot = true;
	}


	Fire::Fire(Fire* const parent, const scalar& posRef) : tile::Data(parent->root, posRef), firstFire{ parent->firstFire }
	{
		region = parent->region;
		regionID = parent->regionID;
		++(firstFire->incidents);
		fireTime = 0;
	}

	void Fire::operator=(const Template* p_root){
		Data::operator=(p_root);
		fireTime = 0;
	}

	void Fire::update(int ms)
	{
		Data::update(ms);
		if (fireTime >= 0)
			fireTime += ms/3 * fireSpeed;
	}
}