#include "SimulationManager.h"


SimulationManager::SimulationManager()
{
	grid = t_grid();
}


SimulationManager::~SimulationManager()
{
}

bool SimulationManager::hasLookup(const int& i)
{
	if (lookup.count(i) > 0)
		return true;
	return false;
}

void SimulationManager::createLookup(const int& key, const std::string& path)
{
	lookup.insert(std::make_pair(key, path));
}

std::string SimulationManager::path(const int& key)
{
	if (lookup.count(key) != 0)
	{
		return lookup[key];
	}
	else return "";
}

int SimulationManager::getTile(const std::string& group, const pos& cell)
{
	if (grid.count(group) != 0 && grid[group].count(cell) != 0)
		return grid[group][cell];
	else
		return -1;
}

void SimulationManager::updateTile(const std::string& group, const pos& cell, int newIndex)
{
	if (grid.count(group) != 0)
	{
		if (grid[group].count(cell) != 0)
			grid[group][cell] = newIndex;
		else
			grid[group].insert(std::make_pair(cell,newIndex));
	}
	else {
		grid.insert(std::make_pair(group, t_chunk()));
	}
}