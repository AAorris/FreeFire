#pragma once
#include "scalar.h"
#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include <iosfwd>
#include <set>
#include <boost\property_tree\ptree.hpp>

//looks like you're going to have to name your tile variables carefully. :)
namespace tile
{
	using id_type = char;
	using group_type = char;
	using properties_type = _cfg;//std::unordered_map<std::string, flag_type>;

	enum groups {
		WEATHERGROUP	= 'w',
		UNITGROUP		= 'u',
		FIREGROUP		= 'f',
		OBJECTGROUP		= 'o',
		GEOGRAPHYGROUP	= 'g',
	};
	const std::vector<id_type> group_order = { GEOGRAPHYGROUP, OBJECTGROUP, FIREGROUP, UNITGROUP, WEATHERGROUP };

	using timer_type = int;

	class Template
	{
	public:
		using assets_type = std::set<const std::string>;

		const group_type group;
		const id_type id;
		const assets_type assets;
		const properties_type properties;

		Template();
		Template(const group_type& p_group, const id_type& p_id, const assets_type& p_assets, const properties_type& p_properties);
		//const flag_type operator()(const flag::first_type& property) const;
	};

	class Data
	{
	public:

		id_type id = 0;
		properties_type properties = properties_type{};
		const tile::Template* root;
		//bool burning;

		Data(const tile::Template* config, const scalar& posRef);
		virtual ~Data() = default;
		virtual void operator=(const Template*);
		virtual bool operator==(const Data& other);
		virtual bool operator<(const Data& other);
		//virtual void apply(const properties_type& property);
		virtual void update(int ms);
		bool hasProperty(const std::string& property);
		void setProperty(const std::string& property, double speed);
	};

	class Unit : public tile::Data
	{
	public:
		scalar position;

		const std::string status_idle = "idle";
		const std::string status_moving = "moving";
		const std::string status_mobilizing = "mobilizing";
		const std::string status_demobilizing = "demobilizing";

		std::string status = status_idle;

		enum DIRECTION {
			DIR_NONE,
			DIR_UP = 0x0001,
			DIR_LEFT = 0x0010,
			DIR_RIGHT = 0x0100,
			DIR_DOWN = 0x1000
		};
		int dir = DIR_NONE;
		boost::optional<scalar> destination;
		timer_type move_time = 0;
		//
		Unit(const tile::Template* config, const scalar& posRef);
		virtual void update(int ms);
	};

	class Fire : public tile::Data
	{
	public:
		timer_type fireTime = 0;
		Fire(const tile::Template* config, const scalar& posRef);
		virtual void operator=(const Template*);
		virtual void update(int ms);
	};
}





//position
/*auto positionData = input.get_optional<std::string>("pos").get_value_or("0,0");
std::istringstream ss{ positionData };
scalar pos{ 0, 0 };
std::string xStr;
std::string yStr;
std::getline(ss, xStr, ',');
std::getline(ss, yStr);
pos.x = std::stod(xStr);
pos.y = std::stod(yStr);*/