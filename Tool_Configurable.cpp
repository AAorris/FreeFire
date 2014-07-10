#include "stdafx.h"
#include "Tool_Configurable.h"
//#include <iostream>
#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\info_parser.hpp>
#include <boost\property_tree\xml_parser.hpp>

#define DEF Tool_Configurable //these get UNDEF'ed
#define IMPL Tool_Configurable::Impl //DONT FORGET THE UNDEF :-)

using boost::property_tree::ptree;
using boost::property_tree::read_info;

IMPL::Impl() :
	data { new boost::property_tree::ptree() }
{
}

void IMPL::load(const std::string& path)
{
	//Load the data from the file here. In this case, populates a ptree.
	read_info(path,*data);
}
/*
template <typename T>
void IMPL::put(const T& t)
{

}
*/

DEF::Tool_Configurable(const std::string& path)
: p{ new Impl() }
{
	p->load(path);
}


DEF::~Tool_Configurable()
{
}

ptree DEF::getT(const std::string& path)
{
	return p->data->get_child(path,ptree());
}

std::string DEF::get(const std::string& path)
{
	return p->data->get<std::string>(path, "");
}

std::vector<std::string> DEF::getAssets()
{
	using boost::property_tree::ptree;
	std::vector<std::string> result;

	//get assets from the templates
	auto templates = getT("Templates");
	for (auto i_template : templates)
	{
		char id = i_template.second.get<char>("id");
		auto asset = i_template.second.get<std::string>("asset");
		std::stringstream ss;
		ss << id << asset;
		result.push_back(ss.str());
	}
	return result;
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