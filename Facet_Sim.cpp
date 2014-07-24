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
for(int y = begin.y; y <= end.y; y++){\
	for(int x = begin.x; x <= end.x; x++)
/*Just to close the outer for loop*/
#define forEnd }

CTOR() : data{} {}

tile::Data* CLASS::operator()(const tile::group_type& group, const scalar& pos)
{
	return data[group][pos];
}

std::vector<CLASS::group_type::value_type> CLASS::around(const tile::group_type& type, const scalar& pos)
{
	std::vector<group_type::value_type> result{};
	auto begin = pos - scalar(1, 1);
	auto end = pos + scalar(1, 1);

	forArea(begin, end) {
		auto sample = scalar( x, y );
		auto& group = data[type];
		auto it = group.find(sample);
		if(it!=std::end(group))
			result.push_back(*it);
	} forEnd

	return result;
}

void Facet_Sim::connect(_cfg& session)
{
	for (auto item : session->get_child("Templates"))
	{
		tile::group_type group = item.first[0]; //nab the first character
		tile::id_type id = item.second.data()[0];
		tile::Template::assets_type assets = tile::Template::assets_type{};
		for (auto asset : item.second.get_child("asset"))
			assets.insert(asset.second.data());
		auto properties = tile::properties_type{};
		auto propertyList = item.second.get_child_optional("properties");
		if (propertyList.is_initialized())
		{
			for (auto property : *propertyList)
				properties.insert(tile::properties_type::value_type{ property.first, property.second.get_value<bool>() });
		}
		auto t = tile::Template{ group, id, assets, properties };
		templates.insert(std::make_pair(t.id, t));
	}
}

void Facet_Sim::update(int ms)
{
	for (auto& item : data[tile::FIREGROUP]){
		auto fire = static_cast<tile::Fire*>(item.second);
		auto location = item.first;
		if (fire->fireTime >= 0)
			fire->fireTime += ms;
		if (fire->fireTime > 1000)
		{
			for (auto subItem : around(tile::OBJECTGROUP, location))
			{
				using tile::operator&&;

				if (subItem.second->config("burnable") && tile::make_flag("burns") )
				{
					auto subLocation = subItem.first;
					//auto it = data[tile::FIREGROUP].find(subLocation);
					insert(subLocation, fire->root->id);
				}
			}
			fire->fireTime = -1;
				//subItem.second->apply(tile::make_flag( "burning" ));
		}
	}
}

template <typename T>
bool setHelper(const scalar& pos, Facet_Sim::group_type& group, const Facet_Sim::template_type& root)
{
	auto& it = group.find(pos);
	if (it != end(group))
		it->second->operator=(&root);
	else
		return group.insert(std::make_pair(pos, new T( &root ))).second;
}

template <typename T>
bool insertHelper(const scalar& pos, Facet_Sim::group_type& group, const Facet_Sim::template_type& root)
{
	auto& it = group.find(pos);
	if (it != end(group))
		return false;//it->second->operator=(&root);
	else
		return group.insert(std::make_pair(pos, new T(&root))).second;
}

bool Facet_Sim::insert(const scalar& pos, const template_key& key)
{
	const template_type& root = templates[key];
	auto& group = data[root.group];
	if (root.group == tile::FIREGROUP)
		return insertHelper<tile::Fire>(pos, group, root);
	else if (root.group == tile::OBJECTGROUP)
		return insertHelper<tile::Data>(pos, group, root);
	else if (root.group == tile::UNITGROUP)
		return insertHelper<tile::Unit>(pos, group, root);
	else if (root.group == tile::WEATHERGROUP) return false;
		//insertHelper<tile::Fire>(pos, group, root);
	return false;
}

void Facet_Sim::set(const scalar& pos, const template_key& key)
{
	auto& root = templates[key];
	auto& group = data[root.group];
	if (root.group == tile::FIREGROUP)
		setHelper<tile::Fire>(pos, group, root);
	else if (root.group == tile::OBJECTGROUP)
		setHelper<tile::Data>(pos, group, root);
	else if (root.group == tile::UNITGROUP)
		setHelper<tile::Unit>(pos, group, root);
	else if (root.group == tile::WEATHERGROUP) return;
	//master_type::iterator& group_it = data.find(root.group);
	/*if (group_it == end(data)) {
		group_it = data.insert(std::make_pair(root.group, group_type{})).first;
	}
	auto it = group_it->second.find(pos);
	if (it != end(group_it->second))
	{
		auto obj = it->second;
		obj->operator=(&root);
	}
	else {
		group_it->second.insert(std::make_pair(pos, new tile::Data{ &root }));
	}*/
}


#undef CLASS