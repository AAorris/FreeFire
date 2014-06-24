#include "SimulationManager.h"


SimulationManager::SimulationManager()
{
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

void SimulationManager::updateTile(const pos& p, int newIndex)
{
	if (tiles.count(p) != 0)
	{
		tiles.find(p)->second = newIndex;
	}
	else {
		tiles.insert(std::make_pair(p, newIndex));
	}
}