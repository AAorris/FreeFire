#pragma once
#include "scalar.h"
#include "Tool_Configurable.h"
#include "Tool_Asset.h"
#include <set>
const unsigned long BURNABLE = 0x1;

//different from a config tool - it holds variable data compared to static data.
class Tool_Data
{
public:
	scalar pos;
	const char id;
	const unsigned long flags;
	std::set<const std::string> assets;
	bool burning;

	Tool_Data();
	Tool_Data(const Tool_Configurable& config, const scalar& where);
	Tool_Data(char tileID, unsigned long startFlags, std::vector<std::string> assets);
	virtual ~Tool_Data();
	void apply(const std::string& action);
};


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