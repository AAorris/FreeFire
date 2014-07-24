#pragma once
#include "Facet.h"
#include "Tool_Pos.h"
#include "Facet_Gfx.h"
#include "camera_data.h"
#include "Tool_Configurable.h"
#include "Tool_Data.h"
#include "scalar.h"
//#include <map>
#include <unordered_map>
#include <set>

class Facet_Sim
{
public:
	using template_type = tile::Template;
	using template_key = tile::id_type;
	using group_type = std::unordered_map<scalar, tile::Data*>;
	//attempting to order lexicographically by id, and then position.
	//Imagine updating all trees, all houses, all units, all fires.
	//This should help with branch prediction when update checks id (maybe?)
	using master_type =
		std::unordered_map<
			tile::group_type,
			group_type
		>;
	master_type data;
	std::map<template_key, const template_type> templates;

	int time = 0;

	Facet_Sim();
	~Facet_Sim()=default;
	tile::Data* operator()(const tile::group_type& group, const scalar& pos);

	void connect(_cfg& session);
	void set(const scalar& pos, const template_key& key);
	bool insert(const scalar& pos, const template_key& key);
	void update(int ms);
	std::vector<group_type::value_type> around(const tile::group_type& type, const scalar& pos);
	
};

//typedef Facet_Sim _sim;