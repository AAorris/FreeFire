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
for (int y = static_cast<int>(begin.y); y <= static_cast<int>(end.y); y++){\
for (int x = static_cast<int>(begin.x); x <= static_cast<int>(end.x); x++)
/*Just to close the outer for loop*/
#define forEnd }

CTOR() : data{} {}

int windN = 0;
int windS = 0;
int windE = 0;
int windW = 0;

tile::Data* CLASS::operator()(const tile::group_type& group, const scalar& pos)
{
	return data[group].find(facet::sim_item{ pos, NULL })->data;
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
	try {
		information.add_child("Config", session.data);
		for (auto templateConfigItem : session->get_child("Templates"))
		{
			tile::group_type group = templateConfigItem.first[0]; //nab the first character
			tile::id_type id = templateConfigItem.second.data()[0];
			tile::Template::assets_type assets = tile::Template::assets_type{};
			for (auto asset : templateConfigItem.second.get_child("asset"))
				assets.insert(asset.second.data());

			auto properties = tile::properties_type{};
			auto propertyList = templateConfigItem.second.get_child_optional("properties");

			if (propertyList.is_initialized()) {
				properties.data = propertyList.get();
			}


			auto t = tile::Template{ group, id, assets, properties };
			templates.insert(std::make_pair(t.id, t));
		}
	}
	catch (std::exception e)
	{
		SDL_ShowSimpleMessageBox(0, "Configuration problem", "Couldn't connect simulation to configuration because of a missing file...", NULL);
	}
}

void Facet_Sim::update(int ms)
{
	group_type newUnits;
	std::vector<scalar> oldUnits;
	std::vector<scalar> oldFires;

	windN += rand() % 1000;
	windS += rand() % 1000;
	windE += rand() % 1000;
	windW += rand() % 1000;

	int incidents = 0;

	for (auto& group : data)
	{
		for (auto& item : group.second)
		{
			if (item.second == nullptr)
				continue;
			item.second->update(ms);

			if (group.first == tile::UNITGROUP)
			{
				auto unit = static_cast<tile::Unit*>(item.second);
				if (unit->position != item.first)
				{
					newUnits.insert(std::pair<group_type::key_type, group_type::mapped_type>(unit->position, item.second));
					item.second = NULL;
					oldUnits.push_back(item.first);
				}
			}

			if (group.first == tile::FIREGROUP)
			{
				auto fire = static_cast<tile::Fire*>(item.second);
				auto location = item.first;


				if (windW > windE && fire->fireTime > 10000 / 5)
				{
					for (auto subItem : around(tile::OBJECTGROUP, location))
					{
						if (subItem.first.x > item.first.x)
							continue;
						if (subItem.second->hasProperty("Burnable"))
						{
							auto subLocation = subItem.first;
							//auto it = data[tile::FIREGROUP].find(subLocation);
							if (insert(subLocation, fire->root->id))
								++incidents;
						}
					}
				}

				if (windW < windE && fire->fireTime > 10000 / 5)
				{
					for (auto subItem : around(tile::OBJECTGROUP, location))
					{
						if (subItem.first.x < item.first.x)
							continue;
						if (subItem.second->hasProperty("Burnable"))
						{
							auto subLocation = subItem.first;
							//auto it = data[tile::FIREGROUP].find(subLocation);
							if (insert(subLocation, fire->root->id))
								++incidents;
							subItem.second->setProperty("burning", 1);
						}
					}
				}

				if (fire->fireTime > 10000)
				{
					for (auto subItem : around(tile::OBJECTGROUP, location))
					{
						if (subItem.second->hasProperty("Burnable"))
						{
							auto subLocation = subItem.first;
							//auto it = data[tile::FIREGROUP].find(subLocation);
							if (insert(subLocation, fire->root->id))
								++incidents;
							subItem.second->setProperty("burning", 1);
						}
					}
					fire->fireTime = -1;
					//subItem.second->apply(tile::make_flag( "burning" ));
				}
			}
		}
	}
	information.put<int>("Incidents", information.get_optional<int>("Incidents").get_value_or(0) + incidents);
	auto& units = data[tile::UNITGROUP];
	for (auto& pos : oldUnits){
		auto begin = units.lower_bound(pos);
		auto end = units.upper_bound(pos);
		begin++;
		units.erase(begin,end);
	}
	data[tile::UNITGROUP].insert(begin(newUnits), end(newUnits));
	for (auto& pos : oldFires) {
		auto it = data[tile::FIREGROUP].find(pos);
		if (it == end(data[tile::FIREGROUP]))
			continue;
		data[tile::FIREGROUP].erase(it);
	}
}

template <typename T>
bool setHelper(const scalar& pos, Facet_Sim::group_type& group, const Facet_Sim::template_type& root)
{
	auto& it = group.find(pos);
	if (it != end(group)){
		it->second->operator=(&root);
		return true;
	}
	else {
		facet::insert(pos, new T(&root, pos), &group);
		return true;
	}
}

template <typename T>
bool insertHelper(const scalar& pos, Facet_Sim::group_type& group, const Facet_Sim::template_type& root)
{
	auto it = facet::item(pos,&group);
	if (it != end(group))
		return false;//it->second->operator=(&root);
	else{
		facet::insert(pos, new T(&root, pos), &group);
		return true;
	}
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

void Facet_Sim::select(const scalar& cell)
{
	int x = 0;
	int y = 0;
	x = static_cast<int>(round(cell.x));
	y = static_cast<int>(round(cell.y));

	auto it = data[tile::UNITGROUP].find(scalar(x, y));
	if (it != end(data[tile::UNITGROUP]))
		selectedUnit = static_cast<tile::Unit*>(it->second);
	else
		selectedUnit = nullptr;
}

int& Facet_Sim::wind(std::string direction)
{
	if (direction == "N")
		return windN;
	if (direction == "E")
		return windE;
	if (direction == "S")
		return windS;
	if (direction == "W")
		return windW;
	throw std::runtime_error("correct wind not specified");
}

#undef CLASS