#pragma once
#include "Tool_Data.h"
#include <unordered_map>
#include <set>
#include <unordered_set>

namespace facet {

	struct sim_item {
		scalar pos;
		tile::Data* data;
		sim_item(scalar _pos, tile::Data* _data = nullptr) : pos{ _pos }, data{ _data } {}
		bool operator==(const sim_item& right) const {
			return pos == right.pos && data == right.data;
		}
		bool operator<(const sim_item& other) const {
			return pos < other.pos && data < other.data;
		}
		tile::Data* operator->() { return data; }
	};

	using template_type = tile::Template;
	using template_key = tile::id_type;

	using group_item_key = scalar;
	using group_item_value = std::unordered_set<tile::Data*>;

	using master_group_key = char;

	using master_group_type = std::map<group_item_key,group_item_value>;
	using ptree = boost::property_tree::ptree;
	//attempting to order lexicographically by id, and then position.
	//Imagine updating all trees, all houses, all units, all fires.
	//This should help with branch prediction when update checks id (maybe?)
	using master_type =
		std::unordered_map<
		master_group_key,
		master_group_type
		>;

	inline master_type::iterator Group(const master_group_key& key, master_type* master) {
		return master->find(key);
	}
	inline master_group_type::iterator Item(const master_group_type::key_type& key, master_group_type* group) {
		return group->find(key);
	}
	inline void insert(const scalar& key, tile::Data* value, master_group_type* p_group)
	{
		//container->insert(group_type::value_type(key, value)); /*Multimap*/
		master_group_type& group = *p_group;
		group[key].insert(value);
	}
}

namespace std {
	template<> struct hash<facet::sim_item> {
		size_t operator()(const facet::sim_item& item) const {
			size_t hash = std::hash<scalar>()(item.pos);
			return hash;
		}
	};
}
