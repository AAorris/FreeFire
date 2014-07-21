#pragma once

#include "Tool_Data.h"
#include <boost\property_tree\ptree_fwd.hpp>
#include <boost\optional\optional_fwd.hpp>

/**Ptree translator*/
struct DataTranslator {
	// the type we return
	typedef Tool_Data external_type;
	// the type expect as an argument
	typedef boost::property_tree::ptree internal_type;
	boost::optional< external_type > get_value(const internal_type& input);
};

Tool_Data create_data(_cfg::config_type& cfg, DataTranslator& translator, const scalar& s);
