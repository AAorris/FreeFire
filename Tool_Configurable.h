#pragma once

#include "Tool.h"
#include <boost/property_tree/ptree.hpp>

class Tool_Configurable
{
public:
	using config_type = boost::property_tree::ptree;
	using value_type = std::string;
	config_type data;

	Tool_Configurable();
	Tool_Configurable(config_type& input);
	Tool_Configurable(const std::string& path);
	Tool_Configurable(Tool_Configurable&& c);
	~Tool_Configurable() = default;
	std::string getData(const std::string& path="") const;
	config_type* operator->();
	config_type operator*();
	config_type::iterator begin();
	config_type::iterator end();
};

typedef Tool_Configurable _cfg;

std::ostream& operator << (std::ostream& s, _cfg& c);
std::istream& operator >> (std::istream& s, _cfg& c);

//Tool_Data&& createTool_Data(const _cfg& template, )
//std::string getPath(const _cfg::config_type& tree, const _cfg::config_type& node);