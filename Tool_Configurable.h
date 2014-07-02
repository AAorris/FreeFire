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
		std::unique_ptr<boost::property_tree::ptree> data;
		template <typename T>
		void put(const T& t);
		template <typename T>
		T get(const std::string& path);
		void load(const std::string& config_path);
	};
	std::unique_ptr<Impl> p;

public:
	Tool_Configurable(const std::string& config_path);
	~Tool_Configurable();
	Tool_Configurable(const Tool_Configurable&) = delete;

	std::string serialize();

	template <typename T> T get(const std::string& s)	{ return p->get<T>(t);	}
	template <typename T> void put(const T& t)			{ p->put<T>(t);			}

};