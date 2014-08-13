#include "stdafx.h"
#include "Tool_Configurable.h"

#include <boost\property_tree\info_parser.hpp>

#define CLASS Tool_Configurable
#include "PIMPL.h"

#include <sstream>

CTOR(const std::string& path)
{
	try {
		boost::property_tree::read_info(path, data);
	}
	catch (std::exception e)
	{
		std::stringstream error;
		error << "Configuration file error : " << e.what();
		SDL_ShowSimpleMessageBox(0, "Configuration Missing",error.str().c_str(),NULL);
	}
}

CTOR()
{
	data = config_type{};
}

CTOR(config_type& input)
{
	data = input;
}

CTOR(CLASS&& other)
{
	data = std::move(other.data);
}

std::string CLASS::getData(const std::string& path) const
{
	std::stringstream ss{""};
	boost::property_tree::write_info(ss, data.get_child(path));
	return ss.str();
}

CLASS::config_type CLASS::operator*(){return data;}
CLASS::config_type* CLASS::operator->(){return &data;}

std::ostream& operator << (std::ostream& s, _cfg& c)
{
	s << c.getData();
	return s;
}

std::istream& operator >> (std::istream& s, _cfg& c)
{
	std::string path;
	std::string value;
	s >> path >> value;
	c->put(path, value);
	return s;
}


#undef CLASS