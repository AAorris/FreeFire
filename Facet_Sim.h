#pragma once
#include "FACET_GLOBALS.h"
#include "Facet.h"
#include "Tool_Pos.h"
//#include "Facet_Gfx.h"
#include "camera_data.h"
#include "Tool_Configurable.h"
#include "Tool_Data.h"
#include "scalar.h"
#include <boost\property_tree\ptree.hpp>
//#include <map>
#include <unordered_map>
#include <set>

class Facet_Sim
{
public:
	using master_type = facet::master_type;
	using group_type = facet::master_group_type;
	using ptree = facet::ptree;
	using template_key = facet::template_key;
	using template_type = facet::template_type;

	master_type data;
	ptree information;
	std::map<template_key, const template_type> templates;

	int time = 0;
	tile::Unit* selectedUnit = nullptr;
	void select(const scalar& cell);

	Facet_Sim();
	~Facet_Sim()=default;
	facet::group_item_value& operator()(const tile::group_type& group, const scalar& pos);

	void connect(_cfg& session);
	//void set(const scalar& pos, const template_key& key);
	bool insert(const scalar& pos, const template_key& key);
	void update(int ms);
	int& wind(std::string direction = "N");
	std::vector<std::pair<scalar, tile::Data*>> around(const tile::group_type& type, const scalar& pos);
	
};

//typedef Facet_Sim _sim;