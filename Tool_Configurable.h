#pragma once

#include "Tool.h"
#include <boost\property_tree\ptree_fwd.hpp>

class Tool_Configurable :
	public Tool
{
private:
	class Impl
	{
	public:
		Impl();
		//document me
		std::unique_ptr<boost::property_tree::ptree> data;
		void load(const std::string& config_path);
	};
	std::unique_ptr<Impl> p;

	Tool_Configurable(const Tool_Configurable&) = delete;

public:
	Tool_Configurable(const std::string& config_path);
	~Tool_Configurable();

	std::string serialize();
	std::ostream& operator << (std::ostream& os);
	std::vector<std::string> getAssets();

	//template <typename T> T get(const std::string& s)	{ return p->get<T>(s);	}
	std::string get(const std::string& s);
	boost::property_tree::ptree getT(const std::string& s);

	template <typename T> void put(const T& t)			{ p->put<T>(t);			}

};