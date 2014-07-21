#include "stdafx.h"
#include "DataTranslator.h"
#include <boost\property_tree\ptree.hpp>
#include <boost\optional.hpp>


boost::optional< DataTranslator::external_type > DataTranslator::get_value(const internal_type& input) {
	using std::make_pair;

	unsigned long flags = 0;
	char id = '0';
	std::vector<std::string> assets{};

	//id
	id = input.get_value<char>('0');

	//flags
	auto flagData = input.get_child_optional("flags");
	if (flagData.is_initialized()) {
		for (auto item : flagData.get()) {
			if (item.first == "burnable")
				flags |= BURNABLE * item.second.get_value<int>(0);
		}
	}

	//assets
	assets.push_back(input.get<std::string>("asset"));

	auto assetData = input.get_child_optional("assets");
	if (assetData.is_initialized())
	{
		for (auto item : *assetData)
		{
			std::string label{ item.first };
			std::string path{ item.second.data() };
			//assets.insert(make_pair(label, path));
			assets.push_back(path);
		}
	}

	return external_type(id, flags, assets);
}


Tool_Data create_data(_cfg::config_type& cfg, DataTranslator& translator, const scalar& s)
{
	Tool_Data data = *(translator.get_value(cfg));
	data.pos = s;
	return std::move(data);
}