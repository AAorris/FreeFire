#include "stdafx.h"
#include "Tool_Configurable.h"
//#include <iostream>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\info_parser.hpp>

#define DEF Tool_Configurable //these get UNDEF'ed
#define IMPL Tool_Configurable::Impl //DONT FORGET THE UNDEF :-)

IMPL::Impl() :
	data { new boost::property_tree::ptree() }
{
}

void IMPL::load(const std::string& path)
{
	using boost::property_tree::ptree;
	using boost::property_tree::read_info;
	//Load the data from the file here. In this case, populates a ptree.
	read_info(path,*data);
}

template <typename T>
T IMPL::get(const std::string& path)
{
	return data.get<std::string>(path);
}

template <typename T>
void IMPL::put(const T& t)
{

}

DEF::Tool_Configurable(const std::string& path)
: p{ new Impl() }
{
	p->load(path);
}


DEF::~Tool_Configurable()
{
}

std::string DEF::serialize()
{
	std::ostringstream serial;
	boost::property_tree::write_info(serial,*(p->data));
	return serial.str();
}

std::ostream& Tool_Configurable::operator<<(std::ostream& os)
{
	os << serialize();
	return os;
}

#undef DEF
#undef IMPL