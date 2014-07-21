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

class Facet_Sim
{
public:
	using t_map = std::unordered_map<scalar, Tool_Data>;
	using t_emplates = std::unordered_map<char, _cfg>;

	Facet_Sim();
	~Facet_Sim()=default;
	Tool_Data& operator()(const scalar& pos);

	void set(const scalar& pos, const char& id);
	void update(int ms);
	std::vector<Tool_Data*> around(const scalar& pos);
	
	t_map data;
	t_emplates templates;
};

typedef Facet_Sim _sim;