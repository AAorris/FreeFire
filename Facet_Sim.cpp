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

CTOR() : data{6} {

}

int windN = 0;
int windS = 0;
int windE = 0;
int windW = 0;

facet::group_item_value& CLASS::operator()(const tile::group_type& group, const scalar& pos)
{
	using facet::Group;
	using facet::Item;
	return Item(pos, &Group(group,&data)->second)->second;
}

std::vector<std::pair<scalar, tile::Data*>> CLASS::around(const tile::group_type& type, const scalar& pos)
{
	std::vector<std::pair<scalar, tile::Data*>> result{};
	auto begin = pos - scalar(1, 1);
	auto end = pos + scalar(1, 1);

	forArea(begin, end) {
		auto sample = scalar( x, y );
		auto& group = data[type];
		auto it = group.find(sample);
		if (it != std::end(group))
		{
			for (auto set_it : it->second)
				result.push_back(std::pair<scalar, tile::Data*>(sample,set_it));
		}
	} forEnd

	return result;
}

void Facet_Sim::connect(_cfg& session)
{
	try {
		information.add_child("Config", session.data);
		for (auto templateConfigItem : session->get_child("Config.Entities"))
		{
			tile::group_type group = 'u'; //nab the first character
			tile::id_type id = templateConfigItem.second.get_optional<tile::id_type>("ID").get_value_or(rand());
			tile::Template::assets_type assets = tile::Template::assets_type{};
			for (auto asset : templateConfigItem.second.get_child("Assets"))
			{
				std::string name = asset.first;
				if (name == "Normal" || name == "Small") {
					std::string path = asset.second.get<std::string>("Path");
					assets.insert(path);
				}
			}

			auto properties = tile::properties_type{};
			auto propertyList = templateConfigItem.second.get_child_optional("Abilities");

			if (propertyList.is_initialized()) {
				for (auto& property : propertyList.get())
				{
					if (property.second.data() != "")
						properties.put<double>(property.first, std::stod(property.second.data()));
				}
			}


			auto t = tile::Template{ group, id, assets, properties };
			templates.insert(std::make_pair(t.id, t));
		}
	}
	catch (std::exception e)
	{
		SDL_ShowSimpleMessageBox(0, "Sim Error", e.what(), NULL);
	}
}

void Facet_Sim::update(int ms)
{
	std::vector<tile::Unit*> newUnits;
	std::vector<std::pair<scalar, tile::Data*>> oldUnits;
	std::vector<std::pair<scalar, tile::Data*>> oldFires;

	windN += rand() % 1000;
	windS += rand() % 1000;
	windE += rand() % 1000;
	windW += rand() % 1000;

	int incidents = 0;

	for (auto& group : data)
	{
		for (auto& stack : group.second)
		{
			for (auto& item : stack.second)
			{
				if (item == nullptr)
					continue;
				auto& data = item;
				data->update(ms);

				if (group.first == tile::UNITGROUP)
				{
					auto unit = static_cast<tile::Unit*>(data);
					auto key = stack.first;
					if (unit->position != key)
					{
						tile::Data* landptr = *this->data[tile::GEOGRAPHYGROUP].lower_bound(unit->position)->second.begin();
						auto land = static_cast<tile::Land*>(landptr);
						if (land->isWater() == false) {
							newUnits.push_back(unit);
							oldUnits.push_back(std::pair<scalar, tile::Data*>(key, data));
						}
						else {
							unit->position = key;
							unit->destination.reset();
						}
					}
				}

				if (group.first == tile::FIREGROUP)
				{
					auto fire = static_cast<tile::Fire*>(data);
					auto location = stack.first;

					if (windW > windE && fire->fireTime > 10000 / 5)
					{
						for (auto& subItem : around(tile::OBJECTGROUP, location))
						{
							if (subItem.first.x > location.x)
								continue;
							if (subItem.second->hasProperty("Burnable"))
							{
								auto subLocation = subItem.first;
								//auto it = data[tile::FIREGROUP].find(subLocation);
								if (insert(subLocation, fire))
									++incidents;
							}
						}
					}

					if (windW < windE && fire->fireTime > 10000 / 5)
					{
						for (auto& subItem : around(tile::OBJECTGROUP, location))
						{
							if (subItem.first.x < location.x)
								continue;
							if (subItem.second->hasProperty("Burnable"))
							{
								auto subLocation = subItem.first;
								//auto it = data[tile::FIREGROUP].find(subLocation);
								if (insert(subLocation, fire))
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
								if (insert(subLocation, fire))
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
	}
	information.put<int>("Incidents", information.get_optional<int>("Incidents").get_value_or(0) + incidents);
	auto& units = data[tile::UNITGROUP];
	auto& fires = data[tile::FIREGROUP];
	for (auto it : oldUnits){
		units[it.first].erase(it.second);
	}
	for (auto it: oldFires) {
		fires[it.first].erase(it.second);
	}
	for (auto& it : newUnits) {
		units[it->position].insert(it);
	}
}

bool Facet_Sim::insert(const scalar& pos, const template_key& key)
{
	const template_type& root = templates[key];
	auto& group = data[root.group];
	auto& stack = group[pos];
	if (root.group == tile::FIREGROUP && stack.size() == 0) {
		stack.insert(new tile::Fire(&root, pos));
		return true;
	}
	if (root.group == tile::OBJECTGROUP) {
		stack.insert(new tile::Data(&root, pos));
		return true;
	}
	if (root.group == tile::UNITGROUP) {
		stack.insert(new tile::Unit(&root, pos));
		return true;
	}
	if (root.group == tile::WEATHERGROUP) {
		stack.insert(new tile::Data(&root, pos));
		return true;
	}
		//insertHelper<tile::Fire>(pos, group, root);
	return false;
}

bool Facet_Sim::insert(const scalar& pos, tile::Fire* fire)
{
	if (data[tile::FIREGROUP][pos].size() == 0){
		//TODO : make this monstrocity of a line into something comprehensible
		// cast<Land> ( iterator_value(data[Land][pos][top_layer]) )
		double oldElevation = static_cast<tile::Land*>(*data[tile::GEOGRAPHYGROUP][fire->pos].begin())->elevation;
		double newElevation = static_cast<tile::Land*>(*data[tile::GEOGRAPHYGROUP][pos].begin())->elevation;
		// old/new 1/2
		double slopeSpeed = (newElevation - oldElevation) / (oldElevation);
		fire->fireSpeed += slopeSpeed; // gets bigger when new>old
		data[tile::FIREGROUP][pos].insert(new tile::Fire(fire, pos));
		return true;
	}
	return false;
}

void Facet_Sim::select(const scalar& cell)
{
	int x = 0;
	int y = 0;
	x = static_cast<int>(round(cell.x));
	y = static_cast<int>(round(cell.y));

	auto it = facet::Item(scalar(x,y),&data[tile::UNITGROUP]);
	if (it != end(data[tile::UNITGROUP]))
		selectedUnit = static_cast<tile::Unit*>(*it->second.begin());
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

Facet_Sim::~Facet_Sim() {
	for (auto& x : data)
	{
		for (auto& y : x.second)
		{
			for (auto& z : y.second)
			{
				delete z;
			}
		}
	}
}

#undef CLASS