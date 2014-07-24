#include "stdafx.h"
#include "Tool_Data.h"
#include "scalar.h"
#include <set>

namespace tile {
	flag make_flag(const std::string& name){ flag f = flag{ name, 1 }; return f; }
	void operator|=(properties_type& properties, const flag& f)
	{
		auto it = properties.find(f.first);
		if (it == end(properties))
			properties.insert(f);
		else
			properties[f.first] = f.second;
	}
	flag& operator&(const flag& left, const flag& right)
	{
		assert(left.first == right.first);
		return flag{ left.first, left.second&right.second };
	}
	bool operator&&(const flag& left, const flag& right)
	{
		return left.second && right.second;
	}
	bool operator&&(const flag& left, flag& right)
	{
		return left.second && right.second;
	}
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
	const tile::flag_type Template::operator()(const flag::first_type& property) const
	{
		if (properties.find(property)!=end(properties))
			return properties.at(property);
		else
			return tile::flag_type{};
	}

	//---------------------------------------------------------------------
	
	Data::Data(const tile::Template* config) : root{ config }
	{
		id = root->id;
		status = properties_type{};
	}
	void Data::operator=(const Template* p_root)
	{
		root = p_root;
		id = root->id;
		status = properties_type{};
	}
	bool Data::operator==(const Data& other) {
		return id = other.id;
	}
	bool Data::operator<(const Data& other) {
		return id < other.id;
	}
	void Data::apply(const flag& property) {
		status |= property & flag{property.first, (*root)(property.first)};
	}
	flag Data::config(const flag::first_type& property)
	{
		flag f = flag{ property, root->operator()(property) };
		return f;
	}
	void Data::update(int ms)
	{

	}
	void Data::setStatus(const flag::first_type& key, const flag::second_type& val)
	{
		auto it = status.find(key);
		if (it == end(status))
			status.insert(std::make_pair(key, val));
		else
			status[key] = val;
	}

	Unit::Unit(const tile::Template* config) : tile::Data(config)
	{
		unitTime = 0;
	}
	void Unit::update(int ms)
	{
		Data::update(ms);
		unitTime += ms;
		if (destination.is_initialized())
		{
			if (unitTime > 1000)
			{
				setStatus("canMove", true);
			}
		}
	}


	Fire::Fire(const tile::Template* config) : tile::Data(config)
	{
		fireTime = 0;
	}

	void Fire::operator=(const Template* p_root){
		Data::operator=(p_root);
		fireTime = 0;
	}
}